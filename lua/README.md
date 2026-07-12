# Lua 5.4 Source for WD2-ScriptHook

This directory should contain the official Lua 5.4.7 source files.

## Download

Download Lua 5.4.7 from: https://www.lua.org/ftp/lua-5.4.7.tar.gz

## Setup

1. Extract the archive
2. Copy all `.c` and `.h` files from the `src/` directory into this folder
3. Required files:
   - Headers: `lua.h`, `luaconf.h`, `lualib.h`, `lauxlib.h`
   - Source: `lapi.c`, `lauxlib.c`, `lbaselib.c`, `lcode.c`, `ldo.c`, `ldump.c`, `lfunc.c`, `lgc.c`, `linit.c`, `liolib.c`, `llex.c`, `llimits.h`, `lmathlib.c`, `lmem.c`, `lobject.c`, `lopcodes.c`, `loslib.c`, `lparser.c`, `lstate.c`, `lstring.c`, `lstrlib.c`, `ltable.c`, `ltablib.c`, `ltm.c`, `lundump.c`, `lvm.c`, `lzio.c`

## Build Notes

- Compile with `_CRT_SECURE_NO_WARNINGS` defined
- For MSVC: use `/MT` or `/MD` runtime
- Link against `lua54.lib` or compile as static library
