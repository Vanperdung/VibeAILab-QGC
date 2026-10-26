# VibeAILab custom build

message(STATUS "QGC: Adding VibeAILab custom build")

set(CUSTOM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabAutoPilotPlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabAutoPilotPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabCorePlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabCorePlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabFirmwarePlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabFirmwarePlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabFirmwarePluginFactory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabFirmwarePluginFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vibeailab.qrc
    CACHE INTERNAL "" FORCE
)

set(CUSTOM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration
    CACHE INTERNAL "" FORCE
)

set(CUSTOM_DEFINITIONS
    QGC_CUSTOM_BUILD
    CUSTOMHEADER="VibeAILabCorePlugin.h"
    CUSTOMCLASS=VibeAILabCorePlugin
    CACHE INTERNAL "" FORCE
)
