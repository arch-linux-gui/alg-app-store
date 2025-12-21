#ifndef PACKAGE_DETAILS_DIALOG_H
#define PACKAGE_DETAILS_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include "../utils/types.h"

/**
 * @brief Dialog showing detailed package information and actions.
 * 
 * Memory Management:
 * - All Qt widget members use Qt parent-child ownership (raw pointers are non-owning)
 */
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
    QString findDesktopFile() const;
    bool verifyDesktopFile(const QString& desktopFilePath,
                           const QStringList& nameVariants) const;
    void launchApplication();
    
    PackageInfo m_info;
    bool m_isInstalled = false;
    
    // Qt parent-child managed widgets (non-owning pointers)
    QLabel* m_nameLabel = nullptr;
    QLabel* m_versionLabel = nullptr;
    QLabel* m_repositoryLabel = nullptr;
    QLabel* m_maintainerLabel = nullptr;
    QLabel* m_urlLabel = nullptr;
    QTextEdit* m_descriptionText = nullptr;
    QTextEdit* m_dependenciesText = nullptr;
    QLabel* m_lastUpdatedLabel = nullptr;

    QPushButton* m_installButton = nullptr;
    QPushButton* m_uninstallButton = nullptr;
    QPushButton* m_launchButton = nullptr;
    QPushButton* m_closeButton = nullptr;

    QLabel* m_statusBadge = nullptr;

    QProgressBar* m_progressBar = nullptr;
    QLabel* m_progressLabel = nullptr;
    QWidget* m_progressWidget = nullptr;
    
    // Log viewer (Qt parent-child managed)
    QTextEdit* m_logViewer = nullptr;
    QPushButton* m_toggleLogButton = nullptr;
    QWidget* m_logWidget = nullptr;
    bool m_logVisible = false;
    
    // Progress tracking
    QString m_currentOperation;
    int m_totalPackages = 0;
    int m_currentPackage = 0;
    
private slots:
    void onInstall();
    void onUninstall();
    void onOperationStarted(const QString& message);
    void onOperationOutput(const QString& output);
    void onOperationCompleted(bool success, const QString& message);
    void onOperationError(const QString& error);
};

#endif // PACKAGE_DETAILS_DIALOG_H
