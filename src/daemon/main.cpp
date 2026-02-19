#include "appstore_daemon.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

int
main( int argc, char* argv[] )
{
    QCoreApplication app( argc, argv );

    if ( !QDBusConnection::systemBus().isConnected() )
    {
        qCritical() << "Cannot connect to the D-Bus system bus.\n"
                    << "To start it, run:\n"
                    << "\teval `dbus-launch --auto-syntax`\n";
        return 1;
    }

    AppStoreDaemon daemon;

    // Register our object
    if ( !QDBusConnection::systemBus().registerObject( "/org/arkalinuxgui/AppStore",
                                                       &daemon,
                                                       QDBusConnection::ExportAllSlots
                                                           | QDBusConnection::ExportAllSignals ) )
    {
        qCritical() << "Cannot register D-Bus object:" << QDBusConnection::systemBus().lastError().message();
        return 1;
    }

    // Register the service name
    if ( !QDBusConnection::systemBus().registerService( "org.arkalinuxgui.AppStore" ) )
    {
        qCritical() << "Cannot set D-Bus service name:" << QDBusConnection::systemBus().lastError().message();
        return 1;
    }

    qDebug() << "AppStore daemon started successfully. Listening for D-Bus requests...";
    return app.exec();
}
