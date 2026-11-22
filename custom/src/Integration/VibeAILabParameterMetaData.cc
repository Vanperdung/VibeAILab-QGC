#include "VibeAILabParameterMetaData.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(VibeAILabParameterMetaDataLog, "VibeAILab.ParameterMetaData")

VibeAILabParameterMetaData::VibeAILabParameterMetaData(QObject* parent) : ParameterMetaData(parent) {}

void VibeAILabParameterMetaData::parseParameterJson(const QJsonObject& json)
{
    const QJsonArray parameters = json.value(u"parameters").toArray();
    for (const QJsonValue& parameter : parameters) {
        if (!parameter.isObject()) {
            qCWarning(VibeAILabParameterMetaDataLog) << "Skipping a parameters entry that is not an object";
            continue;
        }

        FactMetaData* const metaData = FactMetaData::createFromJsonObject(parameter.toObject(), kEmptyDefines, this);
        if (metaData->name().isEmpty()) {
            // createFromJsonObject has already logged why
            metaData->deleteLater();
            continue;
        }

        if (_fileMetaData.contains(metaData->name())) {
            qCWarning(VibeAILabParameterMetaDataLog)
                << "Duplicate parameter, keeping the last one:" << metaData->name();
            _fileMetaData.take(metaData->name())->deleteLater();
        }
        _fileMetaData.insert(metaData->name(), metaData);
    }
}

FactMetaData* VibeAILabParameterMetaData::_lookupMetaData(const QString& name, FactMetaData::ValueType_t type)
{
    FactMetaData* const metaData = _fileMetaData.value(name);
    if (metaData && (metaData->type() != type)) {
        qCWarning(VibeAILabParameterMetaDataLog)
            << "Ignoring the metadata of" << name << "- file type" << FactMetaData::typeToString(metaData->type())
            << "but the FC serves" << FactMetaData::typeToString(type);
        return nullptr;
    }

    return metaData;
}

void VibeAILabParameterMetaData::_postProcessMetaData(const QString& name, FactMetaData* metaData)
{
    // Same grouping as QGC gives params of generic firmware: by the name part before the first '_'
    if (metaData->group() != FactMetaData::defaultGroup()) {
        return;
    }

    const qsizetype separator = name.indexOf(u'_');
    if (separator > 0) {
        metaData->setGroup(name.left(separator));
    }
}
