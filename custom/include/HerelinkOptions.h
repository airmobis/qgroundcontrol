#pragma once

#include "QGCOptions.h"

class CustomPlugin;

class HerelinkOptions : public QGCOptions {
public:
    HerelinkOptions(CustomPlugin* plugin, QObject* parent = nullptr);

    // QGCOptions overrides
    bool wifiReliableForCalibration() const override { return true; }
    bool showFirmwareUpgrade() const override { return false; }
    bool multiVehicleEnabled() const override { return false; }
    bool allowJoystickSelection() const override { return false; }
};
