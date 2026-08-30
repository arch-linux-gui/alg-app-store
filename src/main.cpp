#include "gui/mainwindow.h"
#include "utils/logger.h"
#include "utils/version.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("Explorer");
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("Arch Linux GUI");

    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));

    Logger::info("Starting Explorer");
    Logger::info(QString("Qt version: %1").arg(qVersion()));
    
    MainWindow window;
    window.show();
    
    Logger::info("Application window shown");
    
    int result = app.exec();
    
    Logger::info("Application exiting");
    return result;
}
