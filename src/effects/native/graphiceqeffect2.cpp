#include "effects/native/graphiceqeffect2.h"
#include "util/math.h"

// static
QString GraphicEQEffect2::getId() {
    return "org.mixxx.effects.graphiceq2";
}

// static
EffectManifest GraphicEQEffect2::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Graphic EQ2"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "An 8 band Graphic EQ based on Butterworth filters"));
    manifest.setEffectRampsFromDry(true);
    manifest.setIsMasterEQ(true);

    // Display rounded center frequencies for each filter
    float centerFrequencies[8] = {45, 100, 220, 500, 1100, 2500,
                                  5500, 12000};

    EffectManifestParameter* low = manifest.addParameter();
    low->setId(QString("low"));
    low->setName(QString("%1 Hz").arg(centerFrequencies[0]));
    low->setDescription(QObject::tr("Gain for Low Filter"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setNeutralPointOnScale(0.5);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(4.0);

    QString paramName;
    for (int i = 0; i < 6; i++) {
        if (centerFrequencies[i + 1] < 1000) {
            paramName = QString("%1 Hz").arg(centerFrequencies[i + 1]);
        } else {
            paramName = QString("%1 kHz").arg(centerFrequencies[i + 1] / 1000);
        }

        EffectManifestParameter* mid = manifest.addParameter();
        mid->setId(QString("mid%1").arg(i));
        mid->setName(paramName);
        mid->setDescription(QObject::tr("Gain for Band Filter %1").arg(i));
        mid->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
        mid->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
        mid->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
        mid->setNeutralPointOnScale(0.5);
        mid->setDefault(1.0);
        mid->setMinimum(0);
        mid->setMaximum(4.0);
    }

    EffectManifestParameter* high = manifest.addParameter();
    high->setId(QString("high"));
    high->setName(QString("%1 kHz").arg(centerFrequencies[7] / 1000));
    high->setDescription(QObject::tr("Gain for High Filter"));
    high->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    high->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    high->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    high->setDefault(1.0);
    high->setMinimum(0);
    high->setMaximum(4.0);

    return manifest;
}

GraphicEQEffect2GroupState::GraphicEQEffect2GroupState() {
    m_oldLow = 0;
    for (int i = 0; i < 6; i++) {
        m_oldMid.append(1.0);
    }
    m_oldHigh = 0;

    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));
    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));
    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));
    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));
    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));
    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));
    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));
    m_pBufs.append(SampleUtil::alloc(MAX_BUFFER_LEN));

    // Initialize the default center frequencies
    m_centerFrequencies[0] = 81;
    m_centerFrequencies[1] = 100;
    m_centerFrequencies[2] = 222;
    m_centerFrequencies[3] = 494;
    m_centerFrequencies[4] = 1097;
    m_centerFrequencies[5] = 2437;
    m_centerFrequencies[6] = 5416;
    m_centerFrequencies[7] = 9828;

    // Initialize the filters with default parameters
    m_low = new EngineFilterButterworth4Low(44100, m_centerFrequencies[0]);
    m_high = new EngineFilterButterworth4High(44100, m_centerFrequencies[7]);
    for (int i = 1; i < 7; i++) {
        m_bands.append(new EngineFilterButterworth4Band(44100,
                                                      m_centerFrequencies[i],
                                                      m_centerFrequencies[i + 1]));
    }
}

GraphicEQEffect2GroupState::~GraphicEQEffect2GroupState() {
    foreach (EngineFilterButterworth4Band* filter, m_bands) {
        delete filter;
    }

    delete m_low;
    delete m_high;

    foreach(CSAMPLE* buf, m_pBufs) {
        SampleUtil::free(buf);
    }
}

void GraphicEQEffect2GroupState::setFilters(int sampleRate) {
    m_low->setFrequencyCorners(sampleRate, m_centerFrequencies[0]);
    m_high->setFrequencyCorners(sampleRate, m_centerFrequencies[7]);
    for (int i = 0; i < 6; i++) {
        m_bands[i]->setFrequencyCorners(sampleRate, m_centerFrequencies[i + 1],
                m_centerFrequencies[i + 2]);
    }
}

GraphicEQEffect2::GraphicEQEffect2(EngineEffect* pEffect,
                                 const EffectManifest& manifest)
        : m_oldSampleRate(44100) {
    Q_UNUSED(manifest);
    m_pPotLow = pEffect->getParameterById("low");
    for (int i = 0; i < 6; i++) {
        m_pPotMid.append(pEffect->getParameterById(QString("mid%1").arg(i)));
    }
    m_pPotHigh = pEffect->getParameterById("high");
}

GraphicEQEffect2::~GraphicEQEffect2() {
}

void GraphicEQEffect2::processChannel(const ChannelHandle& handle,
                                     GraphicEQEffect2GroupState* pState,
                                     const CSAMPLE* pInput, CSAMPLE* pOutput,
                                     const unsigned int numSamples,
                                     const unsigned int sampleRate,
                                     const EffectProcessor::EnableState enableState,
                                     const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    // If the sample rate has changed, initialize the filters using the new
    // sample rate
    if (m_oldSampleRate != sampleRate) {
        m_oldSampleRate = sampleRate;
        pState->setFilters(sampleRate);
    }

    float fLow;
    float fMid[6];
    float fHigh;

    fLow = m_pPotLow->value();
    fHigh = m_pPotHigh->value();
    for (int i = 0; i < 6; i++) {
        fMid[i] = m_pPotMid[i]->value();
    }


    int bufIndex = 0;
    pState->m_low->process(pInput, pState->m_pBufs[0], numSamples);

    for (int i = 0; i < 6; i++) {
        pState->m_bands[i]->process(pInput,
                                    pState->m_pBufs[i + 1], numSamples);
    }

    pState->m_high->process(pInput,
                            pState->m_pBufs[7], numSamples);

    SampleUtil::copy8WithGain(pOutput,
            pState->m_pBufs[0], fLow,
            pState->m_pBufs[1], fMid[0],
            pState->m_pBufs[2], fMid[1],
            pState->m_pBufs[3], fMid[2],
            pState->m_pBufs[4], fMid[3],
            pState->m_pBufs[5], fMid[4],
            pState->m_pBufs[6], fMid[5],
            pState->m_pBufs[7], fHigh,
            numSamples
    );
}
