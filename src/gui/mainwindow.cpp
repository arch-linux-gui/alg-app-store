#include "mainwindow.h"
#include "home_widget.h"
#include "search_widget.h"
#include "installed_widget.h"
#include "updates_widget.h"
#include "settings_widget.h"
#include "../utils/logging.h"
#include "utils/version.h"
#include "../core/alpm_wrapper.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QApplication>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tabWidget(std::make_unique<QTabWidget>(this)) {
    
    // Initialize ALPM before creating widgets that might need it
    if (!AlpmWrapper::instance().initialize()) {
        QMessageBox::critical(this, "Error", 
            "Failed to initialize package manager. Please check your system configuration.");
        spdlog::error("Failed to initialize ALPM in MainWindow");
    }
    
    setupUi();
    loadStyleSheet();
    
    spdlog::info("MainWindow created successfully");
}

MainWindow::~MainWindow() {
    AlpmWrapper::instance().release();
    spdlog::info("MainWindow destroyed");
}

void MainWindow::setupUi() {
    setWindowTitle("Explorer (Beta)");
    setMinimumSize(1024, 768);
    resize(1124, 868);
    
    // Create widgets
    m_homeWidget = new HomeWidget(this);
    m_searchWidget = new SearchWidget(this);
    m_installedWidget = new InstalledWidget(this);
    m_updatesWidget = new UpdatesWidget(this);
    m_settingsWidget = new SettingsWidget(this);
    
    // Add tabs
    m_tabWidget->addTab(m_homeWidget, "Home");
    m_tabWidget->addTab(m_searchWidget, "Search");
    m_tabWidget->addTab(m_installedWidget, "Installed");
    m_tabWidget->addTab(m_updatesWidget, "Updates");
    m_tabWidget->addTab(m_settingsWidget, "Settings");
    
    // Connect settings signals
    connect(m_settingsWidget, &SettingsWidget::multilibStatusChanged,
            this, [this]() {
        m_searchWidget->updateRepositoryList(
            m_settingsWidget->isMultilibEnabled(),
            m_settingsWidget->isChaoticAurEnabled()
        );
    });
    
    connect(m_settingsWidget, &SettingsWidget::chaoticAurStatusChanged,
            this, [this]() {
        m_searchWidget->updateRepositoryList(
            m_settingsWidget->isMultilibEnabled(),
            m_settingsWidget->isChaoticAurEnabled()
        );
    });
    
    // Initialize search widget with current repository states
    m_searchWidget->updateRepositoryList(
        m_settingsWidget->isMultilibEnabled(),
        m_settingsWidget->isChaoticAurEnabled()
    );
    
	  m_tabWidget->setDocumentMode(true);
	  m_tabWidget->tabBar()->setExpanding(true);

    m_tabWidget->setTabPosition(QTabWidget::North);
    m_tabWidget->setMovable(false);
    
    setCentralWidget(m_tabWidget.get());
    createMenuBar();
}

void MainWindow::createMenuBar() {
    auto* fileMenu = menuBar()->addMenu("&File");
    
    auto* refreshAction = new QAction("&Refresh", this);
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, [this]() {
        int currentIndex = m_tabWidget->currentIndex();
        if (currentIndex == 0) {
            // Home widget refresh
        } else if (currentIndex == 1) {
            // Search widget refresh
        } else if (currentIndex == 2) {
            m_installedWidget->refreshPackages();
        } else if (currentIndex == 3) {
            m_updatesWidget->checkForUpdates();
        }
    });
    fileMenu->addAction(refreshAction);
    
    fileMenu->addSeparator();
    
    auto* quitAction = new QAction("&Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(quitAction);
    
    auto* helpMenu = menuBar()->addMenu("&Help");
    
    auto* aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About Explorer",
            "Explorer (Beta)\n\n"
            "A modern package manager for Arch Linux\n"
            "Version: " APP_VERSION "\n"
            "Built with Qt6 and C++20\n\n"
            "© 2025 Arka Linux GUI");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::loadStyleSheet() {
    QStringList styleFiles = {
        ":/resource/styles/base.qss",
        ":/resource/styles/navigation.qss",
        ":/resource/styles/components.qss",
        ":/resource/styles/containers.qss"
    };

    QString combinedStyleSheet;
    bool anyLoaded = false; 
    
    for (const QString &path : styleFiles) {
        QFile file(path);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            combinedStyleSheet += QLatin1String(file.readAll());
            file.close();
            anyLoaded = true;
        } else {
            spdlog::warn("{}", (QString("Could not load style module: %1").arg(path)).toStdString());
        }
    }

    if (anyLoaded) {
        qApp->setStyleSheet(combinedStyleSheet);
        spdlog::info("Modular stylesheets loaded and combined successfully from resources.");
    } else {
        spdlog::error("Failed to load any stylesheet modules from resources!");
    }
}
