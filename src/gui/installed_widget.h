#ifndef INSTALLED_WIDGET_H
#define INSTALLED_WIDGET_H

#include "../utils/types.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QTimer>
#include <QVector>
#include <QWidget>

/**
 * @brief Widget displaying installed packages.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 * - Package cards are dynamically created/destroyed in displayPackages/clearResults
 */
class InstalledWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InstalledWidget(QWidget* parent = nullptr);
    ~InstalledWidget() override = default;

    void refreshPackages();

private:
    void setupUi();
    void loadInstalledPackages();
    void displayPackages(const QVector<PackageInfo>& packages);
    void filterPackages(const QString& query);
    void clearResults();

    // Qt parent-child managed widgets (non-owning pointers)
    QLineEdit* m_filterInput = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QGridLayout* m_gridLayout = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_countLabel = nullptr;

    QVector<PackageInfo> m_allPackages;
    QVector<PackageInfo> m_filteredPackages;

    QTimer* m_filterTimer;

private slots:
    void onPackageClicked(const PackageInfo& info);
    void onFilterTextChanged(const QString& text);
};

#endif  // INSTALLED_WIDGET_H
