#include "NotesWidget.h"
#include <QApplication>
#include <QDebug>
#include <QDir>

#include <QHBoxLayout>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include "NotesWebPage.h"

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
    // Create and set custom page
    const auto customPage = new NotesWebPage(m_webView);
    m_webView->setPage(customPage);

    // Enable basic settings
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);

    // Load the stylesheet
    QString styleSheet;
    QFile styleFile(":/resources/notes_style.css");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&styleFile);
        styleSheet = stream.readAll();
        styleFile.close();
    }

    // For external file support, also check profile directory
    if (!m_profilePath.isEmpty()) {
        const QString customCssPath = m_profilePath + "/notes_style.css";
        QFile customFile(customCssPath);
        if (customFile.exists() && customFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&customFile);
            styleSheet = stream.readAll();
            customFile.close();
            qDebug() << "Using custom stylesheet from profile directory";
        }
    }

    // m_webView->page()->setUrlRequestInterceptor(new UrlRequestInterceptor());

    // Load the initial HTML with the markdown renderer - HTML string will be provided separately
    m_webView->setHtml(QString(R"(<!DOCTYPE html>
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
            // This will trigger navigation that our interceptor can catch
            window.location.href = this.href;
        });
    }
}    </script>
    <style>
    %1
    </style>
</head>
<body>
    <div id="content"></div>
</body>
</html>)").arg(styleSheet));
}

void NotesWidget::applyEditorStyles() const
{
    // Default styles
    QString styleSheet = "QMarkdownTextEdit { font-family: monospace; font-size: 10pt; }";

    // Check for custom editor styles in profile directory
    if (!m_profilePath.isEmpty()) {
        QString customCssPath = m_profilePath + "/editor_style.css";
        QFile customFile(customCssPath);
        if (customFile.exists() && customFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&customFile);
            styleSheet = stream.readAll();
            customFile.close();
            qDebug() << "Using custom editor stylesheet from profile directory";
        }
    }

    // Apply the stylesheet to the editor
    m_textEdit->setStyleSheet(styleSheet);
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

    initWebView(); // This loads preview styles
    applyEditorStyles(); // This loads editor styles

}

void NotesWidget::reloadStyles() const
{
    // Reload preview stylesheet
    initWebView();

    // Reload editor stylesheet
    applyEditorStyles();

    // Update preview if needed
    if (!m_isEditMode) {
        updatePreview();
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