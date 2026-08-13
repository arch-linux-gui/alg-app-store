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
    connect(m_aurHelper.get(), &AurHelper::error,
            this, &SearchWidget::onAurSearchError);
}

void SearchWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Title
    auto* titleLabel = new QLabel("Search Packages", this);
    titleLabel->setObjectName("view-title");
    mainLayout->addWidget(titleLabel);
    
    // Search bar
    auto* searchLayout = new QHBoxLayout();
    
    m_searchInput->setPlaceholderText("Search for packages...");
    m_searchInput->setMinimumHeight(35);
    m_searchInput->setClearButtonEnabled(true);
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
    m_searchButton->setProperty("class", "primary-btn");
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
    
    if (m_searchInProgress) {
        return;
    }

    m_searchInProgress = true;
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
    m_searchInProgress = false;
    
    if (m_allResults.isEmpty()) {
        m_statusLabel->setText("No results found");
        return;
    }
    
    m_statusLabel->hide();
    
    // Apply filter
    onFilterChanged(m_filterCombo->currentIndex());
    
    Logger::info(QString("Search completed: %1 results").arg(m_allResults.size()));
}

void SearchWidget::onAurSearchError(const QString& errorMsg) {
    Logger::warning(QString("AUR search failed: %1").arg(errorMsg));

    m_searchButton->setEnabled(true);
    m_searchButton->setText("Search");
    m_searchInProgress = false;

    if (m_allResults.isEmpty()) {
        m_statusLabel->setText("No results found");
        m_statusLabel->show();
    }
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

void SearchWidget::updateRepositoryList(bool multilibEnabled, bool chaoticAurEnabled) {
    // Save the current selection
    int currentIndex = m_filterCombo->currentIndex();
    QString currentFilter = m_filterCombo->itemData(currentIndex).toString();
    
    // Check if multilib already exists in the list
    bool multilibExists = false;
    for (int i = 0; i < m_filterCombo->count(); ++i) {
        if (m_filterCombo->itemData(i).toString() == "multilib") {
            multilibExists = true;
            break;
        }
    }
    
    // Check if chaotic-aur already exists in the list
    bool chaoticAurExists = false;
    for (int i = 0; i < m_filterCombo->count(); ++i) {
        if (m_filterCombo->itemData(i).toString() == "chaotic-aur") {
            chaoticAurExists = true;
            break;
        }
    }
    
    // Handle multilib
    if (multilibEnabled && !multilibExists) {
        // Add multilib to the dropdown (insert before AUR)
        int aurIndex = m_filterCombo->findData("AUR");
        if (aurIndex != -1) {
            m_filterCombo->insertItem(aurIndex, "Multilib", "multilib");
        } else {
            m_filterCombo->addItem("Multilib", "multilib");
        }
        Logger::info("Added multilib repository to search filter");
    } else if (!multilibEnabled && multilibExists) {
        // Remove multilib from the dropdown
        int multilibIndex = m_filterCombo->findData("multilib");
        if (multilibIndex != -1) {
            m_filterCombo->removeItem(multilibIndex);
            Logger::info("Removed multilib repository from search filter");
        }
    }
    
    // Handle chaotic-aur
    if (chaoticAurEnabled && !chaoticAurExists) {
        // Add chaotic-aur to the dropdown (insert before AUR)
        int aurIndex = m_filterCombo->findData("AUR");
        if (aurIndex != -1) {
            m_filterCombo->insertItem(aurIndex, "Chaotic-AUR", "chaotic-aur");
        } else {
            m_filterCombo->addItem("Chaotic-AUR", "chaotic-aur");
        }
        Logger::info("Added chaotic-aur repository to search filter");
    } else if (!chaoticAurEnabled && chaoticAurExists) {
        // Remove chaotic-aur from the dropdown
        int chaoticAurIndex = m_filterCombo->findData("chaotic-aur");
        if (chaoticAurIndex != -1) {
            m_filterCombo->removeItem(chaoticAurIndex);
            Logger::info("Removed chaotic-aur repository from search filter");
        }
    }
    
    // Restore previous selection if it still exists
    int newIndex = m_filterCombo->findData(currentFilter);
    if (newIndex != -1) {
        m_filterCombo->setCurrentIndex(newIndex);
    } else {
        // If previous selection was removed, select "All"
        m_filterCombo->setCurrentIndex(0);
    }
}
