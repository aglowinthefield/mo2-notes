#pragma once

#include "qmarkdowntextedit.h"
#include <QFile>
#include <QVBoxLayout>

class NotesWidget final : public QWidget {
    Q_OBJECT

public:
    explicit NotesWidget(QWidget* parent = nullptr);

    void setProfilePath(const QString& profilePath);

private slots:
    void saveNotes();

    void onTextChanged();

private:
    QMarkdownTextEdit* m_textEdit;
    QVBoxLayout* m_layout;
    QString m_profilePath;
    QTimer* m_saveTimer;
    bool m_isDirty = false;
};