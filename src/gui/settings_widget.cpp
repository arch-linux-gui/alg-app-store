#include "settings_widget.h"
#include "../utils/logger.h"
#include "../core/alpm_wrapper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QProcess>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , m_repositoryGroup(nullptr)
    , m_coreRepoCheckbox(nullptr)
    , m_extraRepoCheckbox(nullptr)
    , m_multilibRepoCheckbox(nullptr)
    , m_applyButton(nullptr)
    , m_revertButton(nullptr)
    , m_statusLabel(nullptr)
    , m_originalMultilibState(false) {
    
    setupUi();
    loadCurrentSettings();
    
    Logger::info("SettingsWidget created successfully");
}

void SettingsWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    
    // Title
    auto* titleLabel = new QLabel("Settings", this);
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Repository Settings
    createRepositorySettings();
    mainLayout->addWidget(m_repositoryGroup);
    
    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("QLabel { color: #0066cc; padding: 10px; }");
    m_statusLabel->hide();
    mainLayout->addWidget(m_statusLabel);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_revertButton = new QPushButton("Revert", this);
    m_revertButton->setMinimumWidth(100);
    m_revertButton->setEnabled(false);
    connect(m_revertButton, &QPushButton::clicked, this, &SettingsWidget::onRevertClicked);
    buttonLayout->addWidget(m_revertButton);
    
    m_applyButton = new QPushButton("Apply", this);
    m_applyButton->setMinimumWidth(100);
    m_applyButton->setEnabled(false);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsWidget::onApplyClicked);
    buttonLayout->addWidget(m_applyButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Add stretch at the bottom
    mainLayout->addStretch();
    
    setLayout(mainLayout);
}

void SettingsWidget::createRepositorySettings() {
    m_repositoryGroup = new QGroupBox("Package Repositories", this);
    auto* repoLayout = new QVBoxLayout(m_repositoryGroup);
    
    // Description
    auto* descLabel = new QLabel(
        "Select which package repositories to use for searching and installing packages.\n"
        "Core and Extra repositories are required and cannot be disabled.", 
        this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { color: #666; margin-bottom: 10px; }");
    repoLayout->addWidget(descLabel);
    
    // Core repository (always enabled, cannot be disabled)
    m_coreRepoCheckbox = new QCheckBox("Core - Essential system packages", this);
    m_coreRepoCheckbox->setChecked(true);
    m_coreRepoCheckbox->setEnabled(false);
    m_coreRepoCheckbox->setToolTip("Core repository is required and cannot be disabled");
    repoLayout->addWidget(m_coreRepoCheckbox);
    
    // Extra repository (always enabled, cannot be disabled)
    m_extraRepoCheckbox = new QCheckBox("Extra - Additional official packages", this);
    m_extraRepoCheckbox->setChecked(true);
    m_extraRepoCheckbox->setEnabled(false);
    m_extraRepoCheckbox->setToolTip("Extra repository is required and cannot be disabled");
    repoLayout->addWidget(m_extraRepoCheckbox);
    
    // Multilib repository (optional, can be enabled/disabled)
    m_multilibRepoCheckbox = new QCheckBox("Multilib - 32-bit packages on x86_64", this);
    m_multilibRepoCheckbox->setToolTip(
        "Enable multilib repository for 32-bit applications on 64-bit systems.\n"
        "This modifies /etc/pacman.conf and requires administrator privileges.");
    connect(m_multilibRepoCheckbox, &QCheckBox::checkStateChanged, 
            this, &SettingsWidget::onSettingsChanged);
    repoLayout->addWidget(m_multilibRepoCheckbox);
    
    // Info label
    auto* infoLabel = new QLabel(
        "Note: Changes to repositories require modifying system configuration files "
        "and may require administrator privileges.", 
        this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: #888; font-style: italic; margin-top: 10px; }");
    repoLayout->addWidget(infoLabel);
    
    m_repositoryGroup->setLayout(repoLayout);
}

void SettingsWidget::loadCurrentSettings() {
    // Check if multilib is currently enabled
    bool multilibEnabled = isMultilibEnabledInPacmanConf();
    m_multilibRepoCheckbox->setChecked(multilibEnabled);
    m_originalMultilibState = multilibEnabled;
    
    Logger::info(QString("Loaded settings: multilib=%1").arg(multilibEnabled ? "enabled" : "disabled"));
}

bool SettingsWidget::isMultilibEnabledInPacmanConf() const {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QTextStream in(&file);
    bool inMultilibSection = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Check for [multilib] section header
        if (line == "[multilib]") {
            inMultilibSection = true;
            continue;
        }
        
        // If we found [multilib] section, check if it's not commented
        if (inMultilibSection && !line.isEmpty() && !line.startsWith("#")) {
            // If we find Include directive, multilib is enabled
            if (line.startsWith("Include")) {
                file.close();
                return true;
            }
        }
        
        // If we hit another section, stop
        if (inMultilibSection && line.startsWith("[") && line != "[multilib]") {
            break;
        }
    }
    
    file.close();
    return false;
}

bool SettingsWidget::enableMultilibInPacmanConf() {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QStringList lines;
    QTextStream in(&file);
    bool multilibSectionFound = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        
        // Check if this is a commented [multilib] section
        if (line.trimmed() == "#[multilib]") {
            lines.append("[multilib]");
            multilibSectionFound = true;
        } 
        // Check if the Include line in multilib section is commented
        else if (multilibSectionFound && line.trimmed().startsWith("#Include") && 
                 line.contains("mirrorlist")) {
            lines.append(line.mid(1)); // Remove the # comment character
            multilibSectionFound = false; // Reset flag after processing
        }
        else {
            lines.append(line);
        }
    }
    file.close();
    
    // Write back to file using pkexec for elevated privileges
    QString tempFile = "/tmp/pacman.conf.tmp";
    QFile temp(tempFile);
    if (!temp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::error("Failed to create temporary file");
        return false;
    }
    
    QTextStream out(&temp);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    temp.close();
    
    // Use pkexec to copy the file with elevated privileges
    QProcess process;
    process.start("pkexec", QStringList() << "cp" << tempFile << "/etc/pacman.conf");
    process.waitForFinished(30000); // 30 second timeout
    
    if (process.exitCode() != 0) {
        Logger::error("Failed to update pacman.conf with elevated privileges");
        QFile::remove(tempFile);
        return false;
    }
    
    QFile::remove(tempFile);
    Logger::info("Successfully enabled multilib repository");
    return true;
}

bool SettingsWidget::disableMultilibInPacmanConf() {
    QFile file("/etc/pacman.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::error("Failed to open /etc/pacman.conf for reading");
        return false;
    }
    
    QStringList lines;
    QTextStream in(&file);
    bool inMultilibSection = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QString trimmedLine = line.trimmed();
        
        // Check if this is [multilib] section
        if (trimmedLine == "[multilib]") {
            lines.append("#[multilib]");
            inMultilibSection = true;
        }
        // Check if we're in multilib section and this is the Include line
        else if (inMultilibSection && trimmedLine.startsWith("Include") && 
                 trimmedLine.contains("mirrorlist")) {
            lines.append("#" + line);
            inMultilibSection = false;
        }
        // Check if we hit another section
        else if (trimmedLine.startsWith("[") && trimmedLine != "[multilib]") {
            lines.append(line);
            inMultilibSection = false;
        }
        else {
            lines.append(line);
        }
    }
    file.close();
    
    // Write back to file using pkexec for elevated privileges
    QString tempFile = "/tmp/pacman.conf.tmp";
    QFile temp(tempFile);
    if (!temp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Logger::error("Failed to create temporary file");
        return false;
    }
    
    QTextStream out(&temp);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    temp.close();
    
    // Use pkexec to copy the file with elevated privileges
    QProcess process;
    process.start("pkexec", QStringList() << "cp" << tempFile << "/etc/pacman.conf");
    process.waitForFinished(30000); // 30 second timeout
    
    if (process.exitCode() != 0) {
        Logger::error("Failed to update pacman.conf with elevated privileges");
        QFile::remove(tempFile);
        return false;
    }
    
    QFile::remove(tempFile);
    Logger::info("Successfully disabled multilib repository");
    return true;
}

void SettingsWidget::onSettingsChanged() {
    // Enable apply and revert buttons when settings change
    bool hasChanges = (m_multilibRepoCheckbox->isChecked() != m_originalMultilibState);
    m_applyButton->setEnabled(hasChanges);
    m_revertButton->setEnabled(hasChanges);
    m_statusLabel->hide();
}

void SettingsWidget::onApplyClicked() {
    bool currentMultilibState = m_multilibRepoCheckbox->isChecked();
    bool success = false;
    
    if (currentMultilibState != m_originalMultilibState) {
        // Show confirmation dialog
        QString message;
        if (currentMultilibState) {
            message = "This will enable the multilib repository by modifying /etc/pacman.conf.\n"
                     "You will be prompted for administrator privileges.\n\n"
                     "After enabling, you should run 'sudo pacman -Sy' to sync the databases.\n\n"
                     "Do you want to continue?";
        } else {
            message = "This will disable the multilib repository by modifying /etc/pacman.conf.\n"
                     "You will be prompted for administrator privileges.\n\n"
                     "Do you want to continue?";
        }
        
        auto reply = QMessageBox::question(this, "Confirm Repository Change", 
                                          message,
                                          QMessageBox::Yes | QMessageBox::No);
        
        if (reply != QMessageBox::Yes) {
            return;
        }
        
        // Apply the change
        if (currentMultilibState) {
            success = enableMultilibInPacmanConf();
        } else {
            success = disableMultilibInPacmanConf();
        }
        
        if (success) {
            m_originalMultilibState = currentMultilibState;
            m_statusLabel->setText("Settings applied successfully! Please sync package databases.");
            m_statusLabel->setStyleSheet("QLabel { color: #00aa00; padding: 10px; font-weight: bold; }");
            m_statusLabel->show();
            
            m_applyButton->setEnabled(false);
            m_revertButton->setEnabled(false);
            
            // Emit signal to notify other widgets
            emit multilibStatusChanged(currentMultilibState);
            
            // Suggest database sync
            auto reply = QMessageBox::question(this, "Sync Package Database",
                                              "Would you like to sync the package database now?\n"
                                              "(This will run 'pkexec pacman -Sy')",
                                              QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                QProcess process;
                m_statusLabel->setText("Syncing package databases...");
                process.start("pkexec", QStringList() << "pacman" << "-Sy");
                process.waitForFinished(60000); // 60 second timeout
                
                if (process.exitCode() == 0) {
                    m_statusLabel->setText("Package databases synced successfully!");
                    Logger::info("Package databases synced after repository change");
                    
                    // Refresh ALPM databases to pick up the new repository
                    AlpmWrapper::instance().refreshDatabases();
                } else {
                    m_statusLabel->setText("Failed to sync package databases. Please run 'sudo pacman -Sy' manually.");
                    m_statusLabel->setStyleSheet("QLabel { color: #aa0000; padding: 10px; }");
                }
            } else {
                // Even if they don't sync now, refresh ALPM to detect the new repo configuration
                AlpmWrapper::instance().refreshDatabases();
            }
        } else {
            m_statusLabel->setText("Failed to apply settings. Please check permissions.");
            m_statusLabel->setStyleSheet("QLabel { color: #aa0000; padding: 10px; }");
            m_statusLabel->show();
        }
    }
}

void SettingsWidget::onRevertClicked() {
    // Revert to original state
    m_multilibRepoCheckbox->setChecked(m_originalMultilibState);
    m_applyButton->setEnabled(false);
    m_revertButton->setEnabled(false);
    m_statusLabel->setText("Changes reverted");
    m_statusLabel->setStyleSheet("QLabel { color: #0066cc; padding: 10px; }");
    m_statusLabel->show();
    
    Logger::info("Settings reverted to original state");
}

bool SettingsWidget::isMultilibEnabled() const {
    return m_multilibRepoCheckbox->isChecked() && 
           (m_multilibRepoCheckbox->isChecked() == m_originalMultilibState);
}

void SettingsWidget::applySettings() {
    onApplyClicked();
}
