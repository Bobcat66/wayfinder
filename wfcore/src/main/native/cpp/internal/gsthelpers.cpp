// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "impl/gsthelpers.h"
#include "wfcore/logging/LoggerManager.h"
#include <unordered_map>

#include <gst/gst.h>
#include <unordered_set>

static const std::unordered_set<const gchar*> licenseAllowlist = {
    "LGPL", "LGPL-2.0", "LGPL-2.1", "LGPL-2.1+", "LGPL-3.0", "LGPL-3.0+",
    "MIT", "BSD", "BSD-2-Clause", "BSD-3-Clause", "ISC", "Apache-2.0", "X11"
};
static wf::loggerPtr logger = wf::LoggerManager::getInstance().getLogger("gst-internal",wf::LogGroup::Gstreamer);
static std::unordered_set<std::string> loadedPlugins;
static std::unordered_map<std::string,bool> queriedPlugins;
static std::unordered_set<std::string> pluginBlocklist;
static bool populatedBlocklist = false;

namespace wf::impl {

    void populateGSTBlocklist() {

        GstRegistry *registry = gst_registry_get();
        GList *plugins = gst_registry_get_plugin_list(registry);

        for (const GList *l = plugins; l != NULL; l = l->next) {
            GstPlugin *plugin = (GstPlugin *)l->data;
            const gchar *license = gst_plugin_get_license(plugin);
            const gchar *name = gst_plugin_get_name(plugin);
            if (licenseAllowlist.find(license) == licenseAllowlist.end()) {pluginBlocklist.emplace(name);}
            gst_object_unref(plugin);
        }

        g_list_free(plugins);
        populatedBlocklist = true;
    }

    bool GSTPluginBlocked(const std::string& pluginName) {
        if (!populatedBlocklist) {populateGSTBlocklist();}
        return (pluginBlocklist.find(pluginName) != pluginBlocklist.end());
    }

    bool searchForGSTPlugin(const std::string& pluginName) {
        logger->trace("searching for plugin %s",pluginName);
        auto map_it = queriedPlugins.find(pluginName);

        if (map_it != queriedPlugins.end()) {
            return map_it->second; // Plugin already searched for, we don't need to perform another search
        }

        if (GSTPluginBlocked(pluginName)) {
            logger->warn("Plugin %s is blocked due to potential license incompatibility",pluginName);
            queriedPlugins[pluginName] = false;
            return false;
        }

        // We have not searched for this particular plugin, begin searching for it
        GstRegistry* registry = gst_registry_get();
        GstPlugin* plugin = gst_registry_find_plugin(registry,pluginName.c_str());
        bool found = false;

        if (plugin) {
            found = true;
            gst_object_unref(plugin);
        }

        queriedPlugins[pluginName] = found;
        return found;
    }

    bool loadGSTPlugin(const std::string pluginName) {

        if (loadedPlugins.find(pluginName) != loadedPlugins.end()) {
            logger->debug("Plugin %s already loaded",pluginName);
            return true;
        }

        if (!searchForGSTPlugin(pluginName)){
            logger->warn("Plugin %s not found in registry",pluginName);
            return false;
        }

        logger->info("Attempting to load plugin %s",pluginName);
        GstPlugin* plugin = gst_plugin_load_by_name(pluginName.c_str());

        if (plugin) {
            gst_object_unref(plugin); 
            loadedPlugins.emplace(pluginName);
            return true;
        }

        logger->error("Plugin %s not found",pluginName);
        return false;
    }

    GstElement* getGSTElement(const std::string& elementType, const std::string& instanceName) {
        GstRegistry* registry = gst_registry_get();
        GstPluginFeature* factory = gst_registry_lookup_feature(registry, elementType.c_str());
        
        if (!factory) {
            logger->warn("No factory found for element type %s", elementType);
            return nullptr;
        }

        const gchar* pluginName = gst_plugin_feature_get_plugin_name(factory);
        logger->debug("Element type %s part of plugin %s",elementType,pluginName);

        if (GSTPluginBlocked(pluginName)) {
            gst_object_unref(factory);
            return nullptr;
        }

        if (!loadGSTPlugin(pluginName)) {
            logger->warn("Failed to load plugin %s",pluginName);
            gst_object_unref(factory);
            return nullptr;
        }

        auto plugin = 
    }

}