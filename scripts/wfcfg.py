#!/usr/bin/env python3

# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2025 Jesse Kane
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# This is a simple command-line utility that exposes low-level access to Wayfinder's internal configuration
# This is only for debugging and testing

# 0 = exited successfully
# 1 = WF_ENV_PATH not set

# TODO: Better error handling

import argparse
import os
import re
from pathlib import Path
import sys
import subprocess
from typing import Union

def load_env_file(path):
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            key, _, value = line.partition("=")
            value = value.strip()
            if (value.startswith('"') and value.endswith('"')) or \
               (value.startswith("'") and value.endswith("'")):
                value = value[1:-1]  # remove surrounding quotes
            os.environ[key] = value

# Returns number of lines modified
def sed_s(filepath: Path, pattern: str, replacement: str) -> int:
    with open(filepath, 'r') as f:
        lines = f.readlines()

    lines_modified: int = 0
    with open(filepath, 'w') as f:
        for line in lines:
            new_line = re.sub(pattern, replacement, line)
            if (new_line != line):
                lines_modified += 1
            f.write(new_line)
        return lines_modified

# Returns number of lines commented
def comment_key(filepath: Path, key: str) -> int:
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    lines_commented: int = 0
    with open(filepath,'w') as f:
        for line in lines:
            if re.search(f"^{re.escape(key)}=.*",line):
                line = "#" + line
                lines_commented += 1
            f.write(line)
    return lines_commented

# Returns number of lines uncommented
def uncomment_key(filepath: Path, key: str) -> int:
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    lines_commented: int = 0
    with open(filepath,'w') as f:
        for line in lines:
            if re.search(f"^#{re.escape(key)}=.*",line):
                line = line[1:]
                lines_commented += 1
            f.write(line)
    return lines_commented

# Returns number of lines printed
def grep(filepath: Path, pattern: str) -> int:
    with open(filepath,'r') as f:
        line = f.readline()
        counter: int = 0
        while line:
            if re.search(pattern,line):
                print(line.strip())
                counter += 1
            line = f.readline()
        return counter
    
def putenv(key: str, val: str) -> int:
    envpath = os.environ.get("WF_ENV_PATH")
    if (envpath is None):
        return 1
    res = sed_s(envpath,f"^{re.escape(key)}=.*",f"{key}=\"{val}\"")
    if (res == 0):
        with open(envpath,'a') as f:
            f.write(f"\n{key}=\"{val}\"")
    return 0

def getenv(key: str) -> int:
    envpath = os.environ.get("WF_ENV_PATH")
    if (envpath is None):
        return 1
    grep(envpath,f"^{re.escape(key)}=.*")
    return 0

def sethost() -> int:
    envpath = os.environ.get("WF_ENV_PATH")
    if (envpath is None):
        return 1
    load_env_file(envpath)
    hostname = os.environ.get("WF_DEVICE_NAME")
    print(f"Setting hostname to '{hostname}'")
    subprocess.run(["hostnamectl","set-hostname",hostname],check=True)
    return 0

# A restart is required for changes to take effect
def ipctl(ip: Union[str,None], static: bool):
    hres = sethost()
    if hres != 0:
        return hres
    networkfile = Path("/etc/systemd/network/10-eth0.network")
    if ip is not None:
        sed_s(networkfile,"Address=.*",f"Address={ip}")
    if static:
        uncomment_key(networkfile,"Address")
        comment_key(networkfile,"DHCP")
    else:
        uncomment_key(networkfile,"DHCP")
        comment_key(networkfile,"Address")
    return 0

def putenv_cli(args) -> int:
    return putenv(args.key,args.val)

def getenv_cli(args) -> int:
    return getenv(args.key)

def ipctl_cli(args) -> int:
    return ipctl(args.ip,args.static)

def sethost_cli(args) -> int:
    return sethost()

def dump_cli(args) -> int:
    envpath = os.environ.get("WF_ENV_PATH")
    if (envpath is None):
        return 1
    with open(envpath,'r') as f:
        sys.stdout.write(f.read())
    return 0

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="A lightweight Command-line utility for configuring wayfinder")
    subparsers = parser.add_subparsers(title="subcommands", dest="command")
    subparsers.required = True  # require a subcommand

    parser_putenv = subparsers.add_parser("putenv",help="Set an environment variable")
    parser_putenv.add_argument("key",help="Environment variable name")
    parser_putenv.add_argument("val",help="Environment variable value")
    parser_putenv.set_defaults(func=putenv_cli)

    parser_getenv = subparsers.add_parser("getenv",help="Get an environment variable")
    parser_getenv.add_argument("key",help="Environment variable name")
    parser_getenv.set_defaults(func=getenv_cli)

    parser_ipctl = subparsers.add_parser("ipctl",help="Configure network")
    parser_ipctl.add_argument("--ip",type=str,help="Static IP address")
    parser_ipctl.add_argument("--static","-s",action="store_true")
    parser_ipctl.set_defaults(func=ipctl_cli)

    parser_sethost = subparsers.add_parser("sethost",help="Set hostname to device name (FOR INTERNAL USE ONLY)")
    parser_sethost.set_defaults(func=sethost_cli)

    parser_devname = subparsers.add_parser("devname",help="Sets device name")
    parser_devname.add_argument("name",type=str,help="new device name")

    parser_dump = subparsers.add_parser("dump",help="Dumps wayfinder environment")
    parser_dump.set_defaults(func=dump_cli)

    args = parser.parse_args()
    sys.exit(args.func(args))




