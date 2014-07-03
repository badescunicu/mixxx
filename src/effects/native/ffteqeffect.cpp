#include "effects/native/ffteqeffect.h"
#include "util/math.h"

// static
QString FFTEQEffect::getId() {
    return "org.mixxx.effects.ffteqeffect";
}

// static
EffectManifest FFTEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("FFT EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription("FFT EQ Effect");
    return manifest;
}

FFTEQEffectGroupState::FFTEQEffectGroupState() {

    // Because the sample size is 2048 (left + right channels)
    fft_obj = new ffft::FFTReal<float>(1024);

    m_chLeft = new CSAMPLE[1024];
    m_chRight = new CSAMPLE[1024];
    m_fftChLeft = new CSAMPLE[1024];
    m_fftChRight = new CSAMPLE[1024];
}

FFTEQEffectGroupState::~FFTEQEffectGroupState() {
    delete m_chLeft;
    delete m_chRight;
    delete m_fftChLeft;
    delete m_fftChRight;
}

FFTEQEffect::FFTEQEffect(EngineEffect* pEffect,
                   const EffectManifest& manifest) {
    Q_UNUSED(manifest);
    Q_UNUSED(pEffect);
}

FFTEQEffect::~FFTEQEffect() {
}

void FFTEQEffect::processGroup(const QString& group,
        FFTEQEffectGroupState* pState,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const unsigned int numSamples,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);

    int j = 0;
    for (unsigned int i = 0; i < numSamples; i += 2) {
        pState->m_chLeft[j] = pInput[i];
        pState->m_chRight[j] = pInput[i + 1];
        j++;
    }

    pState->fft_obj->do_fft(pState->m_fftChLeft, pState->m_chLeft);
    pState->fft_obj->do_fft(pState->m_fftChRight, pState->m_chRight);

    // Increase the frequencies
    int coef = 2;
    for (int i = 0; i < 1024; i++) {
        pState->m_fftChLeft[i] *= coef;
        pState->m_fftChLeft[512 + i] *= coef;
        pState->m_fftChRight[i] *= coef;
        pState->m_fftChRight[512 + i] *= coef;
    }

    pState->fft_obj->do_ifft(pState->m_fftChLeft, pState->m_chLeft);
    pState->fft_obj->do_ifft(pState->m_fftChRight, pState->m_chRight);


    // Rescale, because IFFT(FFT(x)) = x * length(x)
    pState->fft_obj->rescale(pState->m_chLeft);
    pState->fft_obj->rescale(pState->m_chRight);

    j = 0;
    for (unsigned int i = 0; i < numSamples; i += 2) {
        pOutput[i] = pState->m_chLeft[j];
        pOutput[i + 1] = pState->m_chRight[j];
        j++;
    }
}
