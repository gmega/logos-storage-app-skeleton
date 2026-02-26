#include "logos_manager.h"

#include <QDebug>
#include <iostream>

extern "C" {
void logos_core_set_plugins_dir(const char* plugins_dir);
void logos_core_start();
void logos_core_cleanup();
char** logos_core_get_loaded_plugins();
int logos_core_load_plugin(const char* plugin_name);
}

Logos::Logos(const QString& pluginsDir)
    : m_pluginsDir(pluginsDir)
{}

Logos::~Logos()
{
    if (m_initialized) {
        cleanup();
    }
}

bool Logos::init(const QString& name)
{
    std::cout << "Setting plugins directory to: " << m_pluginsDir.toStdString() << std::endl;
    logos_core_set_plugins_dir(m_pluginsDir.toUtf8().constData());

    logos_core_start();
    std::cout << "Logos Core started successfully!" << std::endl;

    // capability_module is loaded automatically by logos_core_start()
    if (!logos_core_load_plugin("storage_module")) {
        std::cerr << "Failed to load storage_module plugin" << std::endl;
        return false;
    }
    std::cout << "Successfully loaded storage_module plugin" << std::endl;

    m_api = new LogosAPI(name);
    m_modules = new LogosModules(m_api);
    m_initialized = true;
    return true;
}

void Logos::cleanup()
{
    if (!m_initialized) {
        return;
    }

    qDebug() << "Logos: cleanup...";

    delete m_modules;
    delete m_api;
    logos_core_cleanup();

    m_initialized = false;
}

LogosModules* Logos::modules()
{
    return m_modules;
}
