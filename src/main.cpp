#include "gui/mainwindow.h"
#include "utils/logging.h"
#include "utils/version.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
    // Parses and strips -v/-vv/-vvv and -D <N> before Qt ever sees argv.
    Log::init(argc, argv);

    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("Explorer");
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("Arch Linux GUI");

    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));

    spdlog::info("Starting Explorer");
    spdlog::info("{}", (QString("Qt version: %1").arg(qVersion())).toStdString());
    
    MainWindow window;
    window.show();
    
    spdlog::info("Application window shown");
    
    int result = app.exec();
    
    spdlog::info("Application exiting");
    return result;
}
