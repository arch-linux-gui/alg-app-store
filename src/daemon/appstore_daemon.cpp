#include "appstore_daemon.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <sys/types.h>
#include <unistd.h>

AppStoreDaemon::AppStoreDaemon( QObject* parent )
    : QObject( parent )
    , m_process( std::make_unique< QProcess >() )
{

    connect( m_process.get(), &QProcess::finished, this, &AppStoreDaemon::onProcessFinished );
    connect( m_process.get(), &QProcess::errorOccurred, this, &AppStoreDaemon::onProcessError );
    connect( m_process.get(), &QProcess::readyReadStandardOutput, this, &AppStoreDaemon::onProcessOutput );
    connect( m_process.get(), &QProcess::readyReadStandardError, this, &AppStoreDaemon::onProcessOutput );

    m_process->setProcessChannelMode( QProcess::MergedChannels );
}

AppStoreDaemon::~AppStoreDaemon()
{
    if ( m_process && m_process->state() != QProcess::NotRunning )
    {
        m_process->terminate();
        m_process->waitForFinished( 3000 );
    }
}

bool
AppStoreDaemon::checkAuthorization( const QString& actionId )
{
    if ( !calledFromDBus() )
    {
        qWarning() << "Method not called from D-Bus!";
        return false;
    }

    QDBusMessage msg = message();
    QString sender = msg.service();

    // Call polkit to check authorization
    QDBusInterface polkit( "org.freedesktop.PolicyKit1",
                           "/org/freedesktop/PolicyKit1/Authority",
                           "org.freedesktop.PolicyKit1.Authority",
                           QDBusConnection::systemBus() );

    if ( !polkit.isValid() )
    {
        qCritical() << "Failed to connect to polkit!";
        sendErrorReply( QDBusError::Failed, "Failed to connect to polkit" );
        return false;
    }

    // Prepare Subject struct (SystemBusName)
    QVariantMap subject;
    subject.insert( "subject-kind", "system-bus-name" );
    subject.insert( "name", sender );

    // Prepare Details map (empty)
    QVariantMap details;

    // CheckAuthorization flags:
    // 1 = AllowUserInteraction (show password dialog)
    uint flags = 1;

    QDBusReply< QDBusArgument > reply
        = polkit.call( "CheckAuthorization", QVariant( subject ), actionId, QVariant( details ), flags, "" );

    if ( !reply.isValid() )
    {
        qCritical() << "Polkit call failed:" << reply.error().message();
        sendErrorReply( QDBusError::AccessDenied, "Authorization check failed" );
        return false;
    }

    QDBusArgument arg = reply.value();
    bool isAuthorized = false;
    bool isChallenge = false;
    QVariantMap resultDetails;

    // Parse the struct returned by CheckAuthorization
    // struct { boolean is_authorized, boolean is_challenge, dict details }
    arg.beginStructure();
    arg >> isAuthorized >> isChallenge >> resultDetails;
    arg.endStructure();

    if ( !isAuthorized )
    {
        qWarning() << "Polkit authorization denied for" << actionId;
        sendErrorReply( QDBusError::AccessDenied, "Not authorized" );
        return false;
    }

    return true;
}

void
AppStoreDaemon::executeCommand( const QString& command, const QStringList& args )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    if ( m_process->state() != QProcess::NotRunning )
    {
        emit OperationFinished( false, "Another operation is already running" );
        return;
    }

    QString fullCommand = command + " " + args.join( " " );
    qDebug() << "Executing:" << fullCommand;
    emit OperationOutput( ">> Executing: " + fullCommand + "\n" );

    m_process->start( command, args );
}

void
AppStoreDaemon::StartInstall( const QString& packageName )
{
    setDelayedReply( true );
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;  // sendErrorReply already called
    }

    executeCommand( "pacman", QStringList() << "-S" << packageName << "--noconfirm" );
}

void
AppStoreDaemon::StartLocalInstall( const QString& packagePath )
{
    setDelayedReply( true );
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;
    }

    executeCommand( "pacman", QStringList() << "-U" << packagePath << "--noconfirm" );
}

void
AppStoreDaemon::StartUninstall( const QString& packageName )
{
    setDelayedReply( true );
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;
    }

    executeCommand( "pacman", QStringList() << "-Rdd" << packageName << "--noconfirm" );
}

void
AppStoreDaemon::StartUpdate( const QString& packageName )
{
    setDelayedReply( true );
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;
    }

    executeCommand( "pacman", QStringList() << "-S" << packageName << "--noconfirm" );
}

void
AppStoreDaemon::StartUpdateAll()
{
    setDelayedReply( true );
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;
    }

    executeCommand( "pacman", QStringList() << "-Syu" << "--noconfirm" );
}

void
AppStoreDaemon::StartSyncRepos()
{
    setDelayedReply( true );
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;
    }

    executeCommand( "pacman", QStringList() << "-Sy" );
}

void
AppStoreDaemon::StartSetupChaoticAur()
{
    setDelayedReply( true );
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;
    }

    QString script = "cd /tmp && "
                     "rm -f chaotic-keyring.pkg.tar.zst chaotic-mirrorlist.pkg.tar.zst && "
                     "curl -L -O https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-keyring.pkg.tar.zst && "
                     "curl -L -O https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-mirrorlist.pkg.tar.zst && "
                     "pacman -U --noconfirm chaotic-keyring.pkg.tar.zst chaotic-mirrorlist.pkg.tar.zst";

    executeCommand( "bash", QStringList() << "-c" << script );
}

void
AppStoreDaemon::StartRemoveChaoticAur()
{
    setDelayedReply( true );
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;
    }

    executeCommand( "pacman", QStringList() << "-Rns" << "--noconfirm" << "chaotic-keyring" << "chaotic-mirrorlist" );
}

void
AppStoreDaemon::CancelOperation()
{
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-packages" ) )
    {
        return;
    }

    std::lock_guard< std::mutex > lock( m_mutex );
    if ( m_process && m_process->state() != QProcess::NotRunning )
    {
        emit OperationOutput( "\n>>> Operation cancelled by user <<<\n" );

        QProcess killProcess;
        killProcess.start( "bash", QStringList() << "-c" << "pkill -TERM pacman" );
        killProcess.waitForFinished( 2000 );

        m_process->terminate();
        if ( !m_process->waitForFinished( 3000 ) )
        {
            killProcess.start( "bash", QStringList() << "-c" << "pkill -KILL pacman" );
            killProcess.waitForFinished( 2000 );
            m_process->kill();
            m_process->waitForFinished( 1000 );
        }

        emit OperationFinished( false, "Operation cancelled by user" );
    }
}

bool
AppStoreDaemon::RemoveLockFile()
{
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-config" ) )
    {
        return false;  // Error reply handled
    }

    QFile lockFile( "/var/lib/pacman/db.lck" );
    if ( lockFile.exists() )
    {
        return lockFile.remove();
    }
    return true;
}

// ==========================================
// Config Editing Helper Implementations
// ==========================================

bool
AppStoreDaemon::enableMultilibInPacmanConf()
{
    QFile file( "/etc/pacman.conf" );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return false;
    }

    QStringList lines;
    QTextStream in( &file );
    bool multilibSectionFound = false;

    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( line.trimmed() == "#[multilib]" )
        {
            lines.append( "[multilib]" );
            multilibSectionFound = true;
        }
        else if ( multilibSectionFound && line.trimmed().startsWith( "#Include" ) && line.contains( "mirrorlist" ) )
        {
            lines.append( line.mid( 1 ) );
            multilibSectionFound = false;
        }
        else
        {
            lines.append( line );
        }
    }
    file.close();

    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        return false;
    }
    QTextStream out( &file );
    for ( const QString& line : lines )
    {
        out << line << "\n";
    }
    return true;
}

bool
AppStoreDaemon::disableMultilibInPacmanConf()
{
    QFile file( "/etc/pacman.conf" );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return false;
    }

    QStringList lines;
    QTextStream in( &file );
    bool inMultilibSection = false;

    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        QString trimmedLine = line.trimmed();

        if ( trimmedLine == "[multilib]" )
        {
            lines.append( "#[multilib]" );
            inMultilibSection = true;
        }
        else if ( inMultilibSection && trimmedLine.startsWith( "Include" ) && trimmedLine.contains( "mirrorlist" ) )
        {
            lines.append( "#" + line );
            inMultilibSection = false;
        }
        else if ( trimmedLine.startsWith( "[" ) && trimmedLine != "[multilib]" )
        {
            lines.append( line );
            inMultilibSection = false;
        }
        else
        {
            lines.append( line );
        }
    }
    file.close();

    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        return false;
    }
    QTextStream out( &file );
    for ( const QString& line : lines )
    {
        out << line << "\n";
    }
    return true;
}

bool
AppStoreDaemon::enableChaoticAurInPacmanConf()
{
    QFile file( "/etc/pacman.conf" );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return false;
    }

    QStringList lines;
    QTextStream in( &file );
    bool inCommentedChaoticAurSection = false;
    bool chaoticAurSectionExists = false;

    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        QString trimmedLine = line.trimmed();

        if ( trimmedLine == "[chaotic-aur]" || trimmedLine == "#[chaotic-aur]" )
        {
            chaoticAurSectionExists = true;
            if ( trimmedLine == "#[chaotic-aur]" )
            {
                lines.append( "[chaotic-aur]" );
                inCommentedChaoticAurSection = true;
            }
            else
            {
                lines.append( line );
            }
        }
        else if ( inCommentedChaoticAurSection && trimmedLine.startsWith( "#" )
                  && ( trimmedLine.contains( "Include" ) || trimmedLine.contains( "Server" ) ) )
        {
            lines.append( line.mid( line.indexOf( '#' ) + 1 ) );
            inCommentedChaoticAurSection = false;
        }
        else if ( trimmedLine.startsWith( "[" ) && trimmedLine != "[chaotic-aur]" && trimmedLine != "#[chaotic-aur]" )
        {
            lines.append( line );
            inCommentedChaoticAurSection = false;
        }
        else
        {
            lines.append( line );
        }
    }
    file.close();

    if ( !chaoticAurSectionExists )
    {
        lines.append( "" );
        lines.append( "[chaotic-aur]" );
        lines.append( "Include = /etc/pacman.d/chaotic-mirrorlist" );
    }

    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        return false;
    }
    QTextStream out( &file );
    for ( const QString& line : lines )
    {
        out << line << "\n";
    }
    return true;
}

bool
AppStoreDaemon::disableChaoticAurInPacmanConf()
{
    QFile file( "/etc/pacman.conf" );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return false;
    }

    QStringList lines;
    QTextStream in( &file );
    bool inChaoticAurSection = false;

    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        QString trimmedLine = line.trimmed();

        if ( trimmedLine == "[chaotic-aur]" )
        {
            lines.append( "#[chaotic-aur]" );
            inChaoticAurSection = true;
        }
        else if ( trimmedLine == "#[chaotic-aur]" )
        {
            lines.append( line );
            inChaoticAurSection = false;
        }
        else if ( inChaoticAurSection && !trimmedLine.startsWith( "#" )
                  && ( trimmedLine.startsWith( "Include" ) || trimmedLine.startsWith( "Server" ) ) )
        {
            lines.append( "#" + line );
            inChaoticAurSection = false;
        }
        else if ( trimmedLine.startsWith( "[" ) && trimmedLine != "[chaotic-aur]" && trimmedLine != "#[chaotic-aur]" )
        {
            lines.append( line );
            inChaoticAurSection = false;
        }
        else
        {
            lines.append( line );
        }
    }
    file.close();

    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        return false;
    }
    QTextStream out( &file );
    for ( const QString& line : lines )
    {
        out << line << "\n";
    }
    return true;
}

bool
AppStoreDaemon::EnableMultilib()
{
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-config" ) )
    {
        return false;
    }
    return enableMultilibInPacmanConf();
}

bool
AppStoreDaemon::DisableMultilib()
{
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-config" ) )
    {
        return false;
    }
    return disableMultilibInPacmanConf();
}

bool
AppStoreDaemon::EnableChaoticAur()
{
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-config" ) )
    {
        return false;
    }
    return enableChaoticAurInPacmanConf();
}

bool
AppStoreDaemon::DisableChaoticAur()
{
    if ( !checkAuthorization( "org.arkalinuxgui.appstore.manage-config" ) )
    {
        return false;
    }
    return disableChaoticAurInPacmanConf();
}

// ==========================================
// Process Callbacks
// ==========================================

void
AppStoreDaemon::onProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    if ( exitStatus == QProcess::NormalExit && exitCode == 0 )
    {
        emit OperationFinished( true, "Operation completed successfully" );
    }
    else
    {
        QString error = m_process->readAllStandardError();
        emit OperationFinished( false, QString( "Operation failed (exit %1): %2" ).arg( exitCode ).arg( error ) );
    }

    // Explicitly send delayed reply for the DBus method call if we need to
    // Since we stream continuously and manage our own signals, we might just let the caller
    // rely on signals and not wait on the method reply, but returning valid is good practice.
}

void
AppStoreDaemon::onProcessError( QProcess::ProcessError error )
{
    emit OperationFinished( false, QString( "Process error: %1" ).arg( m_process->errorString() ) );
}

void
AppStoreDaemon::onProcessOutput()
{
    QString output = m_process->readAll();
    if ( !output.isEmpty() )
    {
        emit OperationOutput( output );
    }
}
