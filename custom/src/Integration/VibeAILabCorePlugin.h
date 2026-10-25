#pragma once

#include "QGCCorePlugin.h"

/// Core plugin for the VibeAILab build.
class VibeAILabCorePlugin : public QGCCorePlugin
{
    Q_OBJECT

public:
    explicit VibeAILabCorePlugin(QObject* parent = nullptr);

    static QGCCorePlugin* instance();
};
