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

/**
 * @brief Widget for searching packages across repositories.
 * 
 * Memory Management:
 * - m_aurHelper: Owned by std::unique_ptr for explicit lifetime management
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 * - Package cards are dynamically created/destroyed in displayResults/clearResults
 */
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
    
    // Qt parent-child managed widgets (non-owning pointers)
    QLineEdit* m_searchInput = nullptr;
    QPushButton* m_searchButton = nullptr;
    QComboBox* m_filterCombo = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QGridLayout* m_gridLayout = nullptr;
    QLabel* m_statusLabel = nullptr;
    
    // Owned resources
    std::unique_ptr<AurHelper> m_aurHelper;
    
    QVector<PackageInfo> m_currentResults;
    QVector<PackageInfo> m_allResults;
    bool m_searchInProgress = false;
    
private slots:
    void onSearchClicked();
    void onAurSearchCompleted(const QVector<PackageInfo>& results);
    void onAurSearchError(const QString& errorMsg);
    void onFilterChanged(int index);
    void onPackageClicked(const PackageInfo& info);
};

#endif // SEARCH_WIDGET_H
