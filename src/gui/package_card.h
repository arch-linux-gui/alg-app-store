#ifndef PACKAGE_CARD_H
#define PACKAGE_CARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "../utils/types.h"

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
    QLabel* m_nameLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_versionLabel;
    QLabel* m_repositoryLabel;
    QLabel* m_statusLabel;
    bool m_isInstalled;
};

#endif // PACKAGE_CARD_H
