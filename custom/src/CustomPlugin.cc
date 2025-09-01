#include "CustomPlugin.h"

#include "AppSettings.h"
#include "AutoConnectSettings.h"
#include "HorizontalFactValueGrid.h"
#include "InstrumentValueData.h"
#include "JoystickManager.h"
#include "MultiVehicleManager.h"
#include "QGCApplication.h"
#include "QGCMAVLink.h"
#include "VideoSettings.h"

QGC_LOGGING_CATEGORY(HerelinkCorePluginLog, "HerelinkCorePluginLog")

Q_APPLICATION_STATIC(CustomPlugin, _customPluginInstance);

CustomPlugin::CustomPlugin(QObject* parent)
    : QGCCorePlugin { parent }
    , _herelinkOptions { this, parent } {
}

QGCCorePlugin* CustomPlugin::instance() {
    return _customPluginInstance;
}

QGCOptions* CustomPlugin::options() {
    return qobject_cast<QGCOptions*>(&_herelinkOptions);
}

bool CustomPlugin::overrideSettingsGroupVisibility(QString name) {
    // Hide all AutoConnect settings
    return name != AutoConnectSettings::name;
}

bool CustomPlugin::adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData) {
    if (settingsGroup == AppSettings::settingsGroup) {
        //-- Default Herelink font size of 10, nice starting point.
        if (metaData.name() == AppSettings::appFontPointSizeName) {
            metaData.setRawDefaultValue(10);
            return true;
        }

        // Dark palette by default.
        if (metaData.name() == AppSettings::indoorPaletteName) {
            metaData.setRawDefaultValue(1);
            return true;
        }
    }

    if (settingsGroup == AutoConnectSettings::settingsGroup) {
        // We have to adjust the Herelink UDP autoconnect settings for the AirLink
        if (metaData.name() == AutoConnectSettings::udpListenPortName) {
            metaData.setRawDefaultValue(14551);
        } else if (metaData.name() == AutoConnectSettings::udpTargetHostIPName) {
            metaData.setRawDefaultValue(QStringLiteral("127.0.0.1"));
        } else if (metaData.name() == AutoConnectSettings::udpTargetHostPortName) {
            metaData.setRawDefaultValue(15552);
        } else {
            // Disable all the other autoconnect types
            const std::string_view disabledAndHiddenSettings[] {
                AutoConnectSettings::autoConnectPixhawkName,
                AutoConnectSettings::autoConnectSiKRadioName,
                AutoConnectSettings::autoConnectRTKGPSName,
                AutoConnectSettings::autoConnectLibrePilotName,
                AutoConnectSettings::autoConnectNmeaPortName,
                AutoConnectSettings::autoConnectZeroConfName,
            };

            for (const std::string_view dahs : disabledAndHiddenSettings) {
                if (dahs == metaData.name()) {
                    metaData.setRawDefaultValue(false);
                }
            }
        }
    } else if (settingsGroup == VideoSettings::settingsGroup) {
        if (metaData.name() == VideoSettings::rtspTimeoutName) {
            metaData.setRawDefaultValue(60);
        } else if (metaData.name() == VideoSettings::videoSourceName) {
            metaData.setRawDefaultValue(VideoSettings::videoSourceHerelinkAirUnit);
        }
    } else if (settingsGroup == AppSettings::settingsGroup) {
        if (metaData.name() == AppSettings::androidSaveToSDCardName) {
            metaData.setRawDefaultValue(true);
        }
    }

    return true;
}

void CustomPlugin::activeVehicleChanged(Vehicle* activeVehicle) {
    if (activeVehicle == nullptr) {
        return;
    }

    QString          herelinkButtonsJoystickName { "gpio-keys" };
    JoystickManager* joystickManager { JoystickManager::instance() };

    if (joystickManager->activeJoystickName() != herelinkButtonsJoystickName) {
        if (!joystickManager->setActiveJoystickName(herelinkButtonsJoystickName)) {
            qgcApp()->showAppMessage("Warning: Herelink buttton setup failed. Buttons will not work.");
            return;
        }
    }

    activeVehicle->setJoystickEnabled(true);
}

namespace {
    struct valueData {
        QString
            vehicle,
            icon,
            text;

        bool showUnits { true };
    };

    void appendOne(InstrumentValueData& ivd, const valueData& d) {
        ivd.setFact("Vehicle", d.vehicle);

        if (!d.icon.isNull()) {
            ivd.setIcon(d.icon);
        }

        ivd.setText(d.text.isNull() ? ivd.fact()->shortDescription() : d.text);
        ivd.setShowUnits(d.showUnits);
    }

    template <std::size_t N>
    void appendMany(QmlObjectListModel& qolm, const valueData (&arr)[N]) {
        for (std::size_t i { 0 }; i < N; ++i) {
            appendOne(*qolm.value<InstrumentValueData*>(i), arr[i]);
        }
    }
}

// Same as original, only we set font size to medium by default for Herelink
void CustomPlugin::factValueGridCreateDefaultSettings() {
    HorizontalFactValueGrid factValueGrid;
    factValueGrid.setFontSize(FactValueGrid::MediumFontSize);

    const QGCMAVLink::VehicleClass_t vc { factValueGrid.vehicleClass() };

    const bool includeFWValues {
        vc == QGCMAVLink::VehicleClassFixedWing
        || vc == QGCMAVLink::VehicleClassVTOL
        || vc == QGCMAVLink::VehicleClassAirship
    };

    for (std::size_t i { 0 }; i < 3 + includeFWValues; ++i) {
        factValueGrid.appendColumn();
    }

    factValueGrid.appendRow();

    std::size_t idx { 0 };

    appendMany(
        *factValueGrid.columns()->value<QmlObjectListModel*>(idx++),
        { { "AltitudeRelative", "arrow-thick-up.svg" },
          { "DistanceToHome", "bookmark copy 3.svg" } }
    );

    appendMany(
        *factValueGrid.columns()->value<QmlObjectListModel*>(idx++),
        { { "ClimbRate", "arrow-simple-up.svg" },
          { "GroundSpeed", "arrow-simple-right.svg" } }
    );

    if (includeFWValues) {
        appendMany(
            *factValueGrid.columns()->value<QmlObjectListModel*>(idx++),
            { { "AirSpeed", {}, "AirSpd" },
              { "ThrottlePct", {}, "Thr" } }
        );
    }

    appendMany( // We still increment here just in case we add more code later.
        *factValueGrid.columns()->value<QmlObjectListModel*>(idx++),
        { { "FlightTime", "timer.svg", {}, false },
          { "FlightDistance", "travel-walk.svg" } }
    );
}
