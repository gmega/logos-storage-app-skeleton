#include "logos_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <iostream>
#include <thread>

int app_main(LogosModules* modules, int argc, char* argv[]);

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Logos");
    QCoreApplication::setApplicationName("LogosStorageCLI");

    Logos logos(QCoreApplication::applicationDirPath() + "/../modules");
    if (!logos.init()) {
        std::cerr << "Failed to initialize Logos" << std::endl;
        return 1;
    }

    int result = app_main(logos.modules(), argc, argv);
    logos.cleanup();
    return result;
}

int app_main(LogosModules* modules, int argc, char* argv[]) {
    // Your CLI application logic here
    std::cout << "Running app" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}