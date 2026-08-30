#include "home_widget.h"
#include "package_card.h"
#include "package_details_dialog.h"
#include "../core/alpm_wrapper.h"
#include "../core/aur_helper.h"
#include "../utils/logging.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QEventLoop>

HomeWidget::HomeWidget(QWidget* parent)
    : QWidget(parent)
    , m_scrollArea(new QScrollArea(this))
    , m_contentWidget(new QWidget())
    , m_gridLayout(new QGridLayout(m_contentWidget))
    , m_updateTimer(new QTimer(this)) {
    
    setupUi();
    loadFeaturedPackages();
    createPackageCards();
    
    // Setup timer to periodically check installation status
    connect(m_updateTimer, &QTimer::timeout, this, &HomeWidget::onUpdateTimer);
    m_updateTimer->start(3000); // Check every 3 seconds
    
    // Initial check
    checkInstalledPackages();
}

void HomeWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    auto* titleLabel = new QLabel("Featured Packages", this);
    titleLabel->setObjectName("view-title");
    mainLayout->addWidget(titleLabel);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_gridLayout->setSpacing(15);
    m_gridLayout->setContentsMargins(10, 10, 10, 10);
    
    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);
}

void HomeWidget::loadFeaturedPackages() {
    // Featured packages list with initial repositories
    m_featuredPackages = {
        {"firefox", "Latest", "Fast, Private & Safe Web Browser", "extra"},
        {"gimp", "Latest", "GNU Image Manipulation Program", "extra"},
        {"vlc", "Latest", "Multi-platform MPEG, VCD/DVD, and DivX player", "extra"},
        {"telegram-desktop", "Latest", "Official Telegram Desktop client", "extra"},
        {"obs-studio", "Latest", "Free, open source software for live streaming and recording", "extra"},
        {"blender", "Latest", "A fully integrated 3D graphics creation suite", "extra"},
        {"spotify", "Latest", "A proprietary music streaming service", "AUR"},
        {"discord", "Latest", "All-in-one voice and text chat for gamers", "extra"},
        {"google-chrome", "Latest", "The popular web browser by Google", "AUR"},
        {"visual-studio-code-bin", "Latest", "Visual Studio Code (official binary version)", "AUR"},
        {"libreoffice-still", "Latest", "Free and Open Source Office Suite", "extra"},
        {"zoom", "Latest", "Video Conferencing and Web Conferencing Service", "AUR"}
    };
    
    // Fetch actual package information from repositories
    for (auto& pkg : m_featuredPackages) {
        PackageInfo repoInfo = AlpmWrapper::instance().getPackageInfo(pkg.name);
        
        if (!repoInfo.name.isEmpty() && !repoInfo.repository.isEmpty()) {
            // Package found in official repos (including chaotic-aur), update with actual information
            pkg.repository = repoInfo.repository;
            pkg.version = repoInfo.version;
            pkg.description = repoInfo.description;
            
            if (pkg.repository.toLower() != "aur") {
                spdlog::debug("{}", (QString("Package %1 found in %2 repository with version %3")
                            .arg(pkg.name, pkg.repository, pkg.version)).toStdString());
            }
        } else if (pkg.repository.toLower() == "aur") {
            // Package not found in official repos (including chaotic-aur)
            // Default to AUR helper (yay/paru) since chaotic-aur is not enabled or doesn't have this package
            pkg.repository = "aur";
            spdlog::debug("{}", (QString("Package %1 not found in enabled repositories, defaulting to AUR helper").arg(pkg.name)).toStdString());
        }
    }
    
    spdlog::info("{}", (QString("Loaded %1 featured packages").arg(m_featuredPackages.size())).toStdString());
}

void HomeWidget::createPackageCards() {
    int row = 0;
    int col = 0;
    const int columns = 3;
    
    for (const auto& pkg : m_featuredPackages) {
        auto* card = new PackageCard(pkg, m_contentWidget);
        connect(card, &PackageCard::clicked, this, &HomeWidget::onPackageClicked);
        
        m_gridLayout->addWidget(card, row, col);
        m_packageCards.append(card);
        
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
    
    // Add stretch to push cards to the top
    m_gridLayout->setRowStretch(row + 1, 1);
}

void HomeWidget::checkInstalledPackages() {
    for (auto* card : m_packageCards) {
        card->checkInstallStatus();
    }
}

void HomeWidget::onUpdateTimer() {
    checkInstalledPackages();
}

void HomeWidget::onPackageClicked(const PackageInfo& info) {
    spdlog::info("{}", (QString("Package clicked: %1").arg(info.name)).toStdString());
    
    // Fetch full package details including dependencies
    PackageInfo fullInfo;
    
    if (info.repository.toLower() == "aur") {
        // For AUR packages, query AUR API for full details
        AurHelper aurHelper;
        QEventLoop loop;
        
        connect(&aurHelper, &AurHelper::packageInfoReceived, [&fullInfo, &loop](const PackageInfo& aurInfo) {
            fullInfo = aurInfo;
            loop.quit();
        });
        
        connect(&aurHelper, &AurHelper::error, [&fullInfo, &info, &loop](const QString& error) {
            spdlog::warn("{}", (QString("Failed to fetch AUR package info: %1").arg(error)).toStdString());
            fullInfo = info; // Fallback to basic info
            loop.quit();
        });
        
        aurHelper.getPackageInfo(info.name);
        loop.exec(); // Wait for response
        
        // If we didn't get full info, use the basic info
        if (fullInfo.name.isEmpty()) {
            fullInfo = info;
        }
    } else {
        // For official repos, fetch full details from ALPM
        fullInfo = AlpmWrapper::instance().getPackageInfo(info.name);
        // If not found, use the basic info
        if (fullInfo.name.isEmpty()) {
            fullInfo = info;
        }
    }
    
    auto* dialog = new PackageDetailsDialog(fullInfo, this);
    dialog->exec();
    dialog->deleteLater();
    
    // Update installation status after dialog closes
    checkInstalledPackages();
}
