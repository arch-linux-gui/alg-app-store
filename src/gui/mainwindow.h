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
    HomeWidget* m_homeWidget;
    SearchWidget* m_searchWidget;
    InstalledWidget* m_installedWidget;
    UpdatesWidget* m_updatesWidget;
};

#endif // MAINWINDOW_H
