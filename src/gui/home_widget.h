#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H

#include <QWidget>
#include <QVector>
#include <QScrollArea>
#include <QGridLayout>
#include <QTimer>
#include "../utils/types.h"

class PackageCard;

class HomeWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit HomeWidget(QWidget* parent = nullptr);
    ~HomeWidget() override = default;
    
private:
    void setupUi();
    void loadFeaturedPackages();
    void createPackageCards();
    void checkInstalledPackages();
    
    QVector<PackageInfo> m_featuredPackages;
    QVector<PackageCard*> m_packageCards;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QGridLayout* m_gridLayout;
    QTimer* m_updateTimer;
    
private slots:
    void onPackageClicked(const PackageInfo& info);
    void onUpdateTimer();
};

#endif // HOME_WIDGET_H
