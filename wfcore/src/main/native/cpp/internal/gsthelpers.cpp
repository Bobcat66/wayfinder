#include "internal/gsthelpers.h"
#include <gst/gst.h>
#include <unordered_map>

using namespace wf::internal;

static std::unordered_map<std::string,bool> queriedPlugins;

bool searchForGSTPlugin(std::string pluginName) {
    auto map_it = queriedPlugins.find(pluginName);
    if (map_it != queriedPlugins.end()) {
        return map_it->second; // Plugin already searched for
    }
    // We have not searched for this particular plugin, begin searching for it
    GstRegistry* registry = gst_registry_get();
    GstPlugin* plugin = gst_registry_find_plugin(registry,pluginName.c_str());
    bool found = false;
    if (plugin) {
        gst_object_unref(plugin);
        found = true;
    }
    queriedPlugins[pluginName] = found;
    return found;
}