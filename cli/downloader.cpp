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

int app_main(LogosModules* modules, int argc, char* argv[]);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <upload|download> <arguments>" << std::endl;
        return 1;
    }

    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Logos");
    QCoreApplication::setApplicationName("LogosDownloader");

    Logos logos(QCoreApplication::applicationDirPath() + "/../modules");
    if (!logos.init("LogosDownloader")) {
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
      "\"listen-port\": 8001,"
      "\"disc-port\": 9001,"
      "\"nat\": \"none\","
      "\"data-dir\": \"./downloader-data\","
      "\"bootstrap-node\": [\"" + QString(argv[1]) + "\"]"
    "}";

    if (!modules->storage_module.init(jsonConfig)) {
        std::cerr << "Failed to initialize storage module" << std::endl;
        return 1;
    }

    QUrl url = QUrl::fromLocalFile(argv[3]);
    QString cid = argv[2];

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

    {
      QEventLoop loop;

      modules->storage_module.on("storageDownloadDone", [&loop](const QVariantList& data) {
        bool success = data[0].toBool();
        if (!success) {
          std::cerr << "Failed to download file: " << data[2].toString().toStdString() << std::endl;
        }
        notify(&loop, success);
      });

      modules->storage_module.downloadToUrl(cid, url);

      if (!await(&loop, DEFAULT_TIMEOUT)) {
        return 1;
      }
    }

    std::cerr << "Download completed successfully." << std::endl;
    return 0;
}