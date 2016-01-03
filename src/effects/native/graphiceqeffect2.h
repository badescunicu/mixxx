#ifndef GRAPHICEQEFFECT2_H
#define GRAPHICEQEFFECT2_H

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbutterworth4.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

class GraphicEQEffect2GroupState {
  public:
    GraphicEQEffect2GroupState();
    virtual ~GraphicEQEffect2GroupState();

    void setFilters(int sampleRate);

    EngineFilterButterworth4Low* m_low;
    QList<EngineFilterButterworth4Band*> m_bands;
    EngineFilterButterworth4High* m_high;
    QList<CSAMPLE*> m_pBufs;
    QList<double> m_oldMid;
    double m_oldLow;
    double m_oldHigh;
    float m_centerFrequencies[8];
};

class GraphicEQEffect2 : public PerChannelEffectProcessor<GraphicEQEffect2GroupState> {
  public:
    GraphicEQEffect2(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~GraphicEQEffect2();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        GraphicEQEffect2GroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    QList<EngineEffectParameter*> m_pPotMid;
    EngineEffectParameter* m_pPotHigh;
    unsigned int m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(GraphicEQEffect2);
};

#endif // GRAPHICEQEFFECT_H
