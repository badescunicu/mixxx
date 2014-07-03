#ifndef FFTEQEFFECT_H
#define FFTEQEFFECT_H

#include <QMap>

#include "controlobjectslave.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbutterworth8.h"
#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "sampleutil.h"

#include "ffft/FFTReal.hpp"

class FFTEQEffectGroupState {
public:
    FFTEQEffectGroupState();
    ~FFTEQEffectGroupState();

    ffft::FFTReal<float>* fft_obj;
    CSAMPLE* m_chLeft;
    CSAMPLE* m_chRight;
    CSAMPLE* m_fftChLeft;
    CSAMPLE* m_fftChRight;
};

class FFTEQEffect : public GroupEffectProcessor<FFTEQEffectGroupState> {
  public:
    FFTEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~FFTEQEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      FFTEQEffectGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    DISALLOW_COPY_AND_ASSIGN(FFTEQEffect);
};

#endif /* FFTEQEFFECT_H */
