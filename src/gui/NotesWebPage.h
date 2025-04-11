#pragma once
#include <QWebEnginePage>

class NotesWebPage final : public QWebEnginePage {
    Q_OBJECT
public:
    explicit NotesWebPage(QObject* parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;
};
