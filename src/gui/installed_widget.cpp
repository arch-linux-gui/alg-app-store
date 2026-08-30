#include "installed_widget.h"
#include "package_card.h"
#include "package_details_dialog.h"
#include "../core/alpm_wrapper.h"
#include "../utils/logging.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QtConcurrent>
#include <QStyle>

InstalledWidget::InstalledWidget(QWidget* parent)
    : QWidget(parent)
    , m_filterInput(new QLineEdit(this))
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_gridLayout(new QGridLayout(m_contentWidget))
    , m_statusLabel(new QLabel(this))
    , m_countLabel(new QLabel(this)) 
    , m_filterTimer(new QTimer(this)) {

    // debounce timer (waits 300ms after last keystroke)
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(300);
    connect(m_filterTimer, &QTimer::timeout, this, [this]() {
        filterPackages(m_filterInput->text());
    });

    setupUi();
    loadInstalledPackages();
}

void InstalledWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Header
    auto* headerLayout = new QHBoxLayout();
    
    auto* titleLabel = new QLabel("Installed Packages", this);
    titleLabel->setObjectName("view-title"); 
    headerLayout->addWidget(titleLabel); 
    headerLayout->addStretch();
    
    // Counter Label
    m_countLabel->setObjectName("package-count-label"); 
    headerLayout->addWidget(m_countLabel);    

    auto* refreshButton = new QPushButton("Refresh", this);
    connect(refreshButton, &QPushButton::clicked, this, &InstalledWidget::refreshPackages);
    headerLayout->addWidget(refreshButton);
    
    mainLayout->addLayout(headerLayout);
    
    // Filter
    m_filterInput->setPlaceholderText("Filter installed packages...");
    m_filterInput->setMinimumHeight(35);
    m_filterInput->setClearButtonEnabled(true);
    connect(m_filterInput, &QLineEdit::textChanged, 
            this, &InstalledWidget::onFilterTextChanged);
    mainLayout->addWidget(m_filterInput);
    
    // Status label
    m_statusLabel->setObjectName("status-message");
	  m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setText("Loading installed packages...");
    mainLayout->addWidget(m_statusLabel);
    
    // Results area
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_gridLayout->setSpacing(15);
    m_gridLayout->setContentsMargins(10, 10, 10, 10);
    
    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);
}

void InstalledWidget::loadInstalledPackages() {
    m_statusLabel->setText("Loading installed packages...");
    m_statusLabel->show();
    
    (void)QtConcurrent::run([this]() {
        auto packages = AlpmWrapper::instance().getInstalledPackages();
        
        QMetaObject::invokeMethod(this, [this, packages]() {
            m_allPackages = packages;
            m_filteredPackages = packages;
            
            m_statusLabel->hide();
            m_countLabel->setText(QString("%1 packages installed")
                                 .arg(packages.size()));
            
            filterPackages(m_filterInput->text());
            
            spdlog::info("{}", (QString("Loaded %1 installed packages").arg(packages.size())).toStdString());
        }, Qt::QueuedConnection);
    });
}

void InstalledWidget::refreshPackages() {
    clearResults();
    loadInstalledPackages();
}

void InstalledWidget::displayPackages(const QVector<PackageInfo>& packages) {
    clearResults();
    
    if (packages.isEmpty()) {
        m_statusLabel->setText("No packages found");
        m_statusLabel->show();
        return;
    }
    
    int row = 0;
    int col = 0;
    const int columns = 3;
    
    for (const auto& pkg : packages) {
        auto* card = new PackageCard(pkg, m_contentWidget);
        card->updateInstallStatus(true);
        connect(card, &PackageCard::clicked, this, &InstalledWidget::onPackageClicked);
        
        m_gridLayout->addWidget(card, row, col);
        
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
    
    m_gridLayout->setRowStretch(row + 1, 1);
}

void InstalledWidget::clearResults() {
    while (auto* item = m_gridLayout->takeAt(0)) {
        if (auto* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void InstalledWidget::filterPackages(const QString& query) {
    if (query.isEmpty()) {
        m_filteredPackages = m_allPackages;
    } else {
        m_filteredPackages.clear();
        QString lowerQuery = query.toLower();
        
        for (const auto& pkg : m_allPackages) {
            if (pkg.name.toLower().contains(lowerQuery) ||
                pkg.description.toLower().contains(lowerQuery)) {
                m_filteredPackages.append(pkg);
            }
        }
    }
    
    m_countLabel->setText(QString("%1 of %2 packages")
                         .arg(m_filteredPackages.size())
                         .arg(m_allPackages.size()));
    
    displayPackages(m_filteredPackages);
}

void InstalledWidget::onFilterTextChanged(const QString& text) {
    Q_UNUSED(text);
    m_filterTimer->start();
}

void InstalledWidget::onPackageClicked(const PackageInfo& info) {
    spdlog::info("{}", (QString("Package clicked: %1").arg(info.name)).toStdString());
    
    auto* dialog = new PackageDetailsDialog(info, this);
    if(dialog->exec() == QDialog::Accepted) {
      // Refresh after dialog closes in case package was uninstalled
      refreshPackages();
    }

    dialog->deleteLater();
}
