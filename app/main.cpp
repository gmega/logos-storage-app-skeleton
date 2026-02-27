// This file contains the basic skeleton for a Logos Storage CLI application.
// It implements basic module initialization and startup, and provides simple
// synchronization primitives.

#include "logos_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <iostream>

// Default timeout for operations, in milliseconds. It is in general
// not a good idea to use the same timeout for all operations, but
// fine for an example.
#define DEFAULT_TIMEOUT 10000

// The main function for the app. This is where application logic
// should go.
int app_main(LogosModules* modules, int argc, char* argv[]);


// The storage module runs operations asynchronously, and we'd often
// like to wait for such operations to complete before we do something
// else; e.g., wait for the module to initialize before we upload a file.
//
// Since Logos Core is Qt-based, we provide two simple synchronization
// primitives which are friendly to Qt's event loop: an `await` call,
// which can be invoked by the main thread to block and wait for an
// operation, and a `notify` call, which can be invoked by the
// operation's signal handler to "wake up" the main thread when it
// completes.

// "Blocks" the main thread until `notify` is called on the
// corresponding signal handler.
bool await(QEventLoop* loop, int timeoutMs) {
  QTimer::singleShot(timeoutMs, loop, [loop]() {
    std::cerr << "Call timed out." << std::endl;
    loop->exit(1);
  });
  return loop->exec() == 0;
}

// "Wakes up" the main thread, signalling that an operation
// has completed. Typically called by a signal handler.
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

    // See https://github.com/logos-co/logos-storage-module/blob/7512d6745f9f412220e0c589e3f82fe454405e6d/storage_module_interface.h#L13
    // for a full list of options.
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

    // This showcases the use of await/notify. Each await/notify pair requires the use of
    // a distinct, scoped event loop.
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
