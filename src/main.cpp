#include "gui/mainwindow.h"
#include "utils/logger.h"
#include <QApplication>
#include <QStyleFactory>

int
main( int argc, char* argv[] )
{
    QApplication app( argc, argv );

    // Set application metadata
    app.setApplicationName( "ALG App Store" );
    app.setApplicationVersion( "2.0.0" );
    app.setOrganizationName( "Arch Linux GUI" );

    // Set application style
    app.setStyle( QStyleFactory::create( "Fusion" ) );

    // Initialize Logger
    Logger::init();

    Logger::info( "Starting ALG App Store" );
    Logger::info( QString( "Qt version: %1" ).arg( qVersion() ) );

    MainWindow window;
    window.show();

    Logger::info( "Application window shown" );

    int result = app.exec();

    Logger::info( "Application exiting" );
    return result;
}
