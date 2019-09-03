# ZzRotPortal
Simple RAT with LL keylogger

It spawns a process and posts all of STDOUT over HTTPS and pipes HTTPS responses into STDIN. 
It also starts a keylogger. Keylogger will save all English language characters. 
Upon detecting Ctrl-V, keylogger will record the contents of the clipboard. 

Need a running HTTP server and a DLL injector. 

Configure the following options in pch.h 

LISTENER_IP: the IP address of your HTTP server. 

POLL_RATE: how often to connect back after losing connection

CHILD_NAME: the process that the agent should spawn (powershell.exe or cmd.exe)

FILE_NAME: The filename to save the keylogger data

To use: 

Stand up an HTTPS server that will print POST requests to the user and respond to GET requests with user input. 
Compile and inject the DLL into some process for which you have appropriate privileges. 

Currently works on Windows 10 and is undetected by Windows defender or any AV (afaik). 
