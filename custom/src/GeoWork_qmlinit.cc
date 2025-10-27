#include "GeoWork.h"
#include <QDebug>
#include <QSslSocket>
#include <QtQml>

static void initGeoWorkQml() {
    // Make the module exist for the import resolver
    qmlRegisterModule("GeoWork", 1, 0);

    // Expose a singleton named GeoWork inside that module
    qmlRegisterSingletonInstance<GeoWork>("GeoWork", 1, 0, "GeoWork", new GeoWork);

    qDebug() << "[GeoWork] QSslSocket version = " << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "[GeoWork_qmlinit] GeoWork 1.0 registered";
}

Q_COREAPP_STARTUP_FUNCTION(initGeoWorkQml)
