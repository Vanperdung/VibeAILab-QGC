#pragma once

#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>

#include "ParameterMetaData.h"

Q_DECLARE_LOGGING_CATEGORY(VibeAILabParameterMetaDataLog)

/// Param metadata (descriptions, limits, decimal places) for VibeAI Lab flight controller.
/// The FC serves no metadata, so it is bundled in the QGC build, in the format of the "parameters" list of
/// MAVLink component metadata. A param missing from the file gets plain metadata, like any generic firmware param.
class VibeAILabParameterMetaData : public ParameterMetaData
{
public:
    explicit VibeAILabParameterMetaData(QObject* parent = nullptr);

protected:
    void parseParameterJson(const QJsonObject& json) final;
    FactMetaData* _lookupMetaData(const QString& name, FactMetaData::ValueType_t type) final;
    void _postProcessMetaData(const QString& name, FactMetaData* metaData) final;

private:
    QHash<QString, FactMetaData*> _fileMetaData;
};
