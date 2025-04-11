#pragma once

#include <QVBoxLayout>
#include "qmarkdowntextedit.h"

class NotesWidget : public QWidget {
    Q_OBJECT

public:
    explicit NotesWidget(QWidget* parent = nullptr);

private:
    QMarkdownTextEdit* m_textEdit;
    QVBoxLayout* m_layout;
};