#ifndef INSTALLED_WIDGET_H
#define INSTALLED_WIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QVector>
#include "../utils/types.h"

class InstalledWidget : public QWidget {
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
    
    QLineEdit* m_filterInput;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QGridLayout* m_gridLayout;
    QLabel* m_statusLabel;
    QLabel* m_countLabel;
    
    QVector<PackageInfo> m_allPackages;
    QVector<PackageInfo> m_filteredPackages;
    
private slots:
    void onPackageClicked(const PackageInfo& info);
    void onFilterTextChanged(const QString& text);
};

#endif // INSTALLED_WIDGET_H
