#pragma once

#include "HerelinkOptions.h"
#include "UrlInterceptor.h"

#include <QGCApplication.h>
#include <QGCCorePlugin.h>
#include <QGCLoggingCategory.h>

Q_DECLARE_LOGGING_CATEGORY(HerelinkCorePluginLog)

class HerelinkCorePlugin final : public QGCCorePlugin {
    Q_OBJECT

    Q_PROPERTY(bool isHerelink READ isHerelink CONSTANT)

public:
    explicit HerelinkCorePlugin(QObject* parent = nullptr);
    virtual ~HerelinkCorePlugin() = default;

    static QGCCorePlugin* instance();
    QGCOptions*           options() final;

    void cleanup() final;

    constexpr bool isHerelink(void) const { return true; }

    bool overrideSettingsGroupVisibility(const QString& name) final;
    bool adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData) final;

    QQmlApplicationEngine* createQmlApplicationEngine(QObject* parent) final;

private slots:
    void activeVehicleChanged(Vehicle* activeVehicle);

private:
    HerelinkOptions        m_herelinkOptions;
    UrlInterceptor*        m_interceptor;
    QQmlApplicationEngine* m_qmlEngine;
};
