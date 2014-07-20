#include "effects/lv2/lv2effectprocessor.h"
#include "engine/effects/engineeffect.h"

#define MAX_PARAMS 100

LV2EffectProcessor::LV2EffectProcessor(EngineEffect* pEngineEffect,
                                       const EffectManifest& manifest,
                                       const LilvPlugin* plugin) {
    m_sampleRate = getSampleRate();
    inputL = new float[MAX_BUFFER_LEN];
    inputR = new float[MAX_BUFFER_LEN];
    outputL = new float[MAX_BUFFER_LEN];
    outputR = new float[MAX_BUFFER_LEN];
    params = new float[MAX_PARAMS];

    handle = lilv_plugin_instantiate(plugin, m_sampleRate, NULL);
    const QList<EffectManifestParameter> effectManifestParameterList = manifest.parameters();
    // Initialize EngineEffectParameters
    foreach (EffectManifestParameter param, effectManifestParameterList) {
        m_parameters.append(pEngineEffect->getParameterById(param.id()));
    }

    for (int i = 0; i < m_parameters.size(); i++) {
        params[i] = m_parameters[i]->value().toFloat();
        lilv_instance_connect_port(handle, i + 4, &params[i]);
    }

    // Only for Calf Flanger, we are hard coding the indexes
    // TODO: somehow remove the hard coding; maybe get a vector of indexes
    // example: index_vector = [iL, iR, oL, oR];
    lilv_instance_connect_port(handle, 0, inputL);
    lilv_instance_connect_port(handle, 1, inputR);
    lilv_instance_connect_port(handle, 2, outputL);
    lilv_instance_connect_port(handle, 3, outputR);

    lilv_instance_activate(handle);
}

void LV2EffectProcessor::initialize(const QSet<QString>& registeredGroups) {
    Q_UNUSED(registeredGroups);

}

void LV2EffectProcessor::process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    for (int i = 0; i < m_parameters.size(); i++) {
        params[i] = m_parameters[i]->value().toFloat();
    }

    int j = 0;
    for (int i = 0; i < numSamples; i += 2) {
        inputL[j] = pInput[i];
        inputR[j] = pInput[i + 1];
        j++;
    }

    lilv_instance_run(handle, numSamples / 2);

    j = 0;
    for (unsigned int i = 0; i < numSamples; i += 2) {
        pOutput[i] = outputL[j];
        pOutput[i + 1] = outputR[j];
        j++;
    }
}
