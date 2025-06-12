// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>
#include <gst/gst.h>
#include <gst/gstplugin.h>
#include <gst/gstelementfactory.h>

#define GST_ELEMENT(x) static_cast<GstElement*>(x)

namespace wf::impl {
    // Populates the plugin blocklist
    void populateGSTBlocklist();
    // Check if a plugin is blocked. Some plugins are blocked due to license incompatibility
    bool GSTPluginBlocked(const std::string& pluginName);
    inline bool GSTPluginBlocked(const gchar* pluginName) {return GSTPluginBlocked(std::string(pluginName));}
    inline bool GSTPluginBlocked(GstPlugin* plugin) {return GSTPluginBlocked(std::string(gst_plugin_get_name(plugin)));}
    // Searches for a plugin in the gstreamer registry, returns whether or not it was found
    bool findGSTPlugin(const std::string& pluginName);
    inline bool findGSTPlugin(const gchar* pluginName) {return findGSTPlugin(std::string(pluginName));}
    // Loads a plugin, returns whether or not the operation was successful
    bool loadGSTPlugin(const std::string& pluginName);
    inline bool loadGSTPlugin(const gchar* pluginName) {return loadGSTPlugin(std::string(pluginName));}

    // Fetches a GST element, returns a nullptr if the operation fails
    GstElement* getGSTElement(const std::string& elementType, std::string& instanceName);
    inline GstElement* getGSTElement(const gchar* elementType, const gchar* instanceName) {
        return getGSTElement(std::string(elementType),std::string(instanceName));
    }
}