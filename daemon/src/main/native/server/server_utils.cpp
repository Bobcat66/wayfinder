/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane, Valentina Carcassi
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

#include "server/server_utils.h"
#include <filesystem>
#include "wfcore/common/envutils.h"
#include <cstdio>
#include <sys/wait.h>

namespace wfsrv {
    std::optional<std::string> getScriptsPath() {
        namespace fs = std::filesystem;
        // for lazy one-time evaluation
        static bool loaded = false;
        static bool badpath = false;
        static std::string spath;
        if (badpath) return std::nullopt;

        if (!loaded) {
            // lazily fetch scripts path from environment
            auto spathstropt = wf::env::getVar("WF_PATH");
            if (!spathstropt) {
                badpath = true;
                return std::nullopt;
            }
            if (!fs::exists(fs::path(spathstropt.value()))) {
                badpath = true;
                return std::nullopt;
            }
            spath = spathstropt.value();
            loaded = true;
            return spath;
        }

        return spath;
    }

    std::string join(const std::vector<std::string>& vec, const std::string& sep) {
        std::ostringstream oss;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i != 0) oss << sep;
            oss << vec[i];
        }
        return oss.str();
    }

    int invoke_shell(const char* cmd, std::ostream& out_capture) {
        FILE* pipe = popen(cmd, "r");

        if (!pipe)
            return -1;

        char buffer[256];
        while (fgets(buffer,sizeof(buffer),pipe)) {
            out_capture << buffer;
        }

        int rc = pclose(pipe);
        
        if (rc == -1) return -1; // pclose() failed
        if (WIFEXITED(rc)) return WEXITSTATUS(rc);
        return -1;                         // child did not exit normally
    }
}