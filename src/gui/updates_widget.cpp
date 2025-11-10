#include "updates_widget.h"
#include "../core/alpm_wrapper.h"
#include "../core/aur_helper.h"
#include "../core/package_manager.h"
#include "../utils/logger.h"
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QtConcurrent>

class UpdateItem : public QWidget {
    Q_OBJECT
    
public:
    UpdateItem(const UpdateInfo& info, QWidget* parent = nullptr)
        : QWidget(parent), m_info(info) {
        
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
        versionLabel->setStyleSheet("color: #888;");
        infoLayout->addWidget(versionLabel);
        
        auto* repoLabel = new QLabel(m_info.repository, this);
        repoLabel->setStyleSheet("color: #666; font-size: 10px;");
        infoLayout->addWidget(repoLabel);
        
        layout->addLayout(infoLayout);
        layout->addStretch();
        
        if (m_info.downloadSize > 0) {
            auto* sizeLabel = new QLabel(formatSize(m_info.downloadSize), this);
            sizeLabel->setStyleSheet("color: #888;");
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
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_contentLayout(new QVBoxLayout(m_contentWidget))
    , m_statusLabel(new QLabel(this))
    , m_countLabel(new QLabel(this))
    , m_updateAllButton(new QPushButton("Update All", this))
    , m_checkButton(new QPushButton("Check for Updates", this)) {
    
    setupUi();
    
    connect(&PackageManager::instance(), &PackageManager::operationCompleted,
            this, [this](bool success, const QString& message) {
        if (success) {
            QMessageBox::information(this, "Success", message);
            checkForUpdates();
        } else {
            QMessageBox::warning(this, "Error", message);
        }
    });
}

void UpdatesWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Header
    auto* headerLayout = new QHBoxLayout();
    
    auto* titleLabel = new QLabel("Available Updates", this);
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    m_countLabel->setStyleSheet("font-size: 14px; color: #888;");
    headerLayout->addWidget(m_countLabel);
    
    m_checkButton->setMinimumHeight(35);
    connect(m_checkButton, &QPushButton::clicked, this, &UpdatesWidget::checkForUpdates);
    headerLayout->addWidget(m_checkButton);
    
    m_updateAllButton->setMinimumHeight(35);
    m_updateAllButton->setMinimumWidth(120);
    m_updateAllButton->setEnabled(false);
    connect(m_updateAllButton, &QPushButton::clicked, this, &UpdatesWidget::onUpdateAll);
    headerLayout->addWidget(m_updateAllButton);
    
    mainLayout->addLayout(headerLayout);
    
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
            
            m_checkButton->setEnabled(true);
            m_checkButton->setText("Check for Updates");
            
            if (updates.isEmpty()) {
                m_statusLabel->setText("Your system is up to date!");
                m_countLabel->clear();
            } else {
                m_statusLabel->hide();
                m_countLabel->setText(QString("%1 updates available")
                                     .arg(updates.size()));
                m_updateAllButton->setEnabled(true);
                displayUpdates(updates);
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

#include "updates_widget.moc"
