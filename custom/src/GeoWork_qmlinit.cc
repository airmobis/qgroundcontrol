#include <GeoWork.h>
#include <QDebug>
#include <QtQml>

// TODO Is this really necessary?
static void initGeoWorkQml() {
    // Make the module exist for the import resolver
    qmlRegisterModule("GeoWork", 1, 0);

    // Expose a singleton named GeoWork inside that module
    qmlRegisterSingletonType<GeoWork>(
        "GeoWork", 1, 0, "GeoWork",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            // WARNING Leave this instance heap-allocated;
            // Qt manually deletes these things.
            static GeoWork* instance { new GeoWork {} };
            return instance;
        }
    );

    qDebug() << "[GeoWork_qmlinit] GeoWork 1.0 registered";
}

Q_COREAPP_STARTUP_FUNCTION(initGeoWorkQml)
