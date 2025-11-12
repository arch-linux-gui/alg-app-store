#ifndef SETTINGS_WIDGET_H
#define SETTINGS_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>

class SettingsWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    ~SettingsWidget() override = default;
    
    bool isMultilibEnabled() const;
    bool isChaoticAurEnabled() const;
    
signals:
    void multilibStatusChanged(bool enabled);
    void chaoticAurStatusChanged(bool enabled);
    
private:
    void setupUi();
    void loadCurrentSettings();
    void createRepositorySettings();
    void createChaoticAurSettings();
    void createMaintenanceSettings();
    bool isMultilibEnabledInPacmanConf() const;
    bool isChaoticAurEnabledInPacmanConf() const;
    bool enableMultilibInPacmanConf();
    bool disableMultilibInPacmanConf();
    bool enableChaoticAurInPacmanConf();
    bool disableChaoticAurInPacmanConf();
    void applySettings();
    
    // Repository settings
    QGroupBox* m_repositoryGroup;
    QCheckBox* m_coreRepoCheckbox;
    QCheckBox* m_extraRepoCheckbox;
    QCheckBox* m_multilibRepoCheckbox;
    QCheckBox* m_chaoticAurCheckbox;
    
    // Chaotic-AUR setup
    QGroupBox* m_chaoticAurGroup;
    QPushButton* m_setupChaoticButton;
    QPushButton* m_removeChaoticButton;
    
    // Maintenance settings
    QGroupBox* m_maintenanceGroup;
    QPushButton* m_removeLockButton;
    QPushButton* m_syncReposButton;
    
    // Control buttons
    QPushButton* m_applyButton;
    QPushButton* m_revertButton;
    
    // Status
    QLabel* m_statusLabel;
    
    // Track original state
    bool m_originalMultilibState;
    bool m_originalChaoticAurState;
    
private slots:
    void onApplyClicked();
    void onRevertClicked();
    void onSettingsChanged();
    void onSetupChaoticClicked();
    void onRemoveChaoticClicked();
    void onRemoveLockClicked();
    void onSyncReposClicked();
};

#endif // SETTINGS_WIDGET_H
