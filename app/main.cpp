#include "logos_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <iostream>

#define DEFAULT_TIMEOUT 10000

int app_main(LogosModules* modules, int argc, char* argv[]);

bool await(QEventLoop* loop, int timeoutMs) {
  QTimer::singleShot(timeoutMs, loop, [loop]() {
    std::cerr << "Call timed out." << std::endl;
    loop->exit(1);
  });
  return loop->exec() == 0;
}

void notify(QEventLoop* loop, bool successValue) {
  loop->exit(successValue ? 0 : 1);
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Logos");
    QCoreApplication::setApplicationName("LogosUploader");

    Logos logos(QCoreApplication::applicationDirPath() + "/../modules");
    if (!logos.init("LogosUploader")) {
        std::cerr << "Failed to initialize Logos" << std::endl;
        return 1;
    }

    int result = app_main(logos.modules(), argc, argv);
    logos.cleanup();
    return result;
}

int app_main(LogosModules* modules, int argc, char* argv[]) {

    const QString jsonConfig = "{"
      "\"listen-addrs\": [\"/ip4/0.0.0.0/tcp/8000\"],"
      "\"disc-port\": 9000,"
      "\"data-dir\": \"./app-data\","
      "\"nat\": \"none\""
    "}";

    if (!modules->storage_module.init(jsonConfig)) {
        std::cerr << "Failed to initialize storage module" << std::endl;
        return 1;
    }

    {
      QEventLoop loop;
      modules->storage_module.on("storageStart", [&loop](const QVariantList& data) {
        bool success = data[0].toBool();
        if (!success) {
          std::cerr << "Failed to start storage module: " << data[1].toString().toStdString() << std::endl;
        }
        notify(&loop, success);
      });

      modules->storage_module.start();

      if (!await(&loop, DEFAULT_TIMEOUT)) {
        return 1;
      }

      std::cerr << "Storage module started successfully" << std::endl;
    }

    // Rest of the app goes here.

    std::cerr << "Exiting..." << std::endl;
    return 0;
}
