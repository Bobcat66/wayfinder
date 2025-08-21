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

# TODO: Right now this is sort of a god class, should modularize and refactor in the future
# TODO: More robust input validation

# TODO: OPTIONS API has changed to be RFC-compliant, update wfctl client
# TODO: Drop JSON Pointer query support, re-implement on client side with JSON fragments
from http.client import HTTPConnection, RemoteDisconnected, HTTPResponse, _DataType, _HeaderValue
import socket
from typing import Union, Mapping, Tuple, List, Dict, Callable, Any, Set
from pathlib import Path
import sys
import re
import json
from dataclasses import dataclass, asdict
import jsonpatch
import jsonpointer
from deepdiff import DeepDiff
from deepdiff.model import DiffLevel

varpattern = re.compile(r"^\$(\d+)$")

agentname: str = "wfcli/1.0"

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
bad_header: int = 8
# 0 = normal
# 1 = Connection failed
# 2 = Bad HTTP status
# 3 = Filesystem error

# TODO: Maybe switch from sys.exit to exceptions

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
    "exist": 2,
    "jtest": 2,
    "jtestf": 3,
    "test": 1,
    "testf": 2,
    "diff": 0,
    "commit": 0,
    "abort": 0,
    "exec": 1,
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

# Deferable commands:
# push
# jp
# delete
@dataclass
class deferredCmd:
    cmdword: str
    args: List[str]
    patch: bool = False

@dataclass
class ResourceCaps:
    supportedMethods: Set[str]
    specialCases: Dict[str,Set[str]]

@dataclass
class BatchRequest:
    method: str
    url: str
    body: str
    headers: Dict[str,str]

class Session:
    def __init__(self, hostname: str, port: int, quiet: bool, keepgoing: bool, globargs: List[str]):
        self.quiet = quiet
        self.printout(f"Connecting to {hostname}:{port}")
        self.conn = HTTPConnection(hostname,port)
        self.transaction = False
        self.hostname = hostname
        self.port = port
        self.globargs = globargs
        self.keepgoing = keepgoing # Determines whether or not the session should terminate on an error
        # Top level collection capabilities
        self.topLevelCapabilities: Dict[str,ResourceCaps] = {}
        # Resource capabilities
        self.resourceCapabilities: Dict[str,Set[str]] = {}
        # Generic differences
        self.diffs: Dict[str,diff] = {} # dictionary of resources, each one mapped to an object keeping track of transaction differences
        self.commands: Dict[str,Callable[[List[str]],Tuple[int,Union[HTTPResponse,None]]]] = {
            "quit": self.quit,
            "fetch": self.fetch,
            "push": self.push,
            "pushf": self.pushf,
            "delete": self.delete,
            "transact": self.transact,
            "jp": self.jp,
            "jpf": self.jpf,
            "exist": self.exist,
            "jtest": self.jtest,
            "jtestf": self.jtestf,
            "diff": self.diff,
            "commit": self.commit,
            "abort": self.abort,
            "exec": self.exec,
            "summary": self.summary,
            "start": self.start,
            "stop": self.stop,
            "shutdown": self.shutdown,
            "reload": self.reload,
            "reboot": self.reboot,
            "restart": self.restart
        }
        self.transactCommands: List[deferredCmd] = []
        connerr = self.checkConnection()
        if connerr == nominal:
            error, devname = self.getDevname()
            if error:
                sys.exit(error)
            self.printout(f"Connected to {devname} @ {hostname}:{port}")
        else:
            sys.exit(connerr)
    
    def checkConnection(self) -> int:
        try:
            self.conn.request("HEAD", "/",headers={"X-Clacks-Overhead": "GNU Terry Pratchett","User-Agent": agentname})
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
        if err != nominal:
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
        headers["User-Agent"] = agentname
        # The wayfinder API *always* responds in JSON
        headers["Accept"] = "application/json"
        headers["Content-Type"] = content_type
        if body is not None and not encode_chunked:
            headers["Content-Length"] = len(body)
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
        
    def parseCommand(self,command: str) -> Tuple[str,Union[None,List[str]]]:
        cmdword,argstr = self.popFirstWord(command)
        numargs = cmdWordArgs.get(cmdword)
        if numargs is None:
            printerr(f"Error: '{cmdword}' is not a recognized wfctl command")
            return cmdword,None
        args: List[str] = []
        for i in range(numargs):
            # Process positional arguments
            if argstr == "":
                printerr(f"Error: not enough positional args")
                return cmdword,None
            arg, argstr = self.popFirstWord(argstr)
            args.append(self.resolveVar(arg))
        args.append(self.resolveVar(argstr)) # Process body
        return cmdword,args
    
    def processCommand(self,command: str) -> int:
        cmdword,args = self.parseCommand(command)
        res, request = self.commands[cmdword](args)
        # TODO: Figure out if we want to do anything with the request
        if res != nominal and not self.keepgoing:
            sys.exit(res)
        return res
        

    
    # Returns the first word, and the string with the first word popped
    def popFirstWord(self,command: str) -> Tuple[str,str]:
        word : str = ""
        stripCommand = command.strip()
        if not stripCommand:
            return "", ""
        while stripCommand and not stripCommand[0].isspace():
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
        if caperr != nominal:
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
            return self.request("PUT",resource,body)

    
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
            if err == bad_status and response is not None and response.status == 404 and allow404:
                self.diffs[resource] = diff(None,staged)
                return nominal
            elif err != nominal:
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
        if caperr != nominal:
            # error while getting capabilities, give up
            return caperr,None
        if "DELETE" not in caps:
            printerr(f"delete is forbidden for '{resource}'")
            return bad_command,None
        
        if self.transaction:
            # Cache the resource if it isn't already cached
            if resource not in self.diffs:
                cache_err = self._cacheResourceDiff(resource,staged=None,allow404=False)
                if cache_err == nominal:
                    self.transactCommands.append(deferredCmd("delete",args))
                return cache_err, None # Diffs are already cached, no need to do any more processing
            # Resource is cached in diffs
            self.diffs[resource].staged = None
            self.transactCommands.append(deferredCmd("delete",args))
        else:
            return self.request("DELETE",resource)
        
    def transact(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if self.transaction:
            printerr("Warning: Already in a transaction")
        self.transaction = True
        return nominal, None
    
    # TODO: Maybe make my own RFC 6902 implementation eventually, and rely only on the stdlib
    def _jp_impl(self,args: List[str],*,cmdword: str="jp") -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        op = args[1]
        json_pointer = args[2]
        body = args[3]
        caperr,caps = self.getCapabilities(resource)
        if caperr != nominal:
            # error while getting capabilities, give up
            return caperr,None
        if "PATCH" not in caps:
            printerr(f"{cmdword} is forbidden for '{resource}'")
            return bad_command,None
        if self.transaction:
            if resource not in self.diffs:
                cache_err = self._cacheResourceDiff(resource,staged=None,allow404=False)
                if cache_err != nominal:
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
                return self.request("PATCH",resource,json.dumps(patch_jobject),content_type="application/json-patch+json")
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
                return self._jp_impl([resource,op,json_pointer,content],cmdword="jpf")
        except (OSError) as e:
            printerr(f"Error while opening file '{filepath}': {e}")
            return bad_file, None
    
    def exist(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        json_pointer = args[1]
        hres,hresponse = self.request("HEAD",f"{resource}?ptr={json_pointer}",suppress_status=True)
        if hres == bad_connection:
            return bad_connection,None
        if hres == nominal:
            println("200 OK")
            return nominal,hresponse
        # Filter out expected bad statuses (404,422), from the actually bad statuses
        match hresponse.status:
            case 404:
                println("404 Not Found")
                return nominal,hresponse
            case 422:
                println("422 Failed")
                return nominal,hresponse
            case _:
                printBadResponse(hresponse)
                return bad_status,hresponse
    
    def _jtest_impl(self,args: List[str],*,cmdword="jtest") -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        json_pointer = args[1]
        test_value = args[2]
        caperr,caps = self.getCapabilities(resource)
        if caperr != nominal:
            # error while getting capabilities, give up
            return caperr,None
        if "PATCH" not in caps:
            printerr(f"{cmdword} is forbidden for '{resource}'")
            return bad_command,None
        
        patch_jobject: List[Dict]
        try:
            patch_jobject = [{"op": "test", "path": json_pointer, "value": json.loads(test_value)}]
        except json.JSONDecodeError as e:
            # JSON parse error, give up
            printerr(f"JSON Error: {e}")
            return bad_json, None
        res,response = self.request("PATCH",resource,json.dumps(patch_jobject),suppress_status=True,content_type="application/json-patch+json")
        if res == nominal:
            println("200 OK")
            return nominal,response
        elif res == bad_status and response.status == 422:
            println("422 Failed")
            return nominal,response
        elif res == bad_status and response.status == 404:
            println("404 Not Found")
            return nominal,response
        else:
            if response:
                printBadResponse(response)
            return res,response
    
    def jtest(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        return self._jtest_impl(args,cmdword="jtest")
    
    def jtestf(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        json_pointer = args[1]
        filepath = Path(args[2])
        if not filepath.exists():
            printerr(f"File '{filepath}' does not exist")
            return bad_file, None
        if not filepath.is_file():
            printerr(f"Path '{filepath}' is not a file")
            return bad_file, None
        try:
            with open(filepath,'r') as f:
                content = f.read()
                return self._jtest_impl([resource,json_pointer,content],cmdword="jtestf")
        except (OSError) as e:
            printerr(f"Error while opening file '{filepath}': {e}")
            return bad_file, None

    def _test_impl(self,args: List[str],*,cmdword="test") -> Tuple[int,Union[HTTPResponse,None]]:
        resource = args[0]
        body = args[1]
        caperr,caps = self.getCapabilities(resource)
        if caperr != nominal:
            # error while getting capabilities, give up
            return caperr,None
        if "GET" not in caps:
            printerr(f"{cmdword} is forbidden for '{resource}'")
            return bad_command,None
        err,response = self.request("GET",resource,suppress_status=True)
        if err is not nominal:
            if response and response.status == 404:
                # Resource not found, nominal
                println("404 Not Found")
                return nominal,response
            elif response:
                # Bad response
                printBadResponse(response)
                return err,response
            else:
                # No response
                return err, None
        # Response was nominal

        test_jobject: Any
        resource_jobject: Any
        try:
            resource_jobject = json.loads(response.read().decode('utf-8'))
        except json.JSONDecodeError as e:
            printerr(f"Server responded with malformed JSON: {e}")
            return bad_json,None
        
        try:
            test_jobject = json.loads(body)
        except json.JSONDecodeError as e:
            printerr(f"Body contained malformed JSON: {e}")
            return bad_json,None
        
        if test_jobject == resource_jobject:
            println("200 OK")
            return nominal, response
        else:
            println("422 Failed")
            return nominal, response
    
    def test(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        return self._test_impl(args,cmdword="test")
    
    def testf(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
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
                return self._test_impl([resource,content],cmdword="testf")
        except (OSError) as e:
            printerr(f"Error while opening file '{filepath}': {e}")
            return bad_file, None
    
    # TODO: Pretty printing
    def diff(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if not self.transaction:
            return nominal, None
        if len(self.diffs.items()) > 0:
            println("Staged Diffs:")
        for diffedResource in self.diffs.items():
            resource = diffedResource[0]
            diffs = diffedResource[1]
            deltas = DeepDiff(diffs.orig,diffs.staged,view='tree')
            if deltas:
                println(f"{resource}:")
                if 'type_changes' in deltas:
                    type_changes: List[DiffLevel] = deltas['type_changes']
                    for delta in type_changes:
                        path = delta.path(output_format="list")
                        ptr = '/' + '/'.join(path)
                        println(f"  {ptr}: {delta.t1} -> {delta.t2}")
                if 'value_changes' in deltas:
                    value_changes: List[DiffLevel] = deltas['value_changes']
                    for delta in value_changes:
                        path=delta.path(output_format="list")
                        ptr = '/' + '/'.join(path)
                        if delta.t1 is None:
                            println(f"  [NEW] {ptr}: {delta.t2}")
                        elif delta.t2 is None:
                            println(f"  [DELETE] {ptr}: {delta.t1}")
                        else:
                            println(f"  {ptr}: {delta.t1} -> {delta.t2}")
                if 'dictionary_item_added' in deltas:
                    dictionary_item_added: List[DiffLevel] = deltas["dictionary_item_added"]
                    for delta in dictionary_item_added:
                        path = delta.path(output_format="list")
                        ptr = '/' + '/'.join(path)
                        println(f"  [NEW] {ptr}: {delta.t2}")
                if 'dictionary_item_removed' in deltas:
                    dictionary_item_removed: List[DiffLevel] = deltas["dictionary_item_removed"]
                    for delta in dictionary_item_removed:
                        path = delta.path(output_format="list")
                        ptr = '/' + '/'.join(path)
                        println(f"  [DELETE] {ptr}: {delta.t1}")
                if 'iterable_item_added' in deltas:
                    iterable_item_added: List[DiffLevel] = deltas["iterable_item_added"]
                    for delta in iterable_item_added:
                        path = delta.path(output_format="list")
                        ptr = '/' + '/'.join(path)
                        println(f"  [NEW] {ptr}: {delta.t2}")
                if 'iterable_item_removed' in deltas:
                    iterable_item_removed: List[DiffLevel] = deltas["iterable_item_removed"]
                    for delta in iterable_item_removed:
                        path = delta.path(output_format="list")
                        ptr = '/' + '/'.join(path)
                        println(f"  [DELETE] {ptr}: {delta.t1}")
        return nominal,None
    
    def commit(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if not self.transaction:
            return nominal,None
        self.transaction = False
        batch_requests: List[BatchRequest] = []
        # These two variables are for batching consecutive patches to the same resource
        makingPatch: bool = False
        previousPatchResource: str
        patchlist: List[Dict[str,Any]] = []
        # This will batch all of the patches in patchlist into a single HTTP request, then add
        # the entire request to the request batch, and it will clear the patchlist
        # TODO: Maybe add JSON error handling???
        def _flush_patches() -> None:
            nonlocal patchlist
            nonlocal batch_requests
            nonlocal previousPatchResource
            nonlocal self
            body = json.dumps(patchlist)
            batch_requests.append(
                BatchRequest(
                    method="PATCH",
                    url='/api/' + previousPatchResource,
                    body=body,
                    headers={
                        "Content-Type": "application/json-patch+json",
                        "Accept": "application/json",
                        "User-Agent": agentname,
                        "Host": f"{self.conn.host}:{self.conn.port}",
                        "Content-Length": f"{len(body)}"
                    }
                )
            )
            patchlist = []
            previousPatchResource = ""
            return     

        for cmd in self.transactCommands:
            if cmd.patch:
                makingPatch = True
                resource = cmd.args[0]
                op = cmd.args[1]
                json_pointer = cmd.args[2]
                body = cmd.args[3]
                if resource == previousPatchResource:
                    patchlist.append({"op": op, "path": json_pointer, "value": json.loads(body)})
                    continue
                else:
                    # This patch is directed to a different resource than the last one,
                    # Or this is the first patch in a sequence, flush patch batch
                    if len(patchlist) > 0:
                        _flush_patches()
                    previousPatchResource = resource
                    patchlist.append({"op": op, "path": json_pointer, "value": json.loads(body)})
                    continue
            # Command is not a patch
            if makingPatch:
                # Just ended a series of consecutive patches, flush patches
                if len(patchlist) > 0:
                    _flush_patches()
                previousPatchResource = ""
                makingPatch = False
            match cmd.cmdword:
                case "push":
                    resource = cmd.args[0]
                    body = cmd.args[1]
                    request = BatchRequest(
                        method="PUT",
                        url='/api/' + resource,
                        body=body,
                        headers={
                            "Content-Type": "application/json",
                            "Accept": "application/json",
                            "User-Agent": agentname,
                            "Host": f"{self.conn.host}:{self.conn.port}",
                            "Content-Length": f"{len(body)}"
                        }
                    )
                    batch_requests.append(request)
                    continue
                case "delete":
                    resource = cmd.args[0]
                    request = BatchRequest(
                        method="DELETE",
                        url='/api/' + resource,
                        headers={
                            "Content-Type": "application/json",
                            "Accept": "application/json",
                            "User-Agent": agentname,
                            "Host": f"{self.conn.host}:{self.conn.port}"
                        }
                    )
                    batch_requests.append(request)
                    continue
        # All transaction commands have been processed into BatchRequests, build the final batch request
        self.transactCommands = []
        self.diffs = {}
        batched_requests_jobject: List[str] = [asdict(req) for req in batch_requests]
        batch_body = json.dumps(batched_requests_jobject)
        return self.request("POST","batch",batch_body)
    
    def abort(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if not self.transaction:
            return nominal,None
        self.transaction = False
        self.transactCommands = []
        self.diffs = {}
        return nominal,None
    
    def summary(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        # TBA
        return nominal, None
    
    def exec(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if self.transaction:
            printerr("ERROR: exec is disabled in transactions")
            return bad_command, None
        filepath = Path(args[0])
        if not filepath.exists():
            printerr(f"File '{filepath}' does not exist")
            return bad_file, None
        if not filepath.is_file():
            printerr(f"Path '{filepath}' is not a file")
            return bad_file, None
        try:
            with open(filepath,'r') as f:
                # TODO: error propagation from the child process
                child = Session(self.hostname,self.port,self.quiet,self.keepgoing,self.globargs)
                for line in f:
                    child.processCommand(line.rstrip())
            return nominal, None
        except (OSError) as e:
            printerr(f"Error while opening file '{filepath}': {e}")
            return bad_file, None
        
    
    def start(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if self.transaction:
            printerr("ERROR: start is disabled in transactions")
            return bad_command, None
        pipeline_name = args[0]
        body = json.dumps({"pipeline": pipeline_name,"active": True})
        return self.request("POST","live/pipelines/running",body)
    
    def stop(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if self.transaction:
            printerr("ERROR: stop is disabled in transactions")
            return bad_command, None
        pipeline_name = args[0]
        body = json.dumps({"pipeline": pipeline_name,"active": False})
        return self.request("POST","live/pipelines/running",body)
    
    def reload(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if self.transaction:
            printerr("ERROR: stop is disabled in transactions")
            return bad_command, None
        return self.request("POST","actions/reload")
    
    def restart(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if self.transaction:
            printerr("ERROR: stop is disabled in transactions")
            return bad_command, None
        return self.request("POST","actions/restart")
    
    def reboot(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if self.transaction:
            printerr("ERROR: stop is disabled in transactions")
            return bad_command, None
        return self.request("POST","actions/reboot")
    
    def shutdown(self,args: List[str]) -> Tuple[int,Union[HTTPResponse,None]]:
        if self.transaction:
            printerr("ERROR: stop is disabled in transactions")
            return bad_command, None
        return self.request("POST","actions/shutdown")
 
    def getCapabilities(self,resource: str) -> Tuple[int,Union[Set[str],None]]:
        if resource not in self.resourceCapabilities:
            # Resource capabilities not already cached, get them
            err,response = self.request("OPTIONS",resource,suppress_status=True)
            if err != nominal:
                # Request failed, give up
                if response is not None:
                    printBadResponse(response)
                return err, None
            # Good response, proceed
            allowheader = response.getheader("Allow")
            if allowheader is None:
                return bad_header, None
            options: List[str] = [m.strip() for m in allowheader.split(",")]
            self.resourceCapabilities[resource] = set(options)
        # Resource capabilities are guaranteed to be cached at this point
        return nominal, self.resourceCapabilities[resource]

            