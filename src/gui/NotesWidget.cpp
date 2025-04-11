#include "NotesWidget.h"

NotesWidget::NotesWidget(QWidget* parent)
    : QWidget(parent)
    , m_textEdit(new QMarkdownTextEdit(this))
    , m_layout(new QVBoxLayout(this))
{
    m_layout->addWidget(m_textEdit);
    setLayout(m_layout);
}
