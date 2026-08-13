#include "package_details_dialog.h"
#include "../core/alpm_wrapper.h"
#include "../core/package_manager.h"
#include "../utils/logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QIcon>
#include <QFrame>
#include <QScrollArea>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextCursor>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>

PackageDetailsDialog::PackageDetailsDialog(const PackageInfo& info, QWidget* parent)
    : QDialog(parent)
    , m_info(info)
    , m_nameLabel(new QLabel(this))
    , m_versionLabel(new QLabel(this))
    , m_repositoryLabel(new QLabel(this))
    , m_maintainerLabel(new QLabel(this))
    , m_urlLabel(new QLabel(this))
    , m_descriptionText(new QTextEdit(this))
    , m_dependenciesText(new QTextEdit(this))
    , m_lastUpdatedLabel(new QLabel(this))
    , m_installButton(new QPushButton("Install", this))
    , m_uninstallButton(new QPushButton("Uninstall", this))
    , m_launchButton(new QPushButton("Launch", this))
    , m_closeButton(new QPushButton("Close", this))
    , m_statusBadge(new QLabel(this))
    , m_progressBar(new QProgressBar(this))
    , m_progressLabel(new QLabel(this))
    , m_progressWidget(new QWidget(this))
    , m_logViewer(new QTextEdit(this))
    , m_toggleLogButton(new QPushButton("Show Logs", this))
    , m_logWidget(new QWidget(this)) {
    
    setupUi();
    checkInstallStatus();
    updateButtonStates();
    
    // Connect to PackageManager signals
    connect(&PackageManager::instance(), &PackageManager::operationStarted,
            this, &PackageDetailsDialog::onOperationStarted);
    connect(&PackageManager::instance(), &PackageManager::operationOutput,
            this, &PackageDetailsDialog::onOperationOutput);
    connect(&PackageManager::instance(), &PackageManager::operationCompleted,
            this, &PackageDetailsDialog::onOperationCompleted);
    connect(&PackageManager::instance(), &PackageManager::operationError,
            this, &PackageDetailsDialog::onOperationError);
}

void PackageDetailsDialog::setupUi() {
    setWindowTitle("Package Details");
    setMinimumSize(700, 600);
    setModal(true);
    
    // Remove window icon
    setWindowIcon(QIcon());
    
    auto* dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->setSpacing(0);
    
    // Create scroll area for content
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Content widget
    auto* contentWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(20, 30, 20, 20);
    mainLayout->setSpacing(20);
    
    // Header with name and repository badge
    auto* headerLayout = new QHBoxLayout();
    m_nameLabel->setText(m_info.name);
	  m_nameLabel->setObjectName("details-title");
    headerLayout->addWidget(m_nameLabel);
    headerLayout->addStretch();
    
    // Repository badge (member so we can update it later)
    m_repositoryLabel->setText(m_info.repository);
    m_repositoryLabel->setProperty("class", "repo-badge");
    headerLayout->addWidget(m_repositoryLabel);

    // Installed status badge (hidden by default)
    m_statusBadge->setText("Installed");
    m_statusBadge->setProperty("class", "status-badge");
    m_statusBadge->hide();
    headerLayout->addWidget(m_statusBadge);
    
    mainLayout->addLayout(headerLayout);
    
    // Description right under the name
    auto* descLabel = new QLabel(m_info.description, this);
	  descLabel->setObjectName("details-desc");
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);
    
    // Separator line
    auto* line1 = new QFrame(this);
    line1->setFrameShape(QFrame::HLine);
    line1->setObjectName("details-separator");
    mainLayout->addWidget(line1);
    
    // Package Details section - 2x2 grid layout
    auto* detailsTitle = new QLabel("Package Details", this);
    detailsTitle->setObjectName("section-header");
    mainLayout->addWidget(detailsTitle);
    
    auto* infoWidget = new QWidget(this);
    auto* infoGrid = new QGridLayout(infoWidget);
    infoGrid->setSpacing(15);
    infoGrid->setContentsMargins(0, 10, 0, 10);
    infoGrid->setColumnStretch(1, 1);
    infoGrid->setColumnStretch(3, 1);
    
    // Version (top-left)
    auto* versionTitle = new QLabel("Version", this);
    versionTitle->setProperty("class", "detail-label");
    m_versionLabel->setText(m_info.version);
    m_versionLabel->setProperty("class", "detail-value");
    infoGrid->addWidget(versionTitle, 0, 0, Qt::AlignTop);
    infoGrid->addWidget(m_versionLabel, 0, 1);
    
    // Maintainer (top-right)
    if (!m_info.maintainer.isEmpty()) {
        auto* maintainerTitle = new QLabel("Maintainer", this);
        maintainerTitle->setProperty("class", "detail-label");
        m_maintainerLabel->setText(m_info.maintainer);
        m_maintainerLabel->setProperty("class", "detail-value");
        infoGrid->addWidget(maintainerTitle, 0, 2, Qt::AlignTop);
        infoGrid->addWidget(m_maintainerLabel, 0, 3);
    }
    
    // Upstream URL (bottom-left)
    if (!m_info.upstreamUrl.isEmpty()) {
        auto* urlTitle = new QLabel("Upstream URL", this);
        urlTitle->setProperty("class", "detail-label");
		    m_urlLabel->setText(QString("<a href='%1' style='color: #3b82f6;'>%1</a>")
                           .arg(m_info.upstreamUrl));
        m_urlLabel->setOpenExternalLinks(true);
        m_urlLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        m_urlLabel->setWordWrap(true);
        m_urlLabel->setProperty("class", "detail-value");
        infoGrid->addWidget(urlTitle, 1, 0, Qt::AlignTop);
        infoGrid->addWidget(m_urlLabel, 1, 1);
    }
    
    // Last Updated (bottom-right)
    if (!m_info.lastUpdated.isNull()) {
        auto* updatedTitle = new QLabel("Last Updated", this);
        updatedTitle->setProperty("class", "detail-label");
        m_lastUpdatedLabel->setText(m_info.lastUpdated.toString("MMM. d, yyyy, h a"));
        m_lastUpdatedLabel->setProperty("class", "detail-value");
        infoGrid->addWidget(updatedTitle, 1, 2, Qt::AlignTop);
        infoGrid->addWidget(m_lastUpdatedLabel, 1, 3);
    }
    
    mainLayout->addWidget(infoWidget);
    
    // Separator line
    auto* line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine);
    line2->setObjectName("details-separator");
    mainLayout->addWidget(line2);
    
    // Dependencies section
    if (!m_info.dependList.isEmpty()) {
        auto* depsTitle = new QLabel("Dependencies", this);
        depsTitle->setObjectName("section-header");
        mainLayout->addWidget(depsTitle);
        
        m_dependenciesText->setPlainText(m_info.dependList.join("\n"));
        m_dependenciesText->setReadOnly(true);
        m_dependenciesText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		    m_dependenciesText->setObjectName("details-text-area");
        
        // Adjust height to fit all dependencies without scrolling
        QFontMetrics fm(m_dependenciesText->font());
        int lineHeight = fm.lineSpacing();
        int numLines = m_info.dependList.size();
        int contentHeight = (numLines * lineHeight) + 20; // +20 for padding
        m_dependenciesText->setMinimumHeight(contentHeight);
        m_dependenciesText->setMaximumHeight(contentHeight);
        
        mainLayout->addWidget(m_dependenciesText);
        
        // Separator line
        auto* line3 = new QFrame(this);
        line3->setFrameShape(QFrame::HLine);
        line3->setObjectName("details-separator");
		    mainLayout->addWidget(line3);
    }
    
    // Command section
    auto* commandTitle = new QLabel("Command", this);
    commandTitle->setObjectName("section-header");
    mainLayout->addWidget(commandTitle);
    
    // Command buttons and text
    auto* commandWidget = new QWidget(this);
    auto* commandLayout = new QVBoxLayout(commandWidget);
    commandLayout->setContentsMargins(0, 0, 0, 0);
    commandLayout->setSpacing(10);
    
    // Determine which package managers to show
    bool isAUR = (m_info.repository.toLower() == "aur");
    QString command;
    
    if (isAUR) {
        // Show yay and paru for AUR
        auto* buttonLayout = new QHBoxLayout();
        
        auto* yayButton = new QPushButton("yay", this);
        yayButton->setCheckable(true);
        yayButton->setChecked(true);
        yayButton->setProperty("class", "command-selector-btn");

        auto* paruButton = new QPushButton("paru", this);
        paruButton->setCheckable(true);
        paruButton->setProperty("class", "command-selector-btn");

        buttonLayout->addWidget(yayButton);
        buttonLayout->addWidget(paruButton);
        buttonLayout->addStretch();
        
        commandLayout->addLayout(buttonLayout);
        
        command = QString("yay -S %1").arg(m_info.name);
        auto* commandText = new QLabel(this);
        commandText->setText(command);
	      commandText->setObjectName("command-display");
		    commandText->setTextInteractionFlags(Qt::TextSelectableByMouse);
        commandLayout->addWidget(commandText);
        
        // Connect buttons to update command
        connect(yayButton, &QPushButton::clicked, [yayButton, paruButton, commandText, this]() {
            yayButton->setChecked(true);
            paruButton->setChecked(false);
            commandText->setText(QString("yay -S %1").arg(m_info.name));
        });
        
        connect(paruButton, &QPushButton::clicked, [yayButton, paruButton, commandText, this]() {
            paruButton->setChecked(true);
            yayButton->setChecked(false);
            commandText->setText(QString("paru -S %1").arg(m_info.name));
        });
        
    } else {
        // Show pacman for official repos
		    auto* buttonLayout = new QHBoxLayout();

        auto* pacmanButton = new QPushButton("pacman", this);
        pacmanButton->setCheckable(true);
        pacmanButton->setChecked(true);
        pacmanButton->setProperty("class", "command-selector-btn"); 

		    buttonLayout->addWidget(pacmanButton);
		    buttonLayout->addStretch();

		    commandLayout->addLayout(buttonLayout);

        command = QString("sudo pacman -S %1").arg(m_info.name);
        auto* commandText = new QLabel(command, this);
        commandText->setObjectName("command-display");
		    commandText->setTextInteractionFlags(Qt::TextSelectableByMouse);
        commandLayout->addWidget(commandText);
    }
    
    mainLayout->addWidget(commandWidget);
    
    auto* noteLabel = new QLabel("Please ensure your system meets the minimum requirements before installation.", contentWidget);
    noteLabel->setProperty("class", "footer-note");
    noteLabel->setWordWrap(true);
    mainLayout->addWidget(noteLabel);
    
    mainLayout->addStretch();
    
    // Set content widget to scroll area
    contentWidget->setLayout(mainLayout);
    scrollArea->setWidget(contentWidget);
    dialogLayout->addWidget(scrollArea, 1);
    
    // Buttons at the bottom (not scrollable)
    auto* buttonWidget = new QWidget(this);
    auto* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(20, 10, 20, 20);
    buttonLayout->addStretch();
    
    m_installButton->setMinimumWidth(100);
    m_installButton->setMinimumHeight(35);
    m_installButton->setProperty("class", "primary-btn");
    connect(m_installButton, &QPushButton::clicked, this, &PackageDetailsDialog::onInstall);
    buttonLayout->addWidget(m_installButton);
    
    m_uninstallButton->setMinimumWidth(100);
    m_uninstallButton->setMinimumHeight(35);
    m_uninstallButton->setProperty("class", "danger-btn");
    connect(m_uninstallButton, &QPushButton::clicked, this, &PackageDetailsDialog::onUninstall);
    buttonLayout->addWidget(m_uninstallButton);
    
    m_launchButton->setMinimumWidth(100);
    m_launchButton->setMinimumHeight(35);
    connect(m_launchButton, &QPushButton::clicked, this, &PackageDetailsDialog::launchApplication);
    buttonLayout->addWidget(m_launchButton);
    
    m_closeButton->setMinimumWidth(100);
    m_closeButton->setMinimumHeight(35);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(m_closeButton);
    
    dialogLayout->addWidget(buttonWidget, 0);
    
    // Progress bar section (hidden by default)
    auto* progressLayout = new QVBoxLayout(m_progressWidget);
    progressLayout->setContentsMargins(20, 0, 20, 20);
    progressLayout->setSpacing(8);
    
    m_progressLabel->setProperty("class", "detail-label");
	  m_progressLabel->setAlignment(Qt::AlignCenter);
    progressLayout->addWidget(m_progressLabel);
    
    m_progressBar->setMinimumHeight(20);
    m_progressBar->setMaximumHeight(20);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p%");
    m_progressBar->setObjectName("details-progress"); 
	  progressLayout->addWidget(m_progressBar);
    
    // Toggle log button
    m_toggleLogButton->setProperty("class", "link-button");
	  connect(m_toggleLogButton, &QPushButton::clicked, this, &PackageDetailsDialog::toggleLogViewer);
    progressLayout->addWidget(m_toggleLogButton, 0, Qt::AlignCenter);
    
    m_progressWidget->hide();
    dialogLayout->addWidget(m_progressWidget, 0);
    
    // Log viewer section (hidden by default)
    auto* logLayout = new QVBoxLayout(m_logWidget);
    logLayout->setContentsMargins(20, 0, 20, 20);
    logLayout->setSpacing(8);
    
    m_logViewer->setReadOnly(true);
    m_logViewer->setMaximumHeight(200);
    m_logViewer->setObjectName("log-viewer");
	  logLayout->addWidget(m_logViewer);
    
    m_logWidget->hide();
    dialogLayout->addWidget(m_logWidget, 0);
    
    setLayout(dialogLayout);
}

void PackageDetailsDialog::checkInstallStatus() {
    m_isInstalled = AlpmWrapper::instance().isPackageInstalled(m_info.name);
    // Update the installed badge in the header
    if (m_isInstalled) {
        m_statusBadge->show();
    } else {
        m_statusBadge->hide();
    }
}

void PackageDetailsDialog::updateButtonStates() {
    m_installButton->setEnabled(!m_isInstalled);
    m_uninstallButton->setEnabled(m_isInstalled);
    
    // Enable launch button only if installed and has a desktop file
    QString desktopFile = findDesktopFile();
    m_launchButton->setEnabled(m_isInstalled && !desktopFile.isEmpty());
}

void PackageDetailsDialog::onInstall() {
    auto reply = QMessageBox::question(this, "Install Package",
        QString("Are you sure you want to install %1?").arg(m_info.name),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        Logger::info(QString("Installing package: %1").arg(m_info.name));
        
        // Disable buttons during operation
        m_installButton->setEnabled(false);
        m_uninstallButton->setEnabled(false);
        m_launchButton->setEnabled(false);
        m_closeButton->setEnabled(false);
        
        PackageManager::instance().installPackage(m_info.name, m_info.repository);
    }
}

void PackageDetailsDialog::onUninstall() {
    auto reply = QMessageBox::question(this, "Uninstall Package",
        QString("Are you sure you want to uninstall %1?\n\n"
                "This will remove the package and skip dependency checks.")
        .arg(m_info.name),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        Logger::info(QString("Uninstalling package: %1").arg(m_info.name));
        
        // Disable buttons during operation
        m_installButton->setEnabled(false);
        m_uninstallButton->setEnabled(false);
        m_launchButton->setEnabled(false);
        m_closeButton->setEnabled(false);
        
        PackageManager::instance().uninstallPackage(m_info.name, m_info.repository);
    }
}

void PackageDetailsDialog::showProgress(const QString& message) {
    m_progressLabel->setText(message);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressWidget->show();
    m_progressBar->show();
    m_progressLabel->show();
    m_logViewer->clear();
    m_currentOperation = message;
    m_totalPackages = 0;
    m_currentPackage = 0;
}

void PackageDetailsDialog::hideProgress() {
    // Hide the progress bar and label, but keep the widget and toggle button visible
    m_progressBar->hide();
    m_progressLabel->hide();
    // Don't hide m_progressWidget - keeps the toggle button visible
    // Don't hide the log widget or reset log visibility
    // This allows users to review logs after operation completes
}

void PackageDetailsDialog::toggleLogViewer() {
    m_logVisible = !m_logVisible;
    if (m_logVisible) {
        m_logWidget->show();
        m_toggleLogButton->setText("Hide Logs");
    } else {
        m_logWidget->hide();
        m_toggleLogButton->setText("Show Logs");
    }
}

void PackageDetailsDialog::parseProgressOutput(const QString& output) {
    // Parse pacman/yay/paru output for progress information
    
    // Pattern: "downloading..." or "installing..."
    if (output.contains("downloading", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Downloading packages...");
    } else if (output.contains("installing", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Installing packages...");
    } else if (output.contains("building", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Building packages...");
    } else if (output.contains("checking", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Checking dependencies...");
    } else if (output.contains("resolving", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Resolving dependencies...");
    }
    
    // Pattern: "(1/5)" or "( 1/5)" to track package progress
    QRegularExpression packagePattern(R"(\(\s*(\d+)/(\d+)\))");
    auto match = packagePattern.match(output);
    if (match.hasMatch()) {
        m_currentPackage = match.captured(1).toInt();
        m_totalPackages = match.captured(2).toInt();
        
        if (m_totalPackages > 0) {
            int percentage = (m_currentPackage * 100) / m_totalPackages;
            m_progressBar->setValue(percentage);
        }
    }
    
    // Pattern: "[##########] 100%" for download progress
    QRegularExpression percentPattern(R"(\s+(\d+)%\s*)");
    auto percentMatch = percentPattern.match(output);
    if (percentMatch.hasMatch()) {
        int percentage = percentMatch.captured(1).toInt();
        m_progressBar->setValue(percentage);
    }
}

void PackageDetailsDialog::onOperationStarted(const QString& message) {
    showProgress(message);
}

void PackageDetailsDialog::onOperationOutput(const QString& output) {
    if (output.trimmed().isEmpty()) {
        return;
    }
    
    // Add to log viewer
    m_logViewer->append(output.trimmed());
    
    // Auto-scroll to bottom
    QTextCursor cursor = m_logViewer->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logViewer->setTextCursor(cursor);
    
    // Parse output for progress information
    parseProgressOutput(output);
    
    // Force UI update
    m_progressLabel->repaint();
    m_progressBar->repaint();
    QCoreApplication::processEvents();
}

void PackageDetailsDialog::onOperationCompleted(bool success, const QString& message) {
    // Refresh ALPM state so subsequent queries reflect the change
    AlpmWrapper::instance().release();
    AlpmWrapper::instance().initialize();

    // Update status and UI
    checkInstallStatus();
    updateButtonStates();

    hideProgress();

    // Re-enable close button
    m_closeButton->setEnabled(true);

    if (success) {
        QMessageBox::information(this, "Success", message);
    } else {
        QMessageBox::warning(this, "Operation Failed", message);
    }
}

void PackageDetailsDialog::onOperationError(const QString& error) {
    // Refresh ALPM state (best-effort)
    AlpmWrapper::instance().release();
    AlpmWrapper::instance().initialize();

    // Update status and UI
    checkInstallStatus();
    updateButtonStates();

    hideProgress();

    // Re-enable close button
    m_closeButton->setEnabled(true);

    QMessageBox::critical(this, "Error", error);
}

QString PackageDetailsDialog::findDesktopFile() const {
    if (!m_isInstalled) {
        return QString();
    }
    
    // Common locations for .desktop files
    QStringList desktopDirs = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        QDir::homePath() + "/.local/share/applications"
    };
    
    // Try to find desktop file matching the package name
    // Common patterns: package.desktop, package-*.desktop
    QStringList patterns = {
        m_info.name + ".desktop",
        m_info.name + "-*.desktop"
    };
    
    for (const QString& dir : desktopDirs) {
        QDir desktopDir(dir);
        if (!desktopDir.exists()) {
            continue;
        }
        
        // First try exact patterns
        for (const QString& pattern : patterns) {
            QStringList matches = desktopDir.entryList(QStringList() << pattern, QDir::Files);
            if (!matches.isEmpty()) {
                QString desktopFile = desktopDir.absoluteFilePath(matches.first());
                Logger::info(QString("Found desktop file for %1: %2").arg(m_info.name, desktopFile));
                return desktopFile;
            }
        }
        
        // If not found, try fuzzy matching with all .desktop files
        QStringList allDesktopFiles = desktopDir.entryList(QStringList() << "*.desktop", QDir::Files);
        
        // Create regex patterns for fuzzy matching
        // Handle reverse domain names: com.obsproject.Studio.desktop -> obs-studio
        // Handle simple names: code.desktop -> visual-studio-code-bin
        QString packageNameLower = m_info.name.toLower();
        QStringList nameVariants;
        
        // Add the full package name
        nameVariants << packageNameLower;
        
        // Extract keywords from package name (split by dash and underscore)
        QStringList parts = packageNameLower.split(QRegularExpression("[-_]"));
        QStringList significantParts;
        for (const QString& part : parts) {
            if (part.length() > 3) { // Skip very short parts to avoid false matches
                significantParts << part;
            }
        }
        
        // Special handling for common patterns
        QStringList specialVariants;
        if (packageNameLower.contains("visual-studio-code")) {
            specialVariants << "vscode" << "code";
        } else if (packageNameLower == "obs-studio") {
            // For obs-studio, look for obsproject specifically
            specialVariants << "obsproject";
        }
        
        // Try special variants first (highest priority)
        for (const QString& variant : specialVariants) {
            for (const QString& desktopFileName : allDesktopFiles) {
                QString fileNameLower = desktopFileName.toLower();
                
                if (fileNameLower.contains(variant)) {
                    QString desktopFile = desktopDir.absoluteFilePath(desktopFileName);
                    
                    if (verifyDesktopFile(desktopFile, nameVariants + specialVariants + significantParts)) {
                        Logger::info(QString("Found desktop file for %1 via special match: %2")
                                    .arg(m_info.name, desktopFile));
                        return desktopFile;
                    }
                }
            }
        }
        
        // Try full package name match
        for (const QString& desktopFileName : allDesktopFiles) {
            QString fileNameLower = desktopFileName.toLower();
            
            if (fileNameLower.contains(packageNameLower)) {
                QString desktopFile = desktopDir.absoluteFilePath(desktopFileName);
                
                if (verifyDesktopFile(desktopFile, nameVariants + specialVariants + significantParts)) {
                    Logger::info(QString("Found desktop file for %1 via full name match: %2")
                                .arg(m_info.name, desktopFile));
                    return desktopFile;
                }
            }
        }
        
        // Finally, try matching individual significant parts (but verify carefully)
        for (const QString& part : significantParts) {
            for (const QString& desktopFileName : allDesktopFiles) {
                QString fileNameLower = desktopFileName.toLower();
                
                // Use word boundary-like matching: ensure part is not in the middle of another word
                // Check if part appears as a separate component (after . or at start, before . or -)
                QRegularExpression wordBoundary(QString("(^|[._-])%1([._-]|$)").arg(QRegularExpression::escape(part)));
                
                if (wordBoundary.match(fileNameLower).hasMatch()) {
                    QString desktopFile = desktopDir.absoluteFilePath(desktopFileName);
                    
                    if (verifyDesktopFile(desktopFile, nameVariants + specialVariants + significantParts)) {
                        Logger::info(QString("Found desktop file for %1 via word match: %2")
                                    .arg(m_info.name, desktopFile));
                        return desktopFile;
                    }
                }
            }
        }
    }
    
    Logger::debug(QString("No desktop file found for package: %1").arg(m_info.name));
    return QString();
}

bool PackageDetailsDialog::verifyDesktopFile(const QString& desktopFilePath,
                                             const QStringList& nameVariants) const {
    QFile file(desktopFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // Check if Exec line contains any of our name variants
    QRegularExpression execPattern(R"(^Exec=(.*)$)", QRegularExpression::MultilineOption);
    auto match = execPattern.match(content);
    
    if (match.hasMatch()) {
        QString execLine = match.captured(1).toLower();
        
        // Check if any variant appears in the Exec line
        for (const QString& variant : nameVariants) {
            if (execLine.contains(variant)) {
                return true;
            }
        }
    }
    
    // Also check Name field as a fallback
    QRegularExpression namePattern(R"(^Name=(.*)$)", QRegularExpression::MultilineOption);
    match = namePattern.match(content);
    
    if (match.hasMatch()) {
        QString nameField = match.captured(1).toLower();
        
        for (const QString& variant : nameVariants) {
            if (nameField.contains(variant)) {
                return true;
            }
        }
    }
    
    return false;
}

void PackageDetailsDialog::launchApplication() {
    QString desktopFile = findDesktopFile();
    
    if (desktopFile.isEmpty()) {
        QMessageBox::warning(this, "Launch Failed",
            QString("Could not find a desktop file for %1.\n"
                    "This application may not have a graphical interface or "
                    "may need to be launched from the terminal.").arg(m_info.name));
        return;
    }
    
    // Launch the application using gtk-launch or similar
    QProcess* process = new QProcess(this);
    
    // Try gtk-launch first (works on most desktop environments)
    QString baseName = QFileInfo(desktopFile).fileName();
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process, baseName](int exitCode, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        
        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            Logger::error(QString("Failed to launch application: %1").arg(baseName));
            QMessageBox::warning(this, "Launch Failed",
                QString("Failed to launch %1.\n"
                        "Exit code: %2").arg(m_info.name).arg(exitCode));
        } else {
            Logger::info(QString("Successfully launched: %1").arg(baseName));
        }
    });
    
    // Try gtk-launch first
    process->start("gtk-launch", QStringList() << baseName);
    
    // If gtk-launch doesn't start, try alternative methods
    if (!process->waitForStarted(1000)) {
        // Try dex (Desktop Entry Execution)
        process->start("dex", QStringList() << desktopFile);
        
        if (!process->waitForStarted(1000)) {
            // Try exo-open (XFCE)
            process->start("exo-open", QStringList() << desktopFile);
            
            if (!process->waitForStarted(1000)) {
                // Last resort: try to parse and execute the Exec line
                process->deleteLater();
                QMessageBox::warning(this, "Launch Failed",
                    "Could not find a suitable desktop file launcher.\n"
                    "Please install gtk-launch, dex, or exo-open.");
                Logger::error("No desktop file launcher available");
            }
        }
    }
}
