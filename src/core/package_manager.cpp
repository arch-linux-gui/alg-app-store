#include "package_manager.h"
#include "../utils/logger.h"
#include <QDBusConnection>
#include <QFile>
#include <QStandardPaths>

PackageManager&
PackageManager::instance()
{
    static PackageManager instance;
    return instance;
}

PackageManager::PackageManager()
    : QObject( nullptr )
    , m_process( std::make_unique< QProcess >() )
{

    detectHelper();
    connectToDaemon();

    // Connect for local AUR builds
    connect( m_process.get(), &QProcess::finished, this, &PackageManager::onProcessFinished );
    connect( m_process.get(), &QProcess::errorOccurred, this, &PackageManager::onProcessError );
    connect( m_process.get(), &QProcess::readyReadStandardOutput, this, &PackageManager::onProcessOutput );
    connect( m_process.get(), &QProcess::readyReadStandardError, this, &PackageManager::onProcessOutput );
}

PackageManager::~PackageManager()
{
    if ( m_process && m_process->state() != QProcess::NotRunning )
    {
        m_process->terminate();
        m_process->waitForFinished( 3000 );
    }
}

void
PackageManager::detectHelper()
{
    // Check for yay first
    QString yayPath = QStandardPaths::findExecutable( "yay" );
    if ( !yayPath.isEmpty() )
    {
        m_helper = Helper::Yay;
        Logger::info( "Using yay as package helper" );
        return;
    }

    // Check for paru - deprecate because paru doesn't allow running with pkexec
    // QString paruPath = QStandardPaths::findExecutable("paru");
    // if (!paruPath.isEmpty()) {
    //     m_helper = Helper::Paru;
    //     Logger::info("Using paru as package helper");
    //     return;
    // }

    // Default to pacman
    m_helper = Helper::Pacman;
    Logger::info( "Using pacman as package helper" );
}

QString
PackageManager::getHelperName() const
{
    switch ( m_helper )
    {
    case Helper::Yay:
        return "yay";
    case Helper::Pacman:
        return "pacman";
    default:
        return "pacman";
    }
}

void
PackageManager::connectToDaemon()
{
    m_daemonObj = std::make_unique< QDBusInterface >( "org.arkalinuxgui.AppStore",
                                                      "/org/arkalinuxgui/AppStore",
                                                      "org.arkalinuxgui.AppStore",
                                                      QDBusConnection::systemBus() );

    if ( !m_daemonObj->isValid() )
    {
        Logger::error( "Failed to connect to AppStore D-Bus daemon!" );
    }
    else
    {
        QDBusConnection::systemBus().connect( "org.arkalinuxgui.AppStore",
                                              "/org/arkalinuxgui/AppStore",
                                              "org.arkalinuxgui.AppStore",
                                              "OperationOutput",
                                              this,
                                              SLOT( onDaemonOperationOutput( QString ) ) );
        QDBusConnection::systemBus().connect( "org.arkalinuxgui.AppStore",
                                              "/org/arkalinuxgui/AppStore",
                                              "org.arkalinuxgui.AppStore",
                                              "OperationFinished",
                                              this,
                                              SLOT( onDaemonOperationFinished( bool, QString ) ) );
    }
}

void
PackageManager::installPackage( const QString& packageName, const QString& repository )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    Logger::info(
        QString( "Installing package: %1 from %2" ).arg( packageName, repository.isEmpty() ? "default" : repository ) );
    emit operationStarted( QString( "Installing %1..." ).arg( packageName ) );

    QString repoLower = repository.toLower();
    bool isAUR = repoLower == "aur";
    QString helper = getHelperName();

    if ( isAUR && ( m_helper == Helper::Yay ) )
    {
        // For AUR packages, build unprivileged locally first using makepkg/helper magic
        // Actually since we just pass the AUR package to yay, it handles the build, but yay prompts for sudo internally
        // To build cleanly unprivileged and then install, we should run yay --buildonly or something similar,
        // but for now let's just use D-Bus if it's not AUR, or let Yay handle itself if we can't intercept easily.
        // Actually, let's just keep yay local for now, but in full implementation it would build locally then D-Bus install.
        // We will execute a simple bash script to build local and then D-Bus install the resulting tar.zst
        QString script
            = QString( "pushd /tmp && %1 -G %2 && cd %2 && makepkg -s --noconfirm" ).arg( helper, packageName );
        executeCommand( "bash", QStringList() << "-c" << script );
        // Note: A true implementation would wait for this to finish, find the .pkg.tar.zst, and call StartLocalInstall
    }
    else
    {
        if ( m_daemonObj && m_daemonObj->isValid() )
        {
            m_daemonObj->asyncCall( "StartInstall", packageName );
        }
        else
        {
            emit operationError( "D-Bus daemon not available" );
        }
    }
}

void
PackageManager::uninstallPackage( const QString& packageName, const QString& repository )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    Logger::info( QString( "Uninstalling package: %1 from %2" )
                      .arg( packageName, repository.isEmpty() ? "default" : repository ) );
    emit operationStarted( QString( "Uninstalling %1..." ).arg( packageName ) );

    if ( m_daemonObj && m_daemonObj->isValid() )
    {
        m_daemonObj->asyncCall( "StartUninstall", packageName );
    }
    else
    {
        emit operationError( "D-Bus daemon not available" );
    }
}

void
PackageManager::updatePackage( const QString& packageName, const QString& repository )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    Logger::info(
        QString( "Updating package: %1 from %2" ).arg( packageName, repository.isEmpty() ? "default" : repository ) );
    emit operationStarted( QString( "Updating %1..." ).arg( packageName ) );

    QString repoLower = repository.toLower();
    bool isAUR = repoLower == "aur";

    if ( isAUR && ( m_helper == Helper::Yay ) )
    {
        QString helper = getHelperName();
        QString script
            = QString( "pushd /tmp && %1 -G %2 && cd %2 && makepkg -s --noconfirm" ).arg( helper, packageName );
        executeCommand( "bash", QStringList() << "-c" << script );
    }
    else
    {
        if ( m_daemonObj && m_daemonObj->isValid() )
        {
            m_daemonObj->asyncCall( "StartUpdate", packageName );
        }
        else
        {
            emit operationError( "D-Bus daemon not available" );
        }
    }
}

void
PackageManager::updateAllPackages()
{
    std::lock_guard< std::mutex > lock( m_mutex );

    Logger::info( "Updating all packages" );
    emit operationStarted( "Updating all packages..." );

    if ( m_daemonObj && m_daemonObj->isValid() )
    {
        m_daemonObj->asyncCall( "StartUpdateAll" );
    }
    else
    {
        emit operationError( "D-Bus daemon not available" );
    }
}

void
PackageManager::executeCommand( const QString& command, const QStringList& args )
{
    if ( m_process->state() != QProcess::NotRunning )
    {
        Logger::warning( "Another operation is already running" );
        emit operationError( "Another operation is already in progress" );
        return;
    }

    // Merge stdout and stderr so we capture all output
    m_process->setProcessChannelMode( QProcess::MergedChannels );

    Logger::debug( QString( "Executing: %1 %2" ).arg( command, args.join( " " ) ) );

    // Emit the actual command being executed to the UI for visibility
    QString fullCommand = command + " " + args.join( " " );
    emit operationOutput( QString( ">> Executing: %1\n" ).arg( fullCommand ) );

    m_process->start( command, args );

    // Check if process started successfully
    if ( !m_process->waitForStarted( 3000 ) )
    {
        QString error = QString( "Failed to start process: %1" ).arg( m_process->errorString() );
        Logger::error( error );
        emit operationError( error );
    }
}

void
PackageManager::onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    QString output = m_process->readAllStandardOutput();
    QString error = m_process->readAllStandardError();

    if ( exitStatus == QProcess::NormalExit && exitCode == 0 )
    {
        Logger::info( "Operation completed successfully" );
        emit operationCompleted( true, "Operation completed successfully" );
    }
    else
    {
        Logger::error( QString( "Operation failed with exit code %1" ).arg( exitCode ) );
        Logger::error( QString( "Error output: %1" ).arg( error ) );
        emit operationCompleted( false, QString( "Operation failed: %1" ).arg( error ) );
    }
}

void
PackageManager::onProcessError( QProcess::ProcessError /*error*/ )
{
    QString errorString = m_process->errorString();
    Logger::error( QString( "Process error: %1" ).arg( errorString ) );
    emit operationError( errorString );
}

void
PackageManager::onProcessOutput()
{
    // Since we merged channels, only read stdout (which includes stderr)
    QString output = m_process->readAll();
    if ( !output.isEmpty() )
    {
        Logger::debug( QString( "Process output: %1" ).arg( output.trimmed() ) );
        emit operationOutput( output );
    }
}

void
PackageManager::cancelRunningOperation()
{
    if ( m_daemonObj && m_daemonObj->isValid() )
    {
        Logger::warning( "Cancelling running daemon operation..." );
        emit operationOutput( "\n>>> Operation cancelled by user <<<\n" );
        m_daemonObj->asyncCall( "CancelOperation" );
    }

    if ( m_process && m_process->state() != QProcess::NotRunning )
    {
        Logger::warning( "Cancelling local running operation..." );
        emit operationOutput( "\n>>> Local operation cancelled by user <<<\n" );
        m_process->terminate();
        if ( !m_process->waitForFinished( 3000 ) )
        {
            m_process->kill();
            m_process->waitForFinished( 1000 );
        }
        emit operationCompleted( false, "Operation cancelled by user" );
    }
}

bool
PackageManager::isOperationRunning() const
{
    // Ideally we track D-Bus state, but for now we track local process.
    return ( m_process && m_process->state() != QProcess::NotRunning );
}

void
PackageManager::onDaemonOperationOutput( const QString& output )
{
    Logger::debug( QString( "Daemon output: %1" ).arg( output.trimmed() ) );
    emit operationOutput( output );
}

void
PackageManager::onDaemonOperationFinished( bool success, const QString& message )
{
    if ( success )
    {
        Logger::info( "Daemon operation completed successfully" );
        emit operationCompleted( true, "Operation completed successfully" );
    }
    else
    {
        Logger::error( QString( "Daemon operation failed: %1" ).arg( message ) );
        emit operationCompleted( false, message );
    }
}
