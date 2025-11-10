#ifndef PACKAGE_DETAILS_DIALOG_H
#define PACKAGE_DETAILS_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include "../utils/types.h"

class PackageDetailsDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit PackageDetailsDialog(const PackageInfo& info, QWidget* parent = nullptr);
    ~PackageDetailsDialog() override = default;
    
private:
    void setupUi();
    void updateButtonStates();
    void checkInstallStatus();
    void showProgress(const QString& message);
    void hideProgress();
    void toggleLogViewer();
    void parseProgressOutput(const QString& output);
    
    PackageInfo m_info;
    bool m_isInstalled;
    
    QLabel* m_nameLabel;
    QLabel* m_versionLabel;
    QLabel* m_repositoryLabel;
    QLabel* m_maintainerLabel;
    QLabel* m_urlLabel;
    QTextEdit* m_descriptionText;
    QTextEdit* m_dependenciesText;
    QLabel* m_lastUpdatedLabel;

    QPushButton* m_installButton;
    QPushButton* m_uninstallButton;
    QPushButton* m_closeButton;

    QLabel* m_statusBadge;

    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    QWidget* m_progressWidget;
    
    // Log viewer
    QTextEdit* m_logViewer;
    QPushButton* m_toggleLogButton;
    QWidget* m_logWidget;
    bool m_logVisible;
    
    // Progress tracking
    QString m_currentOperation;
    int m_totalPackages;
    int m_currentPackage;
    
private slots:
    void onInstall();
    void onUninstall();
    void onOperationStarted(const QString& message);
    void onOperationOutput(const QString& output);
    void onOperationCompleted(bool success, const QString& message);
    void onOperationError(const QString& error);
};

#endif // PACKAGE_DETAILS_DIALOG_H
