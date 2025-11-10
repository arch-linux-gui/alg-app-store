#ifndef UPDATES_WIDGET_H
#define UPDATES_WIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QVector>
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
    QString formatSize(qint64 bytes);
    
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QLabel* m_statusLabel;
    QLabel* m_countLabel;
    QPushButton* m_updateAllButton;
    QPushButton* m_checkButton;
    
    QVector<UpdateInfo> m_updates;
    
private slots:
    void onUpdateAll();
    void onUpdateSingle(const QString& packageName);
};

#endif // UPDATES_WIDGET_H
