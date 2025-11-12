#ifndef UPDATES_WIDGET_H
#define UPDATES_WIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QProgressBar>
#include <QTextEdit>
#include <QLineEdit>
#include "../utils/types.h"

class UpdateItem;

class UpdatesWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit UpdatesWidget(QWidget* parent = nullptr);
    ~UpdatesWidget() override = default;
    
    void checkForUpdates();
    
private:
    void setupUi();
    void displayUpdates(const QVector<UpdateInfo>& updates);
    void clearUpdates();
    void filterUpdates(const QString& searchText);
    QString formatSize(qint64 bytes);
    void showProgress(const QString& message);
    void hideProgress();
    void toggleLogViewer();
    
    QLineEdit* m_searchInput;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QLabel* m_statusLabel;
    QLabel* m_countLabel;
    QPushButton* m_updateAllButton;
    QPushButton* m_checkButton;
    
    // Progress bar and log viewer
    QWidget* m_progressWidget;
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    QPushButton* m_toggleLogButton;
    QWidget* m_logWidget;
    QTextEdit* m_logViewer;
    bool m_logVisible;
    
    QVector<UpdateInfo> m_updates;
    QVector<UpdateInfo> m_filteredUpdates;
    
private slots:
    void onUpdateAll();
    void onUpdateSingle(const QString& packageName);
    void onSearchTextChanged(const QString& text);
    void onOperationStarted(const QString& message);
    void onOperationOutput(const QString& output);
    void onOperationCompleted(bool success, const QString& message);
    void onOperationError(const QString& error);
};

#endif // UPDATES_WIDGET_H
