#pragma once

#include <QString>
#include <memory>

#include "logos_sdk.h"
#include "logos_api.h"

// Logos manages the lifecycle of the Logos Core and its plugins.
// It handles initialization and cleanup of the Logos SDK.
//
// Usage:
//   Logos logos(pluginsDir);
//   if (!logos.init()) { return 1; }
//
//   StorageModule storage(logos.api());
//   LogosResult result = storage.version();
//
//   // cleanup() is called automatically in destructor
class Logos {
public:
    explicit Logos(const QString& pluginsDir);
    ~Logos();

    // Initialize the Logos Core: set plugins directory, start, and load required plugins.
    // Returns true on success.
    bool init();

    // Cleanup the Logos Core. Called automatically in destructor.
    void cleanup();

    LogosModules* modules();

private:
    QString m_pluginsDir;
    bool m_initialized = false;
    LogosAPI* m_api = nullptr;
    LogosModules* m_modules = nullptr;
};
