#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H

#include "../utils/types.h"
#include <QGridLayout>
#include <QScrollArea>
#include <QTimer>
#include <QVector>
#include <QWidget>

class PackageCard;

/**
 * @brief Widget displaying featured packages on the home screen.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 * - m_packageCards contains non-owning pointers to cards owned by m_contentWidget
 */
class HomeWidget : public QWidget
{
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
    QVector<PackageCard*> m_packageCards;  // Non-owning pointers, owned by m_contentWidget

    // Qt parent-child managed widgets (non-owning pointers)
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QGridLayout* m_gridLayout = nullptr;
    QTimer* m_updateTimer = nullptr;

private slots:
    void onPackageClicked(const PackageInfo& info);
    void onUpdateTimer();
};

#endif  // HOME_WIDGET_H
