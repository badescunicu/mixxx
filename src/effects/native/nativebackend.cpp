#include <QtDebug>

#include "effects/native/nativebackend.h"
#include "effects/native/flangereffect.h"
#include "effects/native/bitcrushereffect.h"
#include "effects/native/eqdefault.h"
#include "effects/native/ffteqeffect.h"
#include "effects/native/filtereffect.h"
#include "effects/native/reverbeffect.h"
#include "effects/native/echoeffect.h"

NativeBackend::NativeBackend(QObject* pParent)
        : EffectsBackend(pParent, tr("Native")) {
    registerEffect<FlangerEffect>();
    registerEffect<BitCrusherEffect>();
    registerEffect<FilterEffect>();
    registerEffect<ReverbEffect>();
    registerEffect<EchoEffect>();
    registerEffect<EQDefault>();
    registerEffect<FFTEQEffect>();
}

NativeBackend::~NativeBackend() {
    //qDebug() << debugString() << "destroyed";
}
