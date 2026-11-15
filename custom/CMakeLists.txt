# VibeAILab custom build

message(STATUS "QGC: Adding VibeAILab custom build")

# ----------------------------------------------------------------------------
# Sensors page QML
# ----------------------------------------------------------------------------
qt_add_library(VibeAILabSensorsModule STATIC)

qgc_set_qt_resource_alias(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/qml/VibeAILabSensorsComponent.qml
)

qt_add_qml_module(VibeAILabSensorsModule
    URI VibeAILab.Sensors
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/qml/VibeAILabSensorsComponent.qml
    NO_PLUGIN
)

# ----------------------------------------------------------------------------
# Sources
# ----------------------------------------------------------------------------
set(CUSTOM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabAccelCalibration.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabAccelCalibration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabCalibration.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabCalibration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabGyroCalibration.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabGyroCalibration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabSensorsComponent.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabSensorsComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabSensorsController.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors/VibeAILabSensorsController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabAutoPilotPlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabAutoPilotPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabCorePlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabCorePlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabFirmwarePlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabFirmwarePlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabFirmwarePluginFactory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabFirmwarePluginFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vibeailab.qrc
)

set(CUSTOM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Sensors
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration
)

set(CUSTOM_SOURCES ${CUSTOM_SOURCES} CACHE INTERNAL "" FORCE)
set(CUSTOM_INCLUDE_DIRECTORIES ${CUSTOM_INCLUDE_DIRECTORIES} CACHE INTERNAL "" FORCE)

set(CUSTOM_LIBRARIES
    VibeAILabSensorsModule
    CACHE INTERNAL "" FORCE
)

set(CUSTOM_DEFINITIONS
    QGC_CUSTOM_BUILD
    CUSTOMHEADER="VibeAILabCorePlugin.h"
    CUSTOMCLASS=VibeAILabCorePlugin
    CACHE INTERNAL "" FORCE
)
