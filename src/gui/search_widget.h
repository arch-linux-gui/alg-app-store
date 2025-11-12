#ifndef SEARCH_WIDGET_H
#define SEARCH_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QVector>
#include <memory>
#include "../utils/types.h"
#include "../core/aur_helper.h"

class SearchWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit SearchWidget(QWidget* parent = nullptr);
    ~SearchWidget() override = default;
    
public slots:
    void updateRepositoryList(bool multilibEnabled, bool chaoticAurEnabled = false);
    
private:
    void setupUi();
    void performSearch();
    void displayResults(const QVector<PackageInfo>& results);
    void clearResults();
    
    QLineEdit* m_searchInput;
    QPushButton* m_searchButton;
    QComboBox* m_filterCombo;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QGridLayout* m_gridLayout;
    QLabel* m_statusLabel;
    
    std::unique_ptr<AurHelper> m_aurHelper;
    QVector<PackageInfo> m_currentResults;
    QVector<PackageInfo> m_allResults;
    
private slots:
    void onSearchClicked();
    void onAurSearchCompleted(const QVector<PackageInfo>& results);
    void onFilterChanged(int index);
    void onPackageClicked(const PackageInfo& info);
};

#endif // SEARCH_WIDGET_H
