#include "search_widget.h"
#include "package_card.h"
#include "package_details_dialog.h"
#include "../core/alpm_wrapper.h"
#include "../core/aur_helper.h"
#include "../utils/logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QtConcurrent>
#include <QEventLoop>

SearchWidget::SearchWidget(QWidget* parent)
    : QWidget(parent)
    , m_searchInput(new QLineEdit(this))
    , m_searchButton(new QPushButton("Search", this))
    , m_filterCombo(new QComboBox(this))
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_gridLayout(new QGridLayout(m_contentWidget))
    , m_statusLabel(new QLabel(this))
    , m_aurHelper(std::make_unique<AurHelper>(this)) {
    
    setupUi();
    
    connect(m_aurHelper.get(), &AurHelper::searchCompleted,
            this, &SearchWidget::onAurSearchCompleted);
}

void SearchWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Title
    auto* titleLabel = new QLabel("Search Packages", this);
    auto titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Search bar
    auto* searchLayout = new QHBoxLayout();
    
    m_searchInput->setPlaceholderText("Search for packages...");
    m_searchInput->setMinimumHeight(35);
    connect(m_searchInput, &QLineEdit::returnPressed, this, &SearchWidget::onSearchClicked);
    searchLayout->addWidget(m_searchInput, 1);
    
    // Filter combo
    m_filterCombo->addItem("All", "all");
    m_filterCombo->addItem("Core", "core");
    m_filterCombo->addItem("Extra", "extra");
    m_filterCombo->addItem("AUR", "AUR");
    m_filterCombo->setMinimumHeight(35);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SearchWidget::onFilterChanged);
    searchLayout->addWidget(m_filterCombo);
    
    m_searchButton->setMinimumHeight(35);
    m_searchButton->setMinimumWidth(100);
    connect(m_searchButton, &QPushButton::clicked, this, &SearchWidget::onSearchClicked);
    searchLayout->addWidget(m_searchButton);
    
    mainLayout->addLayout(searchLayout);
    
    // Status label
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();
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

void SearchWidget::onSearchClicked() {
    QString query = m_searchInput->text().trimmed();
    
    if (query.isEmpty()) {
        m_statusLabel->setText("Please enter a search term");
        m_statusLabel->show();
        return;
    }
    
    m_searchButton->setEnabled(false);
    m_searchButton->setText("Searching...");
    m_statusLabel->setText("Searching...");
    m_statusLabel->show();
    
    clearResults();
    performSearch();
}

void SearchWidget::performSearch() {
    QString query = m_searchInput->text().trimmed().toLower().replace(' ', '-');
    
    // Search in official repos using ALPM
    (void)QtConcurrent::run([this, query]() {
        auto results = AlpmWrapper::instance().searchPackages(query);
        
        QMetaObject::invokeMethod(this, [this, results]() {
            m_allResults = results;
            
            // Also search AUR
            m_aurHelper->searchPackages(m_searchInput->text().trimmed());
        }, Qt::QueuedConnection);
    });
}

void SearchWidget::onAurSearchCompleted(const QVector<PackageInfo>& results) {
    // Combine with official repo results
    m_allResults.append(results);
    
    m_searchButton->setEnabled(true);
    m_searchButton->setText("Search");
    
    if (m_allResults.isEmpty()) {
        m_statusLabel->setText("No results found");
        return;
    }
    
    m_statusLabel->hide();
    
    // Apply filter
    onFilterChanged(m_filterCombo->currentIndex());
    
    Logger::info(QString("Search completed: %1 results").arg(m_allResults.size()));
}

void SearchWidget::onFilterChanged(int index) {
    QString filter = m_filterCombo->itemData(index).toString();
    
    if (filter == "all") {
        displayResults(m_allResults);
    } else {
        QVector<PackageInfo> filtered;
        for (const auto& pkg : m_allResults) {
            if (pkg.repository == filter) {
                filtered.append(pkg);
            }
        }
        displayResults(filtered);
    }
}

void SearchWidget::displayResults(const QVector<PackageInfo>& results) {
    clearResults();
    m_currentResults = results;
    
    if (results.isEmpty()) {
        m_statusLabel->setText("No results found for selected filter");
        m_statusLabel->show();
        return;
    }
    
    int row = 0;
    int col = 0;
    const int columns = 3;
    
    for (const auto& pkg : results) {
        auto* card = new PackageCard(pkg, m_contentWidget);
        connect(card, &PackageCard::clicked, this, &SearchWidget::onPackageClicked);
        
        m_gridLayout->addWidget(card, row, col);
        
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
    
    m_gridLayout->setRowStretch(row + 1, 1);
}

void SearchWidget::clearResults() {
    while (auto* item = m_gridLayout->takeAt(0)) {
        if (auto* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void SearchWidget::onPackageClicked(const PackageInfo& info) {
    Logger::info(QString("Package clicked: %1").arg(info.name));
    
    PackageInfo fullInfo = info;
    
    // For AUR packages, fetch complete details including maintainer, URL, dependencies
    if (info.repository.toLower() == "aur") {
        auto* aurHelper = new AurHelper(this);
        QEventLoop loop;
        
        connect(aurHelper, &AurHelper::packageInfoReceived, [&](const PackageInfo& detailedInfo) {
            fullInfo = detailedInfo;
            loop.quit();
        });
        
        connect(aurHelper, &AurHelper::error, [&](const QString& errorMsg) {
            Logger::warning(QString("Failed to fetch AUR details for %1: %2").arg(info.name, errorMsg));
            loop.quit();
        });
        
        aurHelper->getPackageInfo(info.name);
        loop.exec();
        
        aurHelper->deleteLater();
    }
    
    auto* dialog = new PackageDetailsDialog(fullInfo, this);
    dialog->exec();
    dialog->deleteLater();
}
