// CustomWebPage.cpp
#include "NotesWebPage.h"
#include <QDesktopServices>

NotesWebPage::NotesWebPage(QObject* parent)
    : QWebEnginePage(parent)
{
}

bool NotesWebPage::acceptNavigationRequest(const QUrl& url, NavigationType type, bool)
{
    // Accept internal navigation
    if (url.scheme() == "qrc" || url.scheme() == "about") {
        return true;
    }

    // For external links, open in system browser
    if (type == QWebEnginePage::NavigationTypeLinkClicked) {
        QDesktopServices::openUrl(url);
        return false; // Don't navigate in the WebView
    }

    return true;
}