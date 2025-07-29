from http.client import HTTPConnection, RemoteDisconnected, HTTPResponse, _DataType, _HeaderValue
import socket
from typing import Union, Mapping, Tuple, List, Dict
import sys
import re

varpattern = re.compile(r"^\$(\d+)$")

def println(msg: str):
    print(msg,file=sys.stdout)

def printerr(msg: str):
    print(msg,file=sys.stderr)
# Status codes
# 0 = normal
# 1 = Connection failed
# 2 = Bad HTTP status

# Number of args expected by a command
# Excluding the implicit body arg at the end
cmdWordArgs: Dict[str,int] = {
    "quit": 0,
    "post": 1,
    "postf": 2,
    "get": 1,
    "put": 1,
    "putf": 2,
    "patch": 1,
    "patchf": 2,
    "delete": 1,
    "transact": 0,
    "jp": 3,
    "jpf": 4,
    "diff": 0,
    "commit": 0,
    "abort": 0,
    "exec": 1
}

class wfcli:
    def __init__(self, hostname: str, port: int, quiet: bool, globargs: List[str]):
        self.quiet = quiet
        self.printout(f"Connecting to {hostname}:{port}")
        self.conn = HTTPConnection(hostname,port)
        self.transaction = False
        self.hostname = hostname
        self.port = port
        self.globargs = globargs
        if self.checkConnection():
            err, devname = self.getDevname()
            if (err):
                sys.exit(1)
            self.printout(f"Connected to {devname} @ {hostname}:{port}")
        else:
            sys.exit(1)
    
    def checkConnection(self) -> int:
        try:
            self.conn.request("HEAD", "/")  # HEAD requests usually have no body and are light
            response = self.conn.getresponse()
            if 200 <= response.status < 400:
                self.printout("Connection OK")
                return True
            else:
                printerr(f"Server responded with status {response.status}")
                return False 
        except (ConnectionRefusedError, socket.timeout, RemoteDisconnected, OSError) as e:
            printerr(f"Connection failed: {e}")
            return False
    
    def printout(self, msg: str):
        if (not self.quiet):
            println(msg)
    
    def getDevname(self) -> Tuple[int,Union[str,None]]:
        err,name_response = self.request("GET","/env/devname")
        if (err):
            return err, None
        name = name_response.read().decode('utf-8')
        return 0, name
    
    def request(
        self,method: str,url: str,
        body: Union[_DataType,str,None] = None,
        headers: Mapping[str,_HeaderValue] = {},
        *,encode_chunked: bool = False
    ) -> Tuple[int, Union[HTTPResponse,None]]:
        try:
            self.conn.request(method,url,body,headers,encode_chunked=encode_chunked)
            response = self.conn.getresponse()
            if 200 <= response.status < 400:
                return 0, response
            
            # Bad status
            printerr(f"Server responded with status {response.status}")
            return 2, None
        except (ConnectionRefusedError, socket.timeout, RemoteDisconnected, OSError) as e:
            printerr(f"Connection failed: {e}")
            return 1, None
    
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

    
    # Returns the first
    def popFirstWord(self,command: str) -> Tuple[str,str]:
        word : str = ""
        stripCommand = command.strip()
        if not stripCommand:
            return "", ""
        char : str = stripCommand[0]
        while (not char.isspace()) and len(stripCommand) >= 1:
            word += char
            char = stripCommand[0]
            stripCommand = stripCommand[1:]
        stripCommand = stripCommand.strip()
        return word, stripCommand
            