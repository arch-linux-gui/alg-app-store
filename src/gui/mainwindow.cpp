#include "mainwindow.h"
#include "home_widget.h"
#include "search_widget.h"
#include "installed_widget.h"
#include "updates_widget.h"
#include "settings_widget.h"
#include "../utils/logger.h"
#include "../core/alpm_wrapper.h"
#include "../core/auth_manager.h"
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
        Logger::error("Failed to initialize ALPM in MainWindow");
    }

    // Authenticate once at startup for all privileged operations
    if (!AuthManager::instance().authenticate(this)) {
        Logger::error("Authentication failed or cancelled");
        QMessageBox::critical(this, "Authentication Required",
            "This application requires administrator privileges to manage packages.\n"
            "The application will now exit.");
        // Schedule exit after event loop starts
        QMetaObject::invokeMethod(qApp, &QApplication::quit, Qt::QueuedConnection);
        return;
    }
    
    setupUi();
    loadStyleSheet();
    
    Logger::info("MainWindow created successfully");
}

MainWindow::~MainWindow() {
    AlpmWrapper::instance().release();
    Logger::info("MainWindow destroyed");
}

void MainWindow::setupUi() {
    setWindowTitle("ALG App Store (Beta)");
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
        QMessageBox::about(this, "About ALG App Store",
            "ALG App Store (Beta)\n\n"
            "A modern package manager for Arch Linux\n"
            "Version: 0.2.28\n"
            "Built with Qt6 and C++17\n\n"
            "© 2025 Arka Linux GUI");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::loadStyleSheet() {
    QFile styleFile(":/stylesheet.qss");
    
    if (!styleFile.exists()) {
        // Try loading from current directory (for development)
        styleFile.setFileName("stylesheet.qss");
    }
    
    if (!styleFile.exists()) {
        // Try loading from system installation path
        styleFile.setFileName("/usr/share/alg-app-store/stylesheet.qss");
    }
    
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(styleSheet);
        styleFile.close();
        Logger::info(QString("Stylesheet loaded successfully from: %1").arg(styleFile.fileName()));
    } else {
        Logger::warning("Could not load stylesheet");
    }
}
