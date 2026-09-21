#pragma once
typedef unsigned long DWORD; typedef void* HWND;
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_SPACE 0x20
HWND GetForegroundWindow(); DWORD GetWindowThreadProcessId(HWND, DWORD*); DWORD GetCurrentProcessId(); short GetAsyncKeyState(int);
