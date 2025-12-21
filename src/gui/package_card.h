#ifndef PACKAGE_CARD_H
#define PACKAGE_CARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "../utils/types.h"

/**
 * @brief A clickable card widget displaying package information.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 */
class PackageCard : public QWidget {
    Q_OBJECT
    
public:
    explicit PackageCard(const PackageInfo& info, QWidget* parent = nullptr);
    ~PackageCard() override = default;
    
    const PackageInfo& packageInfo() const { return m_info; }
    void updateInstallStatus(bool installed);
    void checkInstallStatus();
    
signals:
    void clicked(const PackageInfo& info);
    
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    
private:
    void setupUi();
    
    PackageInfo m_info;
    
    // Qt parent-child managed widgets (non-owning pointers)
    QLabel* m_nameLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    QLabel* m_versionLabel = nullptr;
    QLabel* m_repositoryLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    
    bool m_isInstalled = false;
};

#endif // PACKAGE_CARD_H
