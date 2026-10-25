#include "VibeAILabCorePlugin.h"

#include <QtCore/QApplicationStatic>

Q_APPLICATION_STATIC(VibeAILabCorePlugin, _vibeAILabCorePluginInstance);

VibeAILabCorePlugin::VibeAILabCorePlugin(QObject* parent) : QGCCorePlugin(parent) {}

QGCCorePlugin* VibeAILabCorePlugin::instance()
{
    return _vibeAILabCorePluginInstance();
}
