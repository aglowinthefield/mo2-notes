#include "NotesWidget.h"
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QHBoxLayout>
#include <QWebEngineScript>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

NotesWidget::NotesWidget(QWidget* parent)
    : QWidget(parent)
      , m_stackedWidget(new QStackedWidget(this))
      , m_textEdit(new QMarkdownTextEdit(this))
      , m_webView(new QWebEngineView(this))
      , m_layout(new QVBoxLayout(this))
      , m_toggleButton(new QPushButton("View Mode", this))
      , m_saveTimer(new QTimer(this))
      , m_previewTimer(new QTimer(this))
{
    // Create a layout for the toggle button
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_toggleButton);

    // Add widgets to the stacked widget
    m_stackedWidget->addWidget(m_textEdit);
    m_stackedWidget->addWidget(m_webView);

    // Set up the main layout
    m_layout->addLayout(buttonLayout);
    m_layout->addWidget(m_stackedWidget);
    setLayout(m_layout);

    // Auto-save setup
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(2000); // Save 2 seconds after typing stops

    // Preview update timer
    m_previewTimer->setSingleShot(true);
    m_previewTimer->setInterval(500); // Update preview after 500ms of inactivity

    // Set up the WebEngine view
    initWebView();

    // Connect signals
    connect(m_textEdit, &QMarkdownTextEdit::textChanged, this, &NotesWidget::onTextChanged);
    connect(m_saveTimer, &QTimer::timeout, this, &NotesWidget::saveNotes);
    connect(m_toggleButton, &QPushButton::clicked, this, &NotesWidget::toggleViewMode);
    connect(m_previewTimer, &QTimer::timeout, this, &NotesWidget::updatePreview);
}

void NotesWidget::initWebView() const
{
    // Enable basic settings
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);

    // Load the initial HTML with the markdown renderer - HTML string will be provided separately
    m_webView->setHtml(R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Markdown Preview</title>
    <script src="qrc:/resources/marked.min.js"></script>
    <script>
    // Configure marked options
    marked.setOptions({
        breaks: true,           // Add 'br' on single line breaks
        gfm: true,              // Use GitHub Flavored Markdown
        headerIds: true,        // Add IDs to headers
        mangle: false,          // Don't escape HTML
        sanitize: false,        // Don't sanitize HTML
        smartLists: true,       // Use smarter list behavior
        smartypants: true,      // Use smart punctuation
        xhtml: false            // Don't use XHTML closing tags
    });

    function updateContent(markdown) {
        document.getElementById('content').innerHTML = marked.parse(markdown);

        // Add click handler to make links open in external browser
        const links = document.getElementsByTagName('a');
        for (let i = 0; i < links.length; i++) {
            links[i].addEventListener('click', function(e) {
                e.preventDefault();
                window.location.href = 'about:blank#' + this.getAttribute('href');
            });
        }
    }
    </script>
    <style>
        body {
            font-family: system-ui, -apple-system, sans-serif;
            line-height: 1.5;
            padding: 0 20px;
            max-width: 900px;
            margin: 0 auto;
        }
        pre {
            background-color: #f0f0f0;
            padding: 10px;
            border-radius: 5px;
            overflow: auto;
        }
        code {
            background-color: #f0f0f0;
            padding: 2px 4px;
            border-radius: 3px;
            font-family: monospace;
        }
        blockquote {
            border-left: 4px solid #ccc;
            margin-left: 0;
            padding-left: 15px;
            color: #777;
        }
        img {
            max-width: 100%;
        }
        h1, h2, h3 {
            border-bottom: 1px solid #eee;
            padding-bottom: 0.3em;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin: 1em 0;
        }
        table th, table td {
            border: 1px solid #ddd;
            padding: 8px;
        }
        table tr:nth-child(even) {
            background-color: #f8f8f8;
        }
        table th {
            padding-top: 12px;
            padding-bottom: 12px;
            text-align: left;
            background-color: #f0f0f0;
        }
        a {
            color: #0366d6;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
        /* Task lists */
        ul.contains-task-list {
            list-style-type: none;
            padding-left: 0;
        }
        ul.contains-task-list ul.contains-task-list {
            padding-left: 20px;
        }
        .task-list-item-checkbox {
            margin-right: 8px;
        }
        /* Media-specific styles */
        @media (prefers-color-scheme: dark) {
            body {
                background-color: #2d2d2d;
                color: #e0e0e0;
            }
            pre, code {
                background-color: #3c3c3c;
            }
            blockquote {
                border-left-color: #555;
                color: #aaa;
            }
            a {
                color: #6ea8fe;
            }
            h1, h2, h3 {
                border-bottom-color: #444;
            }
            table th, table td {
                border-color: #555;
            }
            table tr:nth-child(even) {
                background-color: #333;
            }
            table th {
                background-color: #3c3c3c;
            }
        }
    </style>
</head>
<body>
    <div id="content"></div>
</body>
</html>)");
}

void NotesWidget::toggleViewMode()
{
    m_isEditMode = !m_isEditMode;

    if (m_isEditMode) {
        m_stackedWidget->setCurrentWidget(m_textEdit);
        m_toggleButton->setText("View Mode");
    } else {
        updatePreview();
        m_stackedWidget->setCurrentWidget(m_webView);
        m_toggleButton->setText("Edit Mode");
    }
}

void NotesWidget::updatePreview() const
{
    QString markdownText = m_textEdit->toPlainText();

    // JavaScript string escaping
    markdownText.replace("\\", "\\\\").replace("'", "\\'").replace("\n", "\\n").replace("\r", "");

    // Execute JavaScript to update the content
    const QString script = QString("updateContent('%1');").arg(markdownText);
    m_webView->page()->runJavaScript(script);
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

    // Start preview timer to update the preview if it's visible
    if (!m_isEditMode) {
        m_previewTimer->start();
    }
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