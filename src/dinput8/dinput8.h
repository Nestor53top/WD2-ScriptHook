#pragma once
#ifndef DINPUT8_H
#define DINPUT8_H

#include <windows.h>
#include <dinput.h>
#include <stdio.h>

#define DINPUT8_LOG_FILE "dinput8.log"

// Real DInput8 function pointer types
typedef HRESULT (WINAPI* DirectInput8Create_t)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
typedef HRESULT (WINAPI* DllCanUnloadNow_t)(void);
typedef HRESULT (WINAPI* DllGetClassObject_t)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
typedef HRESULT (WINAPI* DllRegisterServer_t)(void);
typedef HRESULT (WINAPI* DllUnregisterServer_t)(void);

// Exported real function pointers
extern DirectInput8Create_t Real_DirectInput8Create;
extern DllCanUnloadNow_t Real_DllCanUnloadNow;
extern DllGetClassObject_t Real_DllGetClassObject;
extern DllRegisterServer_t Real_DllRegisterServer;
extern DllUnregisterServer_t Real_DllUnregisterServer;

// Forwarded exports (ordinals)
HRESULT WINAPI Proxy_DirectInput8CreateA(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
HRESULT WINAPI Proxy_DllCanUnloadNow(void);
HRESULT WINAPI Proxy_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
HRESULT WINAPI Proxy_DllRegisterServer(void);
HRESULT WINAPI Proxy_DllUnregisterServer(void);

// Utility
void DInput8Log(const char* fmt, ...);
BOOL DInput8LoadRealDll(void);
DWORD WINAPI DInput8InitThread(LPVOID param);

#endif // DINPUT8_H
