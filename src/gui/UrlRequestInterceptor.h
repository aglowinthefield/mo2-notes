// UrlRequestInterceptor.h
#pragma once

#include <QWebEngineUrlRequestInterceptor>
#include <QDesktopServices>

class UrlRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    explicit UrlRequestInterceptor(QObject* parent = nullptr);
    void interceptRequest(QWebEngineUrlRequestInfo& info) override;
};
