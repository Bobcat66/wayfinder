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

from http.client import HTTPConnection, RemoteDisconnected, HTTPResponse, _DataType, _HeaderValue
import socket
from typing import Union, Mapping, Tuple, List, Dict, Callable, Any, Set
from pathlib import Path
import sys
import re
import json
from dataclasses import dataclass
import jsonpatch
import jsonpointer

varpattern = re.compile(r"^\$(\d+)$")

def println(msg: str) -> None:
    print(msg,file=sys.stdout)

def printerr(msg: str) -> None:
    print(msg,file=sys.stderr)

def printBadResponse(response: HTTPResponse) -> None:
    printerr(f"FAILURE: Server responded with status {response.status}: {response.read().decode('utf-8')}")

def require(d: dict, key: str) -> Any:
    if key not in d:
        raise KeyError(f"Missing required key: '{key}'")
    return d[key]

# Status 
nominal: int = 0
bad_connection: int = 1
bad_status: int = 2
bad_file: int = 3
bad_json: int = 4
bad_command: int = 5
bad_patch: int = 6
bad_pointer: int = 7
# 0 = normal
# 1 = Connection failed
# 2 = Bad HTTP status
# 3 = Filesystem error

# Number of args expected by a command
# Excluding the implicit body arg at the end
cmdWordArgs: Dict[str,int] = {
    "quit": 0,
    "fetch": 1,
    "push": 1,
    "pushf": 2,
    "delete": 1,
    "transact": 0,
    "jp": 3,
    "jpf": 4,
    "diff": 0,
    "commit": 0,
    "abort": 0,
    "exec": 1,
    "upload": 3,
    "summary": 0,
    "start": 1,
    "stop": 1,
    "shutdown": 0,
    "reload": 0,
    "restart": 0,
    "reboot": 0
}

@dataclass
class diff:
    orig: Any
    staged: Any

@dataclass
class deferredCmd:
    cmdword: str
    args: List[str]
    patch: bool = False

@dataclass
class ResourceCaps:
    supportedMethods: Set[str]
    specialCases: Dict[str,Set[str]]

class Session:
    def __init__(self, hostname: str, port: int, quiet: bool, globargs: List[str]):
        self.quiet = quiet
        self.printout(f"Connecting to {hostname}:{port}")
        self.conn = HTTPConnection(hostname,port)
        self.transaction = False
        self.hostname = hostname
        self.port = port
        self.globargs = globargs
        # Top level collection capabilities
        self.topLevelCapabilities: Dict[str,ResourceCaps] = {}
        # Resource capabilities
        self.resourceCapabilities: Dict[str,Set[str]] = {}
        # Generic differences
        self.diffs: Dict[str,diff] = {} # dictionary of resources, each one mapped to an object keeping track of transaction differences
        self.commands: Dict[str,Callable[[List[str]],Tuple[int,Union[HTTPResponse,None]]]] = {
            "quit": self.quit
        }
        self.transactCommands: List[deferredCmd] = {}
        connerr = self.checkConnection()
        if connerr is nominal:
            error, devname = self.getDevname()
            if error:
                sys.exit(error)
            self.printout(f"Connected to {devname} @ {hostname}:{port}")
        else:
            sys.exit(connerr)
    
    def checkConnection(self) -> int:
        try:
            self.conn.request("HEAD", "/",headers={"X-Clacks-Overhead","GNU Terry Pratchett"})
            response = self.conn.getresponse()
            if 200 <= response.status < 400:
                self.printout("Connection OK")
                return nominal
            else:
                printerr(f"Server responded with status {response.status}")
                return bad_status 
        except (ConnectionRefusedError, socket.timeout, RemoteDisconnected, OSError) as e:
            printerr(f"Connection failed: {e}")
            return bad_connection
    
    def printout(self, msg: str):
        if (not self.quiet):
            println(msg)
    
    def getDevname(self) -> Tuple[int,Union[str,None]]:
        err,name_response = self.request("GET","env/devname",suppress_status=True)
        if err is not nominal:
            return err, None
        name = name_response.read().decode('utf-8')
        return nominal, name

    # Sends a request to the server
    def request(
        self,
        method: str,url: str,
        body: Union[_DataType,str,None] = None,
        headers: Mapping[str,_HeaderValue] = None,
        *,
        encode_chunked: bool = False,
        suppress_status: bool = False,
        content_type: str = "application/json"
    ) -> Tuple[int, Union[HTTPResponse,None]]:
        headers = dict(headers) if headers else {}
        headers["User-Agent"] = "wfcli/1.0"
        # The wayfinder API *always* expects and responds in JSON
        headers["Accept"] = "application/json"
        headers["Content-Type"] = content_type
        try:
            self.conn.request(method,"/api/" + url,body,headers,encode_chunked=encode_chunked)
            response = self.conn.getresponse()
            if 200 <= response.status < 400:
                if not suppress_status:
                    self.printout(f"SUCCESS: Server responded with status {response.status}")
                return nominal, response
            # Bad status
            if not suppress_status:
                printBadResponse(response)
            return bad_status, response
        except (ConnectionRefusedError, socket.timeout, RemoteDisconnected, OSError) as e:
            printerr(f"FAILURE: Connection failed: {e}")
            return bad_connection, None
    
    
    # Attempts to match a single word to a var, returns the var if it does and returns the unmodified word if it doesn't
    def resolveVar(self,word: str) -> str:
        match = varpattern.search(word)
        if match:
            varnum = int(match.group(1))
            if len(self.globargs) > varnum:
                return self.globargs[varnum]
            return ""
        return word
        
    def parseCommand(self,command: str) -> Union[None,List[str]]:
        cmdword,argstr = self.popFirstWord(command)
        numargs = cmdWordArgs.get(cmdword)
        if numargs is None:
            printerr(f"Error: '{cmdword}' is not a recognized wfcli command")
            return None
        args: List[str] = []
        for i in range(numargs):
            # Process positional arguments
            if argstr == "":
                printerr(f"Error: not enough positional args")
                return None
            arg, argstr = self.popFirstWord(argstr)
            args.append(self.resolveVar(arg))
        args.append(self.resolveVar(argstr)) # Process body
        return args

    
    # Returns the first word, and the string with the first word popped
    def popFirstWord(self,command: str) -> Tuple[str,str]:
        word : str = ""
        stripCommand = command.strip()
        if not stripCommand:
            return "", ""
        char : str = stripCommand[0]
        while (not char.isspace()) and len(stripCommand) >= 1:
            word += stripCommand[0]
            stripCommand = stripCommand[1:]
        stripCommand = stripCommand.strip()
        return word, stripCommand
    
    # quit
    def quit(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        sys.exit(nominal)
    
    # fetch [resource]
    def fetch(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        return self.request("GET",resource)
    
    # cmdword override is solely for logging, logically this always behaves like a push command
    # and assumes that the full content to be pushed is in the args already
    def _push_impl(self,args: List[str],*,cmdword="push") -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        body = args[1]
        caperr,caps = self.getCapabilities(resource)
        if caperr is not nominal:
            # error while getting capabilities, give up
            return caperr,None
        if "PUT" not in caps:
            printerr(f"{cmdword} is forbidden for '{resource}'")
            return bad_command,None
        
        if self.transaction:
            # Cache the resource if it isn't already cached
            if resource not in self.diffs:
                try:
                    self._cacheResourceDiff(resource,staged=json.loads(body))
                    self.transactCommands.append(deferredCmd("push",args)) # We do not use the cmdword override, as we want this command to behave like a push when evaluated on commit
                    return nominal,None
                except json.JSONDecodeError as e:
                    # JSON parse error, give up
                    printerr(f"JSON Error: {e}")
                    return bad_json,None
            # Resource is loaded in diff cache
            try:
                self.diffs[resource].staged = json.loads(body)
                self.transactCommands.append(deferredCmd("push",args)) # We do not use the cmdword override, as we want this command to behave like a push when evaluated on commit
                return nominal,None
            except json.JSONDecodeError as e:
                # JSON parse error, give up
                printerr(f"JSON Error: {e}")
                return bad_json,None
            
        else:
            return self.request("PUT",resource,body,{"Content-Length":len(body)})

    
    # push [resource] [body]. 
    def push(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        return self._push_impl(args,cmdword="push")
    
    def pushf(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        filepath = Path(args[1])
        if not filepath.exists():
            printerr(f"File '{filepath}' does not exist")
            return bad_file, None
        if not filepath.is_file():
            printerr(f"Path '{filepath}' is not a file")
            return bad_file, None
        try:
            with open(filepath,'r') as f:
                content = f.read()
                return self._push_impl([resource,content],cmdword="pushf")
        except (OSError) as e:
            printerr(f"Error while opening file '{filepath}': {e}")
            return bad_file, None
    
    # allow404 determines whether or not the resource can be created if it isn't found
    def _cacheResourceDiff(self,resource: str,*,staged: Any=None,allow404: bool = True) -> int:
            err,response = self.request("GET",resource,suppress_status=True)
            if err is bad_status and response is not None and response.status is 404 and allow404:
                self.diffs[resource] = diff(None,staged)
                return nominal
            elif err is not nominal:
                # Got some other error from the server, give up and propagate return code
                if response is not None:
                    printBadResponse(response)
                return err
            else:
                try:
                    self.diffs[resource] = diff(json.loads(response.read().decode('utf-8')),staged)
                    return nominal
                except json.JSONDecodeError as e:
                    # JSON parse error, give up
                    printerr(f"JSON Error: {e}")
                    return bad_json

    def delete(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        caperr,caps = self.getCapabilities(resource)
        if caperr is not nominal:
            # error while getting capabilities, give up
            return caperr,None
        if "DELETE" not in caps:
            printerr(f"delete is forbidden for '{resource}'")
            return bad_command,None
        
        if self.transaction:
            # Cache the resource if it isn't already cached
            if resource not in self.diffs:
                cache_err = self._cacheResourceDiff(resource,staged=None,allow404=False)
                if cache_err is nominal:
                    self.transactCommands.append(deferredCmd("delete",args))
                return cache_err, None # Diffs are already cached, no need to do any more processing
            # Resource is cached in diffs
            self.diffs[resource].staged = None
            self.transactCommands.append(deferredCmd("delete",args))
        else:
            return self.request("DELETE",resource)
    
    # TODO: Maybe make my own RFC 6902 implementation eventually, and rely only on the stdlib
    def _jp_impl(self,args: List[str],*,cmdword: str="jp") -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        op = args[1]
        json_pointer = args[2]
        body = args[3]
        caperr,caps = self.getCapabilities(resource)
        if caperr is not nominal:
            # error while getting capabilities, give up
            return caperr,None
        if "PATCH" not in caps:
            printerr(f"{cmdword} is forbidden for '{resource}'")
            return bad_command,None
        if self.transaction:
            if resource not in self.diffs:
                cache_err = self._cacheResourceDiff(resource,staged=None,allow404=False)
                if cache_err is not nominal:
                    return cache_err, None
                self.diffs[resource].staged = self.diffs[resource].orig
            # resource should be cached now, generate JSON patch
            if op not in ["add","remove","replace"]:
                printerr(f"{op} is not a supported JSON Patch operation")
                return bad_command,None
            try:
                patch_object = jsonpatch.JsonPatch([{"op": op, "path": json_pointer, "value": json.loads(body)}])
                self.diffs[resource].staged = patch_object.apply(self.diffs[resource].staged)
                self.transactCommands.append(deferredCmd("jp",args,patch=True))
                return nominal,None
            # Failure modes
            except jsonpatch.JsonPatchConflict as e:
                printerr(f"JSON Patch Conflict detected: {e}")
                return bad_patch,None
            except jsonpointer.JsonPointerException as e:
                printerr(f"JSON Pointer Exception: {e}")
                return bad_pointer,None
            except json.JSONDecodeError as e:
                # JSON parse error, give up
                printerr(f"JSON Error: {e}")
                return bad_json,None

        else:
            # Not in transaction, apply operation immediately
            try:
                patch_jobject = [{"op": op, "path": json_pointer, "value": json.loads(body)}]
                return self.request("PATCH",resource,json.dumps(patch_jobject))
            except json.JSONDecodeError as e:
                # JSON parse error, give up
                printerr(f"JSON Error: {e}")
                return bad_json, None

    
    def jp(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        return self._jp_impl(args,cmdword="jp")
    
    def jpf(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        op = args[1]
        json_pointer = args[2]
        filepath = Path(args[3])
        if not filepath.exists():
            printerr(f"File '{filepath}' does not exist")
            return bad_file, None
        if not filepath.is_file():
            printerr(f"Path '{filepath}' is not a file")
            return bad_file, None
        try:
            with open(filepath,'r') as f:
                content = f.read()
                return self._push_impl([resource,op,json_pointer,content],cmdword="jpf")
        except (OSError) as e:
            printerr(f"Error while opening file '{filepath}': {e}")
            return bad_file, None

        
    
    def getCapabilities(self,resource: str) -> Tuple[int,Union[Set[str],None]]:
        if resource not in self.resourceCapabilities:
            # Resource capabilities not already cached, get them
            topLevel = resource.split("/")[0]
            if topLevel not in self.topLevelCapabilities:
                # top level capabilities not cached, request them
                err,response = self.request("OPTIONS",topLevel,suppress_status=True)
                if err is not nominal:
                    # Request failed, give up
                    if response is not None:
                        printBadResponse(response)
                    return err, None
                try:
                    # Good response, proceed
                    # Process top level capabilities
                    options: Dict = json.loads(response.read().decode('utf-8'))
                    if not isinstance(options,dict):
                        raise TypeError("Response is not a JSON Object")

                    tlcaps = ResourceCaps(set(require(options,"supportedMethods")),{})
                    # Process special cases
                    for specialCase in options.get("specialCase") or []:
                        if not isinstance(specialCase,dict):
                            raise TypeError("Special Case is not a JSON object")
                        tlcaps.specialCases[require(specialCase,"url")] = set(require(specialCase,"supportedMethods"))

                    # Cache result
                    self.topLevelCapabilities[topLevel] = tlcaps
                
                # Failure modes
                except json.JSONDecodeError as e:
                    printerr(f"JSON Error: {e}")
                    return bad_json, None
                except KeyError as e:
                    printerr(f"JSON Schema violation: {e}")
                    return bad_json, None
                except TypeError as e:
                    printerr(f"JSON Schema violation: {e}")
                    return bad_json, None
            # Top level capabilities are guaranteed to be cached now
            caps = self.topLevelCapabilities[topLevel]
            methods: Set[str]
            if resource in caps.specialCases:
                methods = caps.specialCases[resource]
            else:
                # Resource is not a special case, default to top level capabilities
                methods = caps.supportedMethods
            
            # Cache result
            self.resourceCapabilities[resource] = methods
        # Resource capabilities are guaranteed to be cached at this point
        return nominal, self.resourceCapabilities[resource]

            