#include "logos_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <iostream>

int app_main(LogosModules* modules, int argc, char* argv[]);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <upload|download> <arguments>" << std::endl;
        return 1;
    }

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
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <bootstrap-spr> <cid> <file>" << std::endl;
        return 1;
    }

    const QString jsonConfig = "{"
      "\"listen-addrs\": [\"/ip4/0.0.0.0/tcp/8001\"],"
      "\"disc-port\": 9001,"
      "\"nat\": \"none\""
    "}";

    if (!modules->storage_module.init(jsonConfig)) {
        std::cerr << "Failed to initialize storage module" << std::endl;
        return 1;
    }

    return 0;
}