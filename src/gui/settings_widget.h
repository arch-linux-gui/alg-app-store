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
    
signals:
    void multilibStatusChanged(bool enabled);
    
private:
    void setupUi();
    void loadCurrentSettings();
    void createRepositorySettings();
    bool isMultilibEnabledInPacmanConf() const;
    bool enableMultilibInPacmanConf();
    bool disableMultilibInPacmanConf();
    void applySettings();
    
    // Repository settings
    QGroupBox* m_repositoryGroup;
    QCheckBox* m_coreRepoCheckbox;
    QCheckBox* m_extraRepoCheckbox;
    QCheckBox* m_multilibRepoCheckbox;
    
    // Control buttons
    QPushButton* m_applyButton;
    QPushButton* m_revertButton;
    
    // Status
    QLabel* m_statusLabel;
    
    // Track original state
    bool m_originalMultilibState;
    
private slots:
    void onApplyClicked();
    void onRevertClicked();
    void onSettingsChanged();
};

#endif // SETTINGS_WIDGET_H
