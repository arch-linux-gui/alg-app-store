#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <memory>
#include <mutex>
#include <thread>

/**
 * @brief Singleton class for managing package operations (install, uninstall, update).
 *
 * Memory Management:
 * - m_process: Owned by std::unique_ptr for RAII-style cleanup and clear ownership
 * - Thread-safe via m_mutex for operation serialization
 * - m_killHelperThread: a std::jthread that runs the `pkexec pkill` calls used
 *   by cancelRunningOperation() (see there for why). Its destructor
 *   auto-requests-stop and joins, so a still-waiting call is interrupted
 *   rather than left dangling if cancelled again or on shutdown.
 */
class PackageManager : public QObject
{
    Q_OBJECT

public:
    enum class Helper
    {
        Pacman,
        Yay,
        Paru
    };

    static PackageManager& instance();

    ~PackageManager() override;

    // Disable copy and move
    PackageManager(const PackageManager&) = delete;
    PackageManager& operator=(const PackageManager&) = delete;
    PackageManager(PackageManager&&) = delete;
    PackageManager& operator=(PackageManager&&) = delete;

    void installPackage(const QString& packageName, const QString& repository = QString());
    void uninstallPackage(const QString& packageName, const QString& repository = QString());
    void updatePackage(const QString& packageName, const QString& repository = QString());
    void updateAllPackages();
    void cancelRunningOperation();
    bool isOperationRunning() const;

    Helper getHelper() const { return m_helper; }
    QString getHelperName() const;

signals:
    void operationStarted(const QString& message);
    void operationOutput(const QString& output);
    void operationCompleted(bool success, const QString& message);
    void operationError(const QString& error);

private:
    PackageManager();

    void detectHelper();
    void executeCommand(const QString& command, const QStringList& args);
    void spawnKillHelper(const QStringList& pkillArgs);

    Helper m_helper = Helper::Pacman;
    std::unique_ptr<QProcess> m_process;
    mutable std::mutex m_mutex;
    std::jthread m_killHelperThread;

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessOutput();
};

#endif  // PACKAGE_MANAGER_H
