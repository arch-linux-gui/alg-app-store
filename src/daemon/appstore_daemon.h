#ifndef APPSTORE_DAEMON_H
#define APPSTORE_DAEMON_H

#include <QDBusContext>
#include <QObject>
#include <QProcess>
#include <memory>
#include <mutex>

class AppStoreDaemon : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", "org.arkalinuxgui.AppStore" )

public:
    explicit AppStoreDaemon( QObject* parent = nullptr );
    ~AppStoreDaemon();

public slots:
    // D-Bus Methods
    void StartInstall( const QString& packageName );
    void StartLocalInstall( const QString& packagePath );
    void StartUninstall( const QString& packageName );
    void StartUpdate( const QString& packageName );
    void StartUpdateAll();
    void StartSetupChaoticAur();
    void StartRemoveChaoticAur();
    void StartSyncRepos();
    void CancelOperation();

    bool EnableMultilib();
    bool DisableMultilib();
    bool EnableChaoticAur();
    bool DisableChaoticAur();
    bool RemoveLockFile();

signals:
    // D-Bus Signals
    void OperationOutput( const QString& output );
    void OperationFinished( bool success, const QString& message );

private slots:
    void onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void onProcessError( QProcess::ProcessError error );
    void onProcessOutput();

private:
    bool checkAuthorization( const QString& actionId );
    void executeCommand( const QString& command, const QStringList& args );

    // Config editing helpers
    bool enableMultilibInPacmanConf();
    bool disableMultilibInPacmanConf();
    bool enableChaoticAurInPacmanConf();
    bool disableChaoticAurInPacmanConf();

    std::unique_ptr< QProcess > m_process;
    std::mutex m_mutex;
};

#endif  // APPSTORE_DAEMON_H
