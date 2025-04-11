#include "UrlRequestInterceptor.h"
#include <QDebug>

UrlRequestInterceptor::UrlRequestInterceptor(QObject* parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
}

void UrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    QUrl url = info.requestUrl();

    // Ignore internal URLs and navigation to about:blank
    if (url.scheme() == "qrc" ||
        url.scheme() == "about" ||
        url.toString() == "about:blank") {
        return;
        }

    // Check if it's a link navigation
    if (info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeMainFrame ||
        info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeSubFrame) {
        qDebug() << "Opening external URL:" << url.toString();

        // Block the navigation in WebView
        info.block(true);

        // Open in system browser
        QDesktopServices::openUrl(url);
        }
}