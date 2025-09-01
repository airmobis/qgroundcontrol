#pragma once

#include <HerelinkOptions.h>

#include <QGCApplication.h>
#include <QGCCorePlugin.h>
#include <QGCLoggingCategory.h>

Q_DECLARE_LOGGING_CATEGORY(HerelinkCorePluginLog)

class CustomPlugin final : public QGCCorePlugin {
    Q_OBJECT

    Q_PROPERTY(bool isHerelink READ isHerelink CONSTANT)

public:
    explicit CustomPlugin(QObject* parent = nullptr);
    virtual ~CustomPlugin() = default;

    static QGCCorePlugin* instance();
    QGCOptions*           options() final;

    constexpr bool isHerelink(void) const { return true; }

    bool overrideSettingsGroupVisibility(const QString& name) final;
    bool adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData) final;

private slots:
    void activeVehicleChanged(Vehicle* activeVehicle);

private:
    HerelinkOptions _herelinkOptions;
};
