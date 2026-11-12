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

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
if(QGC_BUILD_TESTING)
    list(APPEND CUSTOM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors/VibeAILabAccelCalibrationTest.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors/VibeAILabAccelCalibrationTest.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors/VibeAILabGyroCalibrationTest.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors/VibeAILabGyroCalibrationTest.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors/VibeAILabSensorsControllerTest.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors/VibeAILabSensorsControllerTest.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors/VibeAILabSensorsUITest.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors/VibeAILabSensorsUITest.h
    )
    list(APPEND CUSTOM_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/test/Sensors
    )

    # add_qgc_test() is defined after this folder is read, so register the tests at the end of the root folder
    cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL add_qgc_test VibeAILabAccelCalibrationTest LABELS Unit)
    cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL add_qgc_test VibeAILabGyroCalibrationTest LABELS Unit)
    cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR}
        CALL add_qgc_test VibeAILabSensorsControllerTest LABELS Integration Vehicle)
    cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR}
        CALL add_qgc_test VibeAILabSensorsUITest LABELS Integration NoSanitizer TIMEOUT 180)
endif()

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
