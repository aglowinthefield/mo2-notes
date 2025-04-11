#include "NotesWidget.h"
#include <QDir>
#include <QDebug>

NotesWidget::NotesWidget(QWidget* parent)
    : QWidget(parent)
      , m_textEdit(new QMarkdownTextEdit(this))
      , m_layout(new QVBoxLayout(this))
      , m_saveTimer(new QTimer(this))
{
    m_layout->addWidget(m_textEdit);
    setLayout(m_layout);

    // Auto-save setup
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(2000); // Save 2 seconds after typing stops

    // Connect signals
    connect(m_textEdit, &QMarkdownTextEdit::textChanged, this, &NotesWidget::onTextChanged);
    connect(m_saveTimer, &QTimer::timeout, this, &NotesWidget::saveNotes);
}

void NotesWidget::setProfilePath(const QString& profilePath)
{
    m_profilePath = profilePath;

    // Load existing notes if file exists
    const QString notesFilePath = m_profilePath + "/notes.md";
    QFile file(notesFilePath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_textEdit->setPlainText(in.readAll());
        file.close();
        m_isDirty = false;
    }
}

void NotesWidget::onTextChanged()
{
    m_isDirty = true;
    m_saveTimer->start(); // Restart the timer on each text change
}

void NotesWidget::saveNotes()
{
    if (!m_isDirty || m_profilePath.isEmpty()) {
        return;
    }

    const QString notesFilePath = m_profilePath + "/notes.md";
    QFile file(notesFilePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_textEdit->toPlainText();
        file.close();
        m_isDirty = false;
        qDebug() << "Notes saved to:" << notesFilePath;
    } else {
        qWarning() << "Failed to save notes to:" << notesFilePath;
    }
}