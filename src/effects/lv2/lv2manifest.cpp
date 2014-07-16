#include "effects/lv2/lv2manifest.h"

LV2Manifest::LV2Manifest(const LilvPlugin* plug,
                         QHash<QString, LilvNode*>& properties)
        : m_isValid(false) {

    m_pLV2plugin = plug;

    // Get and set the ID
    const LilvNode* id = lilv_plugin_get_uri(m_pLV2plugin);
    m_effectManifest.setId(lilv_node_as_string(id));

    // Get and set the name
    LilvNode* info = lilv_plugin_get_name(m_pLV2plugin);
    m_effectManifest.setName(lilv_node_as_string(info));
    lilv_node_free(info);

    // Get and set the author
    info = lilv_plugin_get_author_name(m_pLV2plugin);
    m_effectManifest.setAuthor(lilv_node_as_string(info));
    lilv_node_free(info);

    int numPorts = lilv_plugin_get_num_ports(plug);
    m_minimum = new float[numPorts];
    m_maximum = new float[numPorts];
    m_default = new float[numPorts];
    lilv_plugin_get_port_ranges_float(m_pLV2plugin, m_minimum, m_maximum,
                                      m_default);

    // Counters to determine the type of the plug in
    int inputPorts = 0;
    int outputPorts = 0;

    for (int i = 0; i < numPorts; i++) {
        const LilvPort *port = lilv_plugin_get_port_by_index(plug, i);

        if (lilv_port_is_a(m_pLV2plugin, port, properties["audio_port"])) {
            if (lilv_port_is_a(m_pLV2plugin, port, properties["input_port"])) {
                inputPorts++;
                info = lilv_port_get_name(m_pLV2plugin, port);
                QString paramName = lilv_node_as_string(info);
                qDebug() << "Input Port name: " << paramName;
            } else if (lilv_port_is_a(m_pLV2plugin, port, properties["output_port"])) {
                outputPorts++;
                info = lilv_port_get_name(m_pLV2plugin, port);
                QString paramName = lilv_node_as_string(info);
                qDebug() << "Output Port name: " << paramName;
            }
        }

        if (lilv_port_is_a(m_pLV2plugin, port, properties["control_port"])) {
            EffectManifestParameter* param = m_effectManifest.addParameter();

            // Get and set the parameter name
            info = lilv_port_get_name(m_pLV2plugin, port);
            QString paramName = lilv_node_as_string(info);
            qDebug() << "Parameter name: " << paramName;
            param->setName(paramName);
            lilv_node_free(info);

            // Build and set the parameter id from its name
            param->setId(paramName.trimmed().toLower().replace(' ', '_'));
            qDebug() << "Parameter id: " << paramName.trimmed().toLower().replace(' ', '_');

            param->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
            if (lilv_port_has_property(m_pLV2plugin, port, properties["integer_port"])) {
                qDebug() << "asta a fost un integer port";
                param->setValueHint(EffectManifestParameter::VALUE_INTEGRAL);
            } else {
                param->setValueHint(EffectManifestParameter::VALUE_FLOAT);
            }

            param->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
            param->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
            param->setDefault(m_default[i]);
            param->setMinimum(m_minimum[i]);
            param->setMaximum(m_maximum[i]);
            qDebug() << paramName << " values(def, min, max)"
                     << m_default[i] << ", " << m_minimum[i]
                     << ", " << m_maximum[i];

            // Currently not available to make button parameters
//            if (lilv_port_has_property(m_pLV2plugin, port, properties["button_port"])) {
//                qDebug() << "asta a fost un button port";
//            }
//            if (lilv_port_has_property(m_pLV2plugin, port, properties["enumeration_port"])) {
//                qDebug() << "asta a fost un enumeration port";
//            }
            qDebug() << inputPorts << " " << outputPorts << endl;
        }
    }

    if (inputPorts == 2 && outputPorts == 2) {
        m_isValid = true;
    }
}

LV2Manifest::~LV2Manifest() {
    delete m_minimum;
    delete m_maximum;
    delete m_default;
}

EffectManifest LV2Manifest::getEffectManifest() {
    return m_effectManifest;
}

bool LV2Manifest::isValid() {
    return m_isValid;
}
