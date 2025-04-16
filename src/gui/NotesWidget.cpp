#include "NotesWidget.h"
#include <QApplication>
#include <QDebug>
#include <QDir>

#include <QHBoxLayout>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineSettings>
#include <QJsonDocument>
#include <QWebEngineView>
#include <QJsonObject>
#include "NotesWebPage.h"

#include <qstyle.h>

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

    // setupMarkdownHighlighter();

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
    // Additional editor settings
    m_textEdit->setLineNumberEnabled(true);
    m_textEdit->setHighlightingEnabled(true);
    // Update the editor's appearance
    // m_textEdit->style()->unpolish(m_textEdit);
    // m_textEdit->style()->polish(m_textEdit);

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

void NotesWidget::setupMarkdownHighlighter() const
{
    const auto highlighter = m_textEdit->highlighter();
    if (!highlighter) {
        qWarning() << "No highlighter found on text edit";
        return;
    }

    QHash<MarkdownHighlighter::HighlighterState, QTextCharFormat> formats;

    // Check for custom markdown style file first
    if (m_profilePath.isEmpty()) {
        qWarning() << "No profile path specified";
        return;
    }

    const QString stylePath = m_profilePath + "/markdown_style.json";

    if (QFile styleFile(stylePath); styleFile.exists() && styleFile.open(QIODevice::ReadOnly)) {
        const QJsonDocument doc = QJsonDocument::fromJson(styleFile.readAll());
        styleFile.close();

        if (doc.isObject() && false) {
            const QJsonObject styleObj = doc.object();

            // Parse each style element from JSON
            for (auto it = styleObj.begin(); it != styleObj.end(); ++it) {
                const QString key           = it.key();
                const QJsonObject formatObj = it.value().toObject();

                // Find the corresponding highlighter state
                int stateValue           = -1;
                const QMetaEnum metaEnum = QMetaEnum::fromType<MarkdownHighlighter::HighlighterState>();
                for (int i = 0; i < metaEnum.keyCount(); ++i) {
                    if (key == metaEnum.key(i)) {
                        stateValue = metaEnum.value(i);
                        break;
                    }
                }

                if (stateValue != -1) {
                    QTextCharFormat format;
                    if (formatObj.contains("foreground"))
                        format.setForeground(QColor(formatObj["foreground"].toString()));
                    if (formatObj.contains("background"))
                        format.setBackground(QColor(formatObj["background"].toString()));
                    if (formatObj.contains("bold"))
                        format.setFontWeight(formatObj["bold"].toBool() ? QFont::Bold : QFont::Normal);
                    if (formatObj.contains("italic"))
                        format.setFontItalic(formatObj["italic"].toBool());
                    if (formatObj.contains("fontSize"))
                        format.setFontPointSize(formatObj["fontSize"].toDouble());
                    if (formatObj.contains("fontFamily"))
                        format.setFontFamilies({ formatObj["fontFamily"].toString() });

                    formats[static_cast<MarkdownHighlighter::HighlighterState>(stateValue)] = format;
                }
            }
        }
    } else {
        // Create default style file if it doesn't exist
        createDefaultMarkdownStyle(stylePath);
    }

    // If no custom styles were loaded, use defaults
    if (formats.isEmpty()) {
        // Basic text format
        QTextCharFormat normalFormat;
        normalFormat.setForeground(QColor("#C7CCD1")); // base05 - default text
        formats[MarkdownHighlighter::NoState] = normalFormat;

        // Headers
        QTextCharFormat h1Format;
        h1Format.setForeground(QColor("#95C7AE")); // base0B - green
        h1Format.setFontWeight(QFont::Bold);
        h1Format.setFontPointSize(24);
        formats[MarkdownHighlighter::H1] = h1Format;

        QTextCharFormat h2Format;
        h2Format.setForeground(QColor("#95C7AE")); // base0B - green
        h2Format.setFontWeight(QFont::Bold);
        h2Format.setFontPointSize(20);
        formats[MarkdownHighlighter::H2] = h2Format;

        QTextCharFormat h3Format;
        h3Format.setForeground(QColor("#95C7AE")); // base0B - green
        h3Format.setFontWeight(QFont::Bold);
        h3Format.setFontPointSize(16);
        formats[MarkdownHighlighter::H3] = h3Format;

        // Emphasis
        QTextCharFormat emphasisFormat;
        emphasisFormat.setForeground(QColor("#ADB3BA")); // base04 - light gray
        emphasisFormat.setFontItalic(true);
        formats[MarkdownHighlighter::Italic] = emphasisFormat;

        // Strong
        QTextCharFormat strongFormat;
        strongFormat.setForeground(QColor("#DFE2E5")); // base06 - lighter gray
        strongFormat.setFontWeight(QFont::Bold);
        formats[MarkdownHighlighter::Bold] = strongFormat;

        // Code formats
        QTextCharFormat codeFormat;
        codeFormat.setForeground(QColor("#C795AE")); // base0E - purple
        codeFormat.setFontFamilies({ "Consolas" });
        formats[MarkdownHighlighter::InlineCodeBlock] = codeFormat;

        QTextCharFormat codeBlockFormat;
        codeBlockFormat.setForeground(QColor("#C795AE")); // base0E - purple
        codeBlockFormat.setBackground(QColor("#393F45")); // base01 - darker background
        codeBlockFormat.setFontFamilies({ "Consolas" });
        formats[MarkdownHighlighter::CodeBlock] = codeBlockFormat;

        // Links
        QTextCharFormat linkFormat;
        linkFormat.setForeground(QColor("#AE95C7")); // base0D - blue
        linkFormat.setFontUnderline(true);
        formats[MarkdownHighlighter::Link] = linkFormat;
    }

    // Apply the formats and force refresh
    MarkdownHighlighter::setTextFormats(formats);
    highlighter->rehighlight();
}

void NotesWidget::createDefaultMarkdownStyle(const QString& path)
{
    if (QFile file(path); file.open(QIODevice::WriteOnly)) {
        QJsonObject styleObj;

        // Headers
        QJsonObject h1;
        h1["foreground"] = "#0077CC";
        h1["bold"]       = true;
        h1["fontSize"]   = 24;
        styleObj["H1"]   = h1;

        QJsonObject h2;
        h2["foreground"] = "#0077CC";
        h2["bold"]       = true;
        h2["fontSize"]   = 20;
        styleObj["H2"]   = h2;

        // Code
        QJsonObject code;
        code["foreground"]          = "#D14";
        code["fontFamily"]          = "Consolas";
        styleObj["InlineCodeBlock"] = code;

        QJsonObject codeBlock;
        codeBlock["foreground"] = "#333333";
        codeBlock["background"] = "#F5F5F5";
        codeBlock["fontFamily"] = "Consolas";
        styleObj["CodeBlock"]   = codeBlock;

        // Links
        QJsonObject link;
        link["foreground"] = "#0077CC";
        link["underline"]  = true;
        styleObj["Link"]   = link;

        // Write the JSON to file
        const QJsonDocument doc(styleObj);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();

        qDebug() << "Created default markdown style file at:" << path;
    } else {
        qWarning() << "Failed to create default markdown style file at:" << path;
    }
}

void NotesWidget::setProfilePath(const QString& profilePath)
{
    m_profilePath = profilePath;

    // Create default style files if needed
    const QString markdownStylePath = m_profilePath + "/markdown_style.json";
    createDefaultMarkdownStyle(markdownStylePath);

    // Load notes content
    const QString notesFilePath = m_profilePath + "/notes.md";
    QFile file(notesFilePath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_textEdit->setPlainText(in.readAll());
        file.close();
        m_isDirty = false;
    }

    // Apply styles to components
    initWebView(); // This loads preview styles
    applyEditorStyles(); // This loads editor styles
    setupMarkdownHighlighter();
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