#pragma once

#ifndef LUA_API
#define LUA_API __declspec(dllimport)
#endif
#ifndef LUALIB_API
#define LUALIB_API __declspec(dllimport)
#endif
#ifndef LUAI_API
#define LUAI_API __declspec(dllimport)
#endif

#ifndef LUA_VERSION_NUM
#define LUA_VERSION_NUM 501
#define LUA_VERSION "5.1"
#define LUA_RELEASE "Lua 5.1.4"
#define LUA_AUTHORS "R. Ierusalimschy, L. H. de Figueiredo & W. Celes"
#define LUA_COPYRIGHT "Copyright (C) 1994-2012 Lua.org, PUC-Rio"
#define LUA_VERSION_MAJOR "5"
#define LUA_VERSION_MINOR "1"
#define LUA_VERSION_NUM 501
#define LUA_VERSION_RELEASE "4"
#endif

typedef double lua_Number;
typedef long long lua_Integer;
typedef struct lua_State lua_State;

#define LUA_REGISTRYINDEX (-10000)
#define LUA_ENVIRONINDEX (-10001)
#define LUA_GLOBALSINDEX (-10002)

#define LUA_OK 0
#define LUA_YIELD 1
#define LUA_ERRRUN 2
#define LUA_ERRSYNTAX 3
#define LUA_ERRMEM 4
#define LUA_ERRERR 5

#define LUA_TNONE (-1)
#define LUA_TNIL 0
#define LUA_TBOOLEAN 1
#define LUA_TLIGHTUSERDATA 2
#define LUA_TNUMBER 3
#define LUA_TSTRING 4
#define LUA_TTABLE 5
#define LUA_TFUNCTION 6
#define LUA_TUSERDATA 7
#define LUA_TTHREAD 8

#define LUA_MULTRET (-1)

#define lua_upvalueindex(i) (LUA_GLOBALSINDEX - (i))

typedef int (*lua_CFunction)(lua_State *L);
typedef void *(*lua_Alloc)(void *ud, void *ptr, size_t osize, size_t nsize);
typedef const char *(*lua_Reader)(lua_State *L, void *ud, size_t *sz);
typedef int (*lua_Writer)(lua_State *L, const void *p, size_t sz, void *ud);

#define lua_assert(exp) ((void)0)

int lua_xlua_pcall(lua_State *L, int nargs, int nresults, int errfunc);
int lua_xlua_load(lua_State *L, lua_Reader reader, void *data, const char *chunkname);
int lua_xlua_newchunk(lua_State *L, lua_Writer writer, void *data);

lua_State *lua_xlua_newstate(lua_Alloc f, void *ud);
void lua_xlua_close(lua_State *L);
lua_State *lua_xlua_newthread(lua_State *L);
lua_CFunction lua_xlua_atpanic(lua_State *L, lua_CFunction panicf);

int lua_xlua_gettop(lua_State *L);
void lua_xlua_settop(lua_State *L, int idx);
void lua_xlua_pushvalue(lua_State *L, int idx);
void lua_xlua_remove(lua_State *L, int idx);
void lua_xlua_insert(lua_State *L, int idx);
void lua_xlua_replace(lua_State *L, int idx);
int lua_xlua_checkstack(lua_State *L, int extra);

int lua_xlua_type(lua_State *L, int idx);
const char *lua_xlua_typename(lua_State *L, int tp);
int lua_xlua_isnumber(lua_State *L, int idx);
int lua_xlua_isstring(lua_State *L, int idx);
int lua_xlua_iscfunction(lua_State *L, int idx);
int lua_xlua_isuserdata(lua_State *L, int idx);

lua_Number lua_xlua_tonumberx(lua_State *L, int idx, int *isnum);
lua_Integer lua_xlua_tointegerx(lua_State *L, int idx, int *isnum);
int lua_xlua_toboolean(lua_State *L, int idx);
const char *lua_xlua_tolstring(lua_State *L, int idx, size_t *len);
size_t lua_xlua_objlen(lua_State *L, int idx);
lua_CFunction lua_xlua_tocfunction(lua_State *L, int idx);
void *lua_xlua_touserdata(lua_State *L, int idx);
int lua_xlua_rawequal(lua_State *L, int idx1, int idx2);
int lua_xlua_lessthan(lua_State *L, int idx1, int idx2);
int lua_xlua_equal(lua_State *L, int idx1, int idx2);

void lua_xlua_pushnumber(lua_State *L, lua_Number n);
void lua_xlua_pushinteger(lua_State *L, lua_Integer n);
void lua_xlua_pushstring(lua_State *L, const char *s);
void lua_xlua_pushlstring(lua_State *L, const char *s, size_t len);
void lua_xlua_pushnil(lua_State *L);
void lua_xlua_pushboolean(lua_State *L, int b);
void lua_xlua_pushcfunction(lua_State *L, lua_CFunction fn);
void lua_xlua_pushlightuserdata(lua_State *L, void *p);
void lua_xlua_pushthread(lua_State *L);
int lua_xlua_pushfstring(lua_State *L, const char *fmt, ...);

lua_Number lua_xlua_checknumber(lua_State *L, int arg);
lua_Integer lua_xlua_checkinteger(lua_State *L, int arg);
const char *lua_xlua_checklstring(lua_State *L, int arg, size_t *len);
void *lua_xlua_checkudata(lua_State *L, int ud, const char *tname);

lua_Number lua_xlua_optnumber(lua_State *L, int arg, lua_Number def);
lua_Integer lua_xlua_optinteger(lua_State *L, int arg, lua_Integer def);
const char *lua_xlua_optlstring(lua_State *L, int arg, const char *def, size_t *len);

void lua_xlua_rawget(lua_State *L, int idx);
void lua_xlua_rawgeti(lua_State *L, int idx, int n);
void lua_xlua_createtable(lua_State *L, int narr, int nrec);
void *lua_xlua_newuserdata(lua_State *L, size_t sz);
int lua_xlua_getmetatable(lua_State *L, int objindex);
int lua_xlua_getfenv(lua_State *L, int idx);
void lua_xlua_rawset(lua_State *L, int idx);
void lua_xlua_rawseti(lua_State *L, int idx, int n);
int lua_xlua_setmetatable(lua_State *L, int objindex);
int lua_xlua_setfenv(lua_State *L, int idx);

int lua_xlua_cpcall(lua_State *L, lua_CFunction func, void *ud);

int lua_xlua_next(lua_State *L, int idx);
int lua_xlua_concat(lua_State *L, int n);
int lua_xlua_yield(lua_State *L, int nresults);
int lua_xlua_resume(lua_State *L, int nargs);
int lua_xlua_status(lua_State *L);

#define lua_gettop(L) lua_xlua_gettop(L)
#define lua_settop(L, idx) lua_xlua_settop(L, idx)
#define lua_pushvalue(L, idx) lua_xlua_pushvalue(L, idx)
#define lua_remove(L, idx) lua_xlua_remove(L, idx)
#define lua_insert(L, idx) lua_xlua_insert(L, idx)
#define lua_replace(L, idx) lua_xlua_replace(L, idx)
#define lua_checkstack(L, extra) lua_xlua_checkstack(L, extra)

#define lua_type(L, idx) lua_xlua_type(L, idx)
#define lua_typename(L, tp) lua_xlua_typename(L, tp)
#define lua_isnumber(L, idx) lua_xlua_isnumber(L, idx)
#define lua_isstring(L, idx) lua_xlua_isstring(L, idx)
#define lua_iscfunction(L, idx) lua_xlua_iscfunction(L, idx)
#define lua_isuserdata(L, idx) lua_xlua_isuserdata(L, idx)

#define lua_tonumber(L, idx) lua_xlua_tonumberx(L, idx, NULL)
#define lua_tonumberx(L, idx, is) lua_xlua_tonumberx(L, idx, is)
#define lua_tointeger(L, idx) lua_xlua_tointegerx(L, idx, NULL)
#define lua_tointegerx(L, idx, is) lua_xlua_tointegerx(L, idx, is)
#define lua_toboolean(L, idx) lua_xlua_toboolean(L, idx)
#define lua_tolstring(L, idx, len) lua_xlua_tolstring(L, idx, len)
#define lua_tostring(L, idx) lua_xlua_tolstring(L, idx, NULL)
#define lua_objlen(L, idx) lua_xlua_objlen(L, idx)
#define lua_tocfunction(L, idx) lua_xlua_tocfunction(L, idx)
#define lua_touserdata(L, idx) lua_xlua_touserdata(L, idx)
#define lua_rawequal(L, idx1, idx2) lua_xlua_rawequal(L, idx1, idx2)
#define lua_lessthan(L, idx1, idx2) lua_xlua_lessthan(L, idx1, idx2)
#define lua_equal(L, idx1, idx2) lua_xlua_equal(L, idx1, idx2)

#define lua_pushnumber(L, n) lua_xlua_pushnumber(L, n)
#define lua_pushinteger(L, n) lua_xlua_pushinteger(L, n)
#define lua_pushstring(L, s) lua_xlua_pushstring(L, s)
#define lua_pushlstring(L, s, len) lua_xlua_pushlstring(L, s, len)
#define lua_pushnil(L) lua_xlua_pushnil(L)
#define lua_pushboolean(L, b) lua_xlua_pushboolean(L, b)
#define lua_pushcfunction(L, fn) lua_xlua_pushcfunction(L, fn)
#define lua_pushlightuserdata(L, p) lua_xlua_pushlightuserdata(L, p)
#define lua_pushthread(L) lua_xlua_pushthread(L)
#define lua_pushfstring(L, ...) lua_xlua_pushfstring(L, __VA_ARGS__)

#define lua_checknumber(L, arg) lua_xlua_checknumber(L, arg)
#define lua_checkinteger(L, arg) lua_xlua_checkinteger(L, arg)
#define lua_checklstring(L, arg, len) lua_xlua_checklstring(L, arg, len)
#define lua_checkstring(L, arg) lua_xlua_checklstring(L, arg, NULL)
#define lua_checkudata(L, ud, tname) lua_xlua_checkudata(L, ud, tname)

#define lua_optnumber(L, arg, def) lua_xlua_optnumber(L, arg, def)
#define lua_optinteger(L, arg, def) lua_xlua_optinteger(L, arg, def)
#define lua_optlstring(L, arg, def, len) lua_xlua_optlstring(L, arg, def, len)
#define lua_optstring(L, arg, def) lua_xlua_optlstring(L, arg, def, NULL)

#define lua_rawget(L, idx) lua_xlua_rawget(L, idx)
#define lua_rawgeti(L, idx, n) lua_xlua_rawgeti(L, idx, n)
#define lua_createtable(L, narr, nrec) lua_xlua_createtable(L, narr, nrec)
#define lua_newuserdata(L, sz) lua_xlua_newuserdata(L, sz)
#define lua_getmetatable(L, objindex) lua_xlua_getmetatable(L, objindex)
#define lua_getfenv(L, idx) lua_xlua_getfenv(L, idx)
#define lua_rawset(L, idx) lua_xlua_rawset(L, idx)
#define lua_rawseti(L, idx, n) lua_xlua_rawseti(L, idx, n)
#define lua_setmetatable(L, objindex) lua_xlua_setmetatable(L, objindex)
#define lua_setfenv(L, idx) lua_xlua_setfenv(L, idx)

#define lua_call(L, nargs, nresults) lua_xlua_call(L, nargs, nresults)
#define lua_pcall(L, nargs, nresults, errfunc) lua_xlua_pcall(L, nargs, nresults, errfunc)
#define lua_cpcall(L, func, ud) lua_xlua_cpcall(L, func, ud)
#define lua_load(L, reader, data, chunkname) lua_xlua_load(L, reader, data, chunkname)

#define lua_next(L, idx) lua_xlua_next(L, idx)
#define lua_concat(L, n) lua_xlua_concat(L, n)
#define lua_yield(L, nresults) lua_xlua_yield(L, nresults)
#define lua_resume(L, nargs) lua_xlua_resume(L, nargs)
#define lua_status(L) lua_xlua_status(L)

#define lua_newstate(f, ud) lua_xlua_newstate(f, ud)
#define lua_close(L) lua_xlua_close(L)
#define lua_newthread(L) lua_xlua_newthread(L)
#define lua_atpanic(L, f) lua_xlua_atpanic(L, f)
