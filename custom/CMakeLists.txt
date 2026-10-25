# VibeAILab custom build: our sources for the QGroundControl target

message(STATUS "QGC: Adding VibeAILab custom build")

set(CUSTOM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabCorePlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Integration/VibeAILabCorePlugin.h
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
