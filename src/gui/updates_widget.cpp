#include "updates_widget.h"
#include "../core/alpm_wrapper.h"
#include "../core/aur_helper.h"
#include "../core/package_manager.h"
#include "../utils/logger.h"
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextCursor>
#include <QtConcurrent>
#include <QStyle>

class UpdateItem : public QWidget {
    Q_OBJECT
    
public:
    UpdateItem(const UpdateInfo& info, QWidget* parent = nullptr)
        : QWidget(parent), m_info(info) {

        // Required for the app-wide stylesheet to paint this custom widget's background
        setAttribute(Qt::WA_StyledBackground, true);

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);
        
        auto* infoLayout = new QVBoxLayout();
        
        auto* nameLabel = new QLabel(m_info.name, this);
        auto nameFont = nameLabel->font();
        nameFont.setBold(true);
        nameFont.setPointSize(12);
        nameLabel->setFont(nameFont);
        infoLayout->addWidget(nameLabel);
        
        auto* versionLabel = new QLabel(
            QString("%1 → %2").arg(m_info.oldVersion, m_info.newVersion), this);
        versionLabel->setProperty("class", "secondary-text");
	      infoLayout->addWidget(versionLabel);
        
        auto* repoLabel = new QLabel(m_info.repository, this);
        repoLabel->setProperty("class", "dim-text");
	      infoLayout->addWidget(repoLabel);
        
        layout->addLayout(infoLayout);
        layout->addStretch();
        
        if (m_info.downloadSize > 0) {
            auto* sizeLabel = new QLabel(formatSize(m_info.downloadSize), this);
            sizeLabel->setProperty("class", "secondary-text");
		        layout->addWidget(sizeLabel);
        }
        
        auto* updateButton = new QPushButton("Update", this);
        updateButton->setMinimumWidth(100);
        connect(updateButton, &QPushButton::clicked, [this]() {
            emit updateRequested(m_info.name);
        });
        layout->addWidget(updateButton);
        
        setLayout(layout);
        setProperty("class", "update-item");
    }
    
signals:
    void updateRequested(const QString& packageName);
    
private:
    UpdateInfo m_info;
    
    QString formatSize(qint64 bytes) {
        const qint64 KB = 1024;
        const qint64 MB = KB * 1024;
        const qint64 GB = MB * 1024;
        
        if (bytes >= GB) {
            return QString("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
        } else if (bytes >= MB) {
            return QString("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 1);
        } else if (bytes >= KB) {
            return QString("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 0);
        }
        return QString("%1 B").arg(bytes);
    }
};

UpdatesWidget::UpdatesWidget(QWidget* parent)
    : QWidget(parent)
    , m_searchInput(new QLineEdit(this))
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_contentLayout(new QVBoxLayout(m_contentWidget))
    , m_statusLabel(new QLabel(this))
    , m_countLabel(new QLabel(this))
    , m_updateAllButton(new QPushButton("Update All", this))
    , m_checkButton(new QPushButton("Check for Updates", this))
    , m_progressWidget(new QWidget(this))
    , m_progressBar(new QProgressBar(this))
    , m_progressLabel(new QLabel(this))
    , m_toggleLogButton(new QPushButton("Show Logs", this))
    , m_logWidget(new QWidget(this))
    , m_logViewer(new QTextEdit(this)) {
    
    setupUi();
    
    // Connect to PackageManager signals
    connect(&PackageManager::instance(), &PackageManager::operationStarted,
            this, &UpdatesWidget::onOperationStarted);
    connect(&PackageManager::instance(), &PackageManager::operationOutput,
            this, &UpdatesWidget::onOperationOutput);
    connect(&PackageManager::instance(), &PackageManager::operationCompleted,
            this, &UpdatesWidget::onOperationCompleted);
    connect(&PackageManager::instance(), &PackageManager::operationError,
            this, &UpdatesWidget::onOperationError);
}

void UpdatesWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Header
    auto* headerLayout = new QHBoxLayout();
    
    auto* titleLabel = new QLabel("Available Updates", this);
    titleLabel->setObjectName("view-title");
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    m_countLabel->setProperty("class", "secondary-text");
	  headerLayout->addWidget(m_countLabel);
    
    m_checkButton->setMinimumHeight(35);
    connect(m_checkButton, &QPushButton::clicked, this, &UpdatesWidget::checkForUpdates);
    headerLayout->addWidget(m_checkButton);
    
    m_updateAllButton->setMinimumHeight(35);
    m_updateAllButton->setMinimumWidth(120);
    m_updateAllButton->setProperty("class", "primary-btn");
    m_updateAllButton->setEnabled(false);
    connect(m_updateAllButton, &QPushButton::clicked, this, &UpdatesWidget::onUpdateAll);
    headerLayout->addWidget(m_updateAllButton);
    
    mainLayout->addLayout(headerLayout);
    
    // Search bar
    m_searchInput->setPlaceholderText("Search updates...");
    m_searchInput->setMinimumHeight(35);
    m_searchInput->setClearButtonEnabled(true);
    m_searchInput->setEnabled(false); // Disabled until updates are loaded
    connect(m_searchInput, &QLineEdit::textChanged, this, &UpdatesWidget::onSearchTextChanged);
    mainLayout->addWidget(m_searchInput);
    
    // Status label
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setText("Click 'Check for Updates' to scan for available updates");
    mainLayout->addWidget(m_statusLabel);
    
    // Updates area
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_contentLayout->setSpacing(5);
    m_contentLayout->setContentsMargins(10, 10, 10, 10);
    m_contentLayout->addStretch();
    
    mainLayout->addWidget(m_scrollArea);
    
    // Progress bar section (hidden by default)
    auto* progressLayout = new QVBoxLayout(m_progressWidget);
	  progressLayout->setContentsMargins(20, 0, 20, 20);
    progressLayout->setSpacing(8);
    
    m_progressLabel->setProperty("class", "dim-text");
	  m_progressLabel->setAlignment(Qt::AlignCenter);
    progressLayout->addWidget(m_progressLabel);
    
    m_progressBar->setMinimumHeight(20);
    m_progressBar->setMaximumHeight(20);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p%");
    m_progressBar->setObjectName("operation-progress");
	  progressLayout->addWidget(m_progressBar);
    
    // Toggle log button
    m_toggleLogButton->setProperty("class", "link-button");
	  connect(m_toggleLogButton, &QPushButton::clicked, this, &UpdatesWidget::toggleLogViewer);
    progressLayout->addWidget(m_toggleLogButton, 0, Qt::AlignCenter);
    
    m_progressWidget->hide();
    mainLayout->addWidget(m_progressWidget, 0);
    
    // Log viewer section (hidden by default)
    auto* logLayout = new QVBoxLayout(m_logWidget);
    logLayout->setContentsMargins(20, 0, 20, 20);
    logLayout->setSpacing(8);
    
    m_logViewer->setReadOnly(true);
    m_logViewer->setMaximumHeight(200);
    m_logViewer->setObjectName("log-viewer"); 
    logLayout->addWidget(m_logViewer);
    
    m_logWidget->hide();
    mainLayout->addWidget(m_logWidget, 0);
    
    setLayout(mainLayout);
}

void UpdatesWidget::checkForUpdates() {
    m_statusLabel->setText("Checking for updates...");
    m_statusLabel->show();
    m_checkButton->setEnabled(false);
    m_checkButton->setText("Checking...");
    m_updateAllButton->setEnabled(false);
    
    clearUpdates();
    
    (void)QtConcurrent::run([this]() {
        auto updates = AlpmWrapper::instance().getAvailableUpdates();
        
        // Also check AUR updates
        AurHelper aurHelper;
        auto aurUpdates = aurHelper.checkAurUpdates();
        updates.append(aurUpdates);
        
        QMetaObject::invokeMethod(this, [this, updates]() {
            m_updates = updates;
            m_filteredUpdates = updates;
            
            m_checkButton->setEnabled(true);
            m_checkButton->setText("Check for Updates");
            
            if (updates.isEmpty()) {
                m_statusLabel->setText("Your system is up to date!");
                m_countLabel->clear();
                m_searchInput->setEnabled(false);
            } else {
                m_statusLabel->hide();
                m_countLabel->setText(QString("%1 updates available")
                                     .arg(updates.size()));
                m_updateAllButton->setEnabled(true);
                m_searchInput->setEnabled(true);
                
                // Apply any existing search filter
                QString searchText = m_searchInput->text();
                if (!searchText.isEmpty()) {
                    filterUpdates(searchText);
                } else {
                    displayUpdates(updates);
                }
            }
            
            Logger::info(QString("Found %1 updates").arg(updates.size()));
        }, Qt::QueuedConnection);
    });
}

void UpdatesWidget::displayUpdates(const QVector<UpdateInfo>& updates) {
    clearUpdates();
    
    for (const auto& update : updates) {
        auto* item = new UpdateItem(update, m_contentWidget);
        connect(item, &UpdateItem::updateRequested,
                this, &UpdatesWidget::onUpdateSingle);
        m_contentLayout->insertWidget(m_contentLayout->count() - 1, item);
    }
}

void UpdatesWidget::clearUpdates() {
    while (m_contentLayout->count() > 1) {
        auto* item = m_contentLayout->takeAt(0);
        if (auto* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void UpdatesWidget::filterUpdates(const QString& searchText) {
    if (searchText.isEmpty()) {
        m_filteredUpdates = m_updates;
        displayUpdates(m_filteredUpdates);
        m_countLabel->setText(QString("%1 updates available").arg(m_updates.size()));
        return;
    }
    
    QString lowerSearch = searchText.toLower();
    m_filteredUpdates.clear();
    
    for (const auto& update : m_updates) {
        if (update.name.toLower().contains(lowerSearch)) {
            m_filteredUpdates.append(update);
        }
    }
    
    displayUpdates(m_filteredUpdates);
    
    // Update count label to show filtered count
    if (m_filteredUpdates.size() == m_updates.size()) {
        m_countLabel->setText(QString("%1 updates available").arg(m_updates.size()));
    } else {
        m_countLabel->setText(QString("%1 of %2 updates").arg(m_filteredUpdates.size()).arg(m_updates.size()));
    }
}

void UpdatesWidget::onSearchTextChanged(const QString& text) {
    filterUpdates(text);
}

void UpdatesWidget::onUpdateAll() {
    auto reply = QMessageBox::question(this, "Update All",
        QString("Are you sure you want to update all %1 packages?")
        .arg(m_updates.size()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        PackageManager::instance().updateAllPackages();
    }
}

void UpdatesWidget::onUpdateSingle(const QString& packageName) {
    auto reply = QMessageBox::question(this, "Update Package",
        QString("Are you sure you want to update %1?").arg(packageName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        PackageManager::instance().updatePackage(packageName);
    }
}

QString UpdatesWidget::formatSize(qint64 bytes) {
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 1);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 0);
    }
    return QString("%1 B").arg(bytes);
}

void UpdatesWidget::showProgress(const QString& message) {
    m_progressLabel->setText(message);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressWidget->show();
    m_progressBar->show();
    m_progressLabel->show();
    m_logViewer->clear();
}

void UpdatesWidget::hideProgress() {
    // Hide the progress bar and label, but keep the widget and toggle button visible
    m_progressBar->hide();
    m_progressLabel->hide();
    // Don't hide m_progressWidget - keeps the toggle button visible
    // Don't hide the log widget or reset log visibility
    // This allows users to review logs after operation completes
}

void UpdatesWidget::toggleLogViewer() {
    m_logVisible = !m_logVisible;
    if (m_logVisible) {
        m_logWidget->show();
        m_toggleLogButton->setText("Hide Logs");
    } else {
        m_logWidget->hide();
        m_toggleLogButton->setText("Show Logs");
    }
}

void UpdatesWidget::onOperationStarted(const QString& message) {
    showProgress(message);
    m_updateAllButton->setEnabled(false);
    m_checkButton->setEnabled(false);
    
    // Disable all individual update buttons
    for (int i = 0; i < m_contentLayout->count() - 1; ++i) {
        if (auto* item = m_contentLayout->itemAt(i)) {
            if (auto* widget = item->widget()) {
                widget->setEnabled(false);
            }
        }
    }
}

void UpdatesWidget::onOperationOutput(const QString& output) {
    m_logViewer->append(output);
    
    // Auto-scroll to bottom
    auto cursor = m_logViewer->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logViewer->setTextCursor(cursor);
    
    // Try to parse progress information from output
    // This is a simple implementation - could be enhanced
    if (output.contains("downloading", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Downloading packages...");
        m_progressBar->setRange(0, 0); // Indeterminate
    } else if (output.contains("installing", Qt::CaseInsensitive)) {
        m_progressLabel->setText("Installing packages...");
        m_progressBar->setRange(0, 0); // Indeterminate
    }
}

void UpdatesWidget::onOperationCompleted(bool success, const QString& message) {
    // Refresh ALPM state so subsequent queries reflect the changes
    AlpmWrapper::instance().release();
    AlpmWrapper::instance().initialize();
    
    hideProgress();
    
    m_updateAllButton->setEnabled(!m_updates.isEmpty());
    m_checkButton->setEnabled(true);
    
    // Re-enable all individual update buttons
    for (int i = 0; i < m_contentLayout->count() - 1; ++i) {
        if (auto* item = m_contentLayout->itemAt(i)) {
            if (auto* widget = item->widget()) {
                widget->setEnabled(true);
            }
        }
    }
    
    if (success) {
        QMessageBox::information(this, "Success", message);
        checkForUpdates();
    } else {
        QMessageBox::warning(this, "Operation Failed", message);
    }
}

void UpdatesWidget::onOperationError(const QString& error) {
    // Refresh ALPM state (best-effort)
    AlpmWrapper::instance().release();
    AlpmWrapper::instance().initialize();
    
    hideProgress();
    
    m_updateAllButton->setEnabled(!m_updates.isEmpty());
    m_checkButton->setEnabled(true);
    
    // Re-enable all individual update buttons
    for (int i = 0; i < m_contentLayout->count() - 1; ++i) {
        if (auto* item = m_contentLayout->itemAt(i)) {
            if (auto* widget = item->widget()) {
                widget->setEnabled(true);
            }
        }
    }
    
    QMessageBox::critical(this, "Error", error);
}

#include "updates_widget.moc"
