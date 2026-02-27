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
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file path>" << std::endl;
        return 1;
    }

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

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    const QString jsonConfig = "{"
      "\"listen-port\": 8000,"
      "\"disc-port\": 9000,"
      "\"data-dir\": \"./uploader-data\","
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
    }

    LogosResult spr = modules->storage_module.spr();
    if (!spr.success) {
        std::cerr << "Failed to get SPR: " << spr.getValue<QString>().toStdString() << std::endl;
        return 1;
    }

    std::cerr << "SPR: " << spr.getValue<QString>().toStdString() << std::endl;

    {
      QEventLoop loop;

      // Uploads file.
      modules->storage_module.on("storageUploadDone", [&loop](const QVariantList& data) {
          bool success = data[0].toBool();
          if (!success) {
              std::cerr << "Failed to upload file: " << data[2].toString().toStdString() << std::endl;
              notify(&loop, false);
              return;
          }

          std::cout << "CID: " << data[2].toString().toStdString() << std::endl;
          notify(&loop, true);
      });

      QUrl url = QUrl::fromLocalFile(argv[1]);
      LogosResult result = modules->storage_module.uploadUrl(url);
      if (!result.success) {
          std::cerr << "Failed to upload file: " << result.getValue<QString>().toStdString() << std::endl;
          return 1;
      }

      if (!await(&loop, DEFAULT_TIMEOUT)) {
        return 1;
      }
    }

    std::cout << "Upload completed successfully. Type CTRL+C to exit." << std::endl;
    return QCoreApplication::exec();
}
