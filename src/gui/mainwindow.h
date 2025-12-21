#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTabWidget>
#include <memory>

class HomeWidget;
class SearchWidget;
class InstalledWidget;
class UpdatesWidget;
class SettingsWidget;

/**
 * @brief Main application window for ALG App Store.
 * 
 * Memory Management:
 * - m_tabWidget: Owned by std::unique_ptr (central widget)
 * - Child widgets (m_homeWidget, etc.): Owned by Qt parent-child hierarchy
 *   through m_tabWidget. Raw pointers are used as non-owning references.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    
private:
    void setupUi();
    void createMenuBar();
    void loadStyleSheet();
    
    std::unique_ptr<QTabWidget> m_tabWidget;
    
    // Non-owning pointers - owned by m_tabWidget via Qt parent-child hierarchy
    HomeWidget* m_homeWidget = nullptr;
    SearchWidget* m_searchWidget = nullptr;
    InstalledWidget* m_installedWidget = nullptr;
    UpdatesWidget* m_updatesWidget = nullptr;
    SettingsWidget* m_settingsWidget = nullptr;
};

#endif // MAINWINDOW_H
