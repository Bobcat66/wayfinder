/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "wfcore/common/envutils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <format>

// These are not in the hot path, performance is not important
namespace impl {
    namespace fs = std::filesystem;
    constexpr std::uint32_t ENV_OK =                 0x00000000;
    constexpr std::uint32_t ENV_BAD_ENVIRONMENT =    0x00b00001;
    constexpr std::uint32_t ENV_BAD_KEY =            0x00b00002;
    constexpr std::uint32_t ENV_BAD_FORMAT =         0x00b00003;
    constexpr std::uint32_t ENV_BAD_RANGE =          0x00b00004;
    constexpr std::uint32_t ENV_UNKNOWN =            0x00bfffff;

    static thread_local std::uint32_t enverr = ENV_OK;
    
    static void set_enverr(std::uint32_t err) {
        enverr = err;
    }

    static void clear_enverr() {
        enverr = ENV_OK;
    }

    static uint32_t get_enverr() {
        return enverr;
    }
}

namespace wf::env {
    using JSON = nlohmann::json;
    using namespace impl;

    std::optional<std::string> getVar(const char* key, bool verbose) {
        const char* raw = std::getenv(key);
        if (raw) {
            clear_enverr();
            return std::string(raw);
        } else {
            set_enverr(ENV_BAD_KEY);
            if (verbose) std::cerr << "Warning: environment variable '" << key << "' not found\n";
            return std::nullopt;
        }
    }

    std::optional<int> getInt(const char* key, bool verbose) {
        auto env_var = getVar(key,verbose);
        if (!env_var.has_value()) return std::nullopt;
        try {
            int value = std::stoi(env_var.value());
            return value;
        } catch (const std::invalid_argument& e) {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "Warning: invalid integer string in environment variable '" << key <<"'\n";
            return std::nullopt;
        } catch (const std::out_of_range& e) {
            set_enverr(ENV_BAD_RANGE);
            if (verbose) std::cerr << "Warning: value out of range while reading environment variable '" << key << "'\n";
            return std::nullopt;
        }
    }

    std::optional<double> getDouble(const char* key, bool verbose) {
        auto env_var = getVar(key,verbose);
        if (!env_var.has_value()) return std::nullopt;
        try {
            double value = std::stod(env_var.value());
            return value;
        } catch (const std::invalid_argument& e) {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "Warning: invalid double string in environment variable '" << key <<"'\n";
            return std::nullopt;
        } catch (const std::out_of_range& e) {
            set_enverr(ENV_BAD_RANGE);
            if (verbose) std::cerr << "Warning: value out of range while reading environment variable '" << key << "'\n";
            return std::nullopt;
        }
    }

    std::optional<bool> getBool(const char* key, bool verbose) {
        static const std::string truthy = "true";
        static const std::string falsy = "false";
        auto env_var = getVar(key,verbose);
        if (!env_var) return std::nullopt;
        if (truthy == env_var.value()) {
            return true;
        } else if (falsy == env_var.value()) {
            return false;
        } else {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "Warning: Environment variable '" << key << "' is not formatted as a boolean!";
            return std::nullopt;
        }
    }

    std::optional<std::vector<int>> getIntArr(const char* key, bool verbose) {
        auto env_var = getVar(key,verbose);
        if (!env_var) return std::nullopt;
        JSON jarray;
        try {
            jarray = JSON::parse(env_var.value());
        } catch (const JSON::parse_error& e) {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "JSON parse error while parsing int array from '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        } catch (const JSON::exception& e) {
            set_enverr(ENV_UNKNOWN);
            if (verbose) std::cerr << "Unknown JSON error while parsing int array from '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        }
        if (!jarray.is_array()) {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "Warning: environment variable '" << key << "' is not an array\n";
            return std::nullopt;
        }
        try {
            auto arr = jarray.get<std::vector<int>>();
            return arr;
        } catch (const JSON::type_error& e) {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "Warning: type error while parsing environment variable '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        } catch (const JSON::exception& e) {
            set_enverr(ENV_UNKNOWN);
            if (verbose) std::cerr << "Warning: unknown error while parsing environment variable '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<std::vector<double>> getDoubleArr(const char* key, bool verbose) {
        auto env_var = getVar(key,verbose);
        if (!env_var) return std::nullopt;
        JSON jarray;
        try {
            jarray = JSON::parse(env_var.value());
        } catch (const JSON::parse_error& e) {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "JSON parse error while parsing double array from '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        } catch (const JSON::exception& e) {
            set_enverr(ENV_UNKNOWN);
            if (verbose) std::cerr << "Unknown JSON error while parsing double array from '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        }
        if (!jarray.is_array()) {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "Warning: environment variable '" << key << "' is not an array\n";
            return std::nullopt;
        }
        try {
            auto arr = jarray.get<std::vector<double>>();
            return arr;
        } catch (const JSON::type_error& e) {
            set_enverr(ENV_BAD_FORMAT);
            if (verbose) std::cerr << "Warning: type error while parsing environment variable '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        } catch (const JSON::exception& e) {
            set_enverr(ENV_UNKNOWN);
            if (verbose) std::cerr << "Warning: unknown error while parsing environment variable '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::uint32_t getError() {
        return get_enverr();
    }

    void clearError() {
        clear_enverr();
    }
}