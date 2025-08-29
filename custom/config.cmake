message(STATUS "QGC: Adding Custom Plugin")

# Enable custom build
set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
    APPEND PROPERTY COMPILE_DEFINITIONS
    QGC_CUSTOM_BUILD
    CUSTOMHEADER="CustomPlugin.h"
    CUSTOMCLASS=CustomPlugin
)

if(ANDROID)
    set(CUSTOM_ANDROID_DIR "${CMAKE_SOURCE_DIR}/custom/android")
    if(EXISTS "${CUSTOM_ANDROID_DIR}")
        file(GLOB CUSTOM_ANDROID_FILES "${CUSTOM_ANDROID_DIR}/*")
        if(CUSTOM_ANDROID_FILES)
            message(STATUS "QGC: Custom Android package template found. Overlaying custom files...")
            set(DEFAULT_ANDROID_DIR "${CMAKE_SOURCE_DIR}/android")
            set(FINAL_ANDROID_DIR "${CMAKE_BINARY_DIR}/custom/android")
            file(COPY "${DEFAULT_ANDROID_DIR}/." DESTINATION "${FINAL_ANDROID_DIR}")
            file(COPY "${CUSTOM_ANDROID_DIR}/." DESTINATION "${FINAL_ANDROID_DIR}")
            set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                            "${DEFAULT_ANDROID_DIR}/"
                            "${CUSTOM_ANDROID_DIR}/"
                        )
            set(QGC_ANDROID_PACKAGE_SOURCE_DIR "${FINAL_ANDROID_DIR}" CACHE PATH "Path to a custom Android package template" FORCE)
            message(STATUS "QGC: Android package template path will be set to: ${QGC_ANDROID_PACKAGE_SOURCE_DIR}")
        else()
            message(STATUS "QGC: Custom Android package template empty. Using default.")
        endif()
    else()
        message(STATUS "QGC: No custom Android package template found. Using default.")
    endif()
endif()

# Our own, custom resources
list(APPEND CUSTOM_RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/custom.qrc
)

set(
    QGC_RESOURCES ${QGC_RESOURCES} ${CUSTOM_RESOURCES}
    CACHE STRING "Paths to .qrc Resources" FORCE
)

set(
    QML_IMPORT_PATH ${QML_IMPORT_PATH} "${CMAKE_CURRENT_LIST_DIR}/res"
    CACHE STRING "Extra qml import paths" FORCE
)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)
set(CMAKE_AUTOMOC ON)

qt_add_library(CustomModule STATIC)

target_link_libraries(CustomModule PUBLIC Qt6::Core)
target_include_directories(CustomModule PUBLIC include)

set_source_files_properties(res/Custom/Widgets/CustomArtificialHorizon.qml PROPERTIES QT_RESOURCE_ALIAS CustomArtificialHorizon.qml)
set_source_files_properties(res/Custom/Widgets/CustomAttitudeWidget.qml PROPERTIES QT_RESOURCE_ALIAS CustomAttitudeWidget.qml)
set_source_files_properties(res/Custom/Widgets/CustomIconButton.qml PROPERTIES QT_RESOURCE_ALIAS CustomIconButton.qml)
set_source_files_properties(res/Custom/Widgets/CustomOnOffSwitch.qml PROPERTIES QT_RESOURCE_ALIAS CustomOnOffSwitch.qml)
set_source_files_properties(res/Custom/Widgets/CustomQuickButton.qml PROPERTIES QT_RESOURCE_ALIAS CustomQuickButton.qml)
set_source_files_properties(res/Custom/Widgets/CustomSignalStrength.qml PROPERTIES QT_RESOURCE_ALIAS CustomSignalStrength.qml)
set_source_files_properties(res/Custom/Widgets/CustomToolBarButton.qml PROPERTIES QT_RESOURCE_ALIAS CustomToolBarButton.qml)
set_source_files_properties(res/Custom/Widgets/CustomVehicleButton.qml PROPERTIES QT_RESOURCE_ALIAS CustomVehicleButton.qml)

qt_add_qml_module(CustomModule
    URI Custom.Widgets
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES
        custom/res/Custom/Widgets/CustomArtificialHorizon.qml
        custom/res/Custom/Widgets/CustomAttitudeWidget.qml
        custom/res/Custom/Widgets/CustomIconButton.qml
        custom/res/Custom/Widgets/CustomOnOffSwitch.qml
        custom/res/Custom/Widgets/CustomQuickButton.qml
        custom/res/Custom/Widgets/CustomSignalStrength.qml
        custom/res/Custom/Widgets/CustomToolBarButton.qml
        custom/res/Custom/Widgets/CustomVehicleButton.qml
    NO_PLUGIN
)

set(CUSTOM_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/CustomPlugin.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/GeoWork.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/GeoWork_qmlinit.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/HerelinkCorePlugin.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/HerelinkOptions.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/VideoStreamControl.cc
    CACHE INTERNAL "" FORCE
)

# Explicitly link QML module, needed by Qt 6.6.3
# TODO: Remove when support for this version is dropped
set(CUSTOM_LIBRARIES
    CustomModule
    CACHE INTERNAL "" FORCE
)

set(CUSTOM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/include
    CACHE INTERNAL "" FORCE
)
