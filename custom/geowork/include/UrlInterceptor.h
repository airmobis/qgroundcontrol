#pragma once

#include <qqmlabstracturlinterceptor.h>

class UrlInterceptor : public QQmlAbstractUrlInterceptor {
public:
    UrlInterceptor() = default;

    QUrl intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) final;
};
