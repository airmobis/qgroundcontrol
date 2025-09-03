#include "UrlInterceptor.h"

#include <QFile>
#include <QUrl>

QUrl UrlInterceptor::intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) {
    switch (type) {
        using enum QQmlAbstractUrlInterceptor::DataType;

    case QmlFile:
    case UrlString:
        if (url.scheme() == QStringLiteral("qrc")) {
            const QString origPath    = url.path();
            const QString overrideRes = QStringLiteral(":/Custom%1").arg(origPath);

            if (QFile::exists(overrideRes)) {
                const QString relPath = overrideRes.mid(2);

                QUrl result;
                result.setScheme(QStringLiteral("qrc"));
                result.setPath('/' + relPath);

                return result;
            }
        }
        break;
    default:
        break;
    }

    return url;
}
