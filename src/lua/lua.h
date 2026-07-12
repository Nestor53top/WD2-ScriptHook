#pragma once

#ifndef LUA_API
#ifdef LUA_BUILDING_LUA
#define LUA_API __declspec(dllexport)
#else
#define LUA_API __declspec(dllimport)
#endif
#endif

#ifndef LUALIB_API
#ifdef LUA_BUILDING_LUA
#define LUALIB_API __declspec(dllexport)
#else
#define LUALIB_API __declspec(dllimport)
#endif
#endif

#ifndef LUA_VERSION_NUM
#define LUA_VERSION_NUM 501
#define LUA_VERSION "5.1"
#define LUA_RELEASE "Lua 5.1.4"
#define LUA_AUTHORS "R. Ierusalimschy, L. H. de Figueiredo & W. Celes"
#define LUA_COPYRIGHT "Copyright (C) 1994-2012 Lua.org, PUC-Rio"
#define LUA_VERSION_MAJOR "5"
#define LUA_VERSION_MINOR "1"
#define LUA_VERSION_RELEASE "4"
#endif

#include <stddef.h>

typedef double lua_Number;
typedef long long lua_Integer;
typedef struct lua_State lua_State;
typedef int (*lua_CFunction)(lua_State *L);
typedef void *(*lua_Alloc)(void *ud, void *ptr, size_t osize, size_t nsize);
typedef const char *(*lua_Reader)(lua_State *L, void *ud, size_t *sz);
typedef int (*lua_Writer)(lua_State *L, const void *p, size_t sz, void *ud);

#define LUA_REGISTRYINDEX  (-100000 - 1000)
#define LUA_ENVIRONINDEX   (-100000 - 1001)
#define LUA_GLOBALSINDEX   (-100000 - 1002)
#define lua_upvalueindex(i) (LUA_GLOBALSINDEX-(i))

#define LUA_OK              0
#define LUA_YIELD           1
#define LUA_ERRRUN          2
#define LUA_ERRSYNTAX       3
#define LUA_ERRMEM          4
#define LUA_ERRERR          5

#define LUA_TNONE           (-1)
#define LUA_TNIL            0
#define LUA_TBOOLEAN        1
#define LUA_TLIGHTUSERDATA  2
#define LUA_TNUMBER         3
#define LUA_TSTRING         4
#define LUA_TTABLE          5
#define LUA_TFUNCTION       6
#define LUA_TUSERDATA       7
#define LUA_TTHREAD         8

#define LUA_MULTRET  (-1)

/* Lua API functions */
LUA_API lua_State *lua_newstate(lua_Alloc f, void *ud);
LUA_API void       lua_close(lua_State *L);
LUA_API lua_State *lua_newthread(lua_State *L);
LUA_API lua_CFunction lua_atpanic(lua_State *L, lua_CFunction panicf);

LUA_API int    lua_gettop(lua_State *L);
LUA_API void   lua_settop(lua_State *L, int idx);
LUA_API void   lua_pushvalue(lua_State *L, int idx);
LUA_API void   lua_remove(lua_State *L, int idx);
LUA_API void   lua_insert(lua_State *L, int idx);
LUA_API void   lua_replace(lua_State *L, int idx);
LUA_API int    lua_checkstack(lua_State *L, int extra);

LUA_API int    lua_type(lua_State *L, int idx);
LUA_API const char *lua_typename(lua_State *L, int tp);
LUA_API int    lua_isnumber(lua_State *L, int idx);
LUA_API int    lua_isstring(lua_State *L, int idx);
LUA_API int    lua_iscfunction(lua_State *L, int idx);
LUA_API int    lua_isuserdata(lua_State *L, int idx);
LUA_API int    lua_rawequal(lua_State *L, int idx1, int idx2);
LUA_API int    lua_equal(lua_State *L, int idx1, int idx2);
LUA_API int    lua_lessthan(lua_State *L, int idx1, int idx2);

LUA_API lua_Number  lua_tonumberx(lua_State *L, int idx, int *isnum);
LUA_API lua_Integer lua_tointegerx(lua_State *L, int idx, int *isnum);
LUA_API int         lua_toboolean(lua_State *L, int idx);
LUA_API const char *lua_tolstring(lua_State *L, int idx, size_t *len);
LUA_API size_t      lua_objlen(lua_State *L, int idx);
LUA_API lua_CFunction lua_tocfunction(lua_State *L, int idx);
LUA_API void       *lua_touserdata(lua_State *L, int idx);

LUA_API void   lua_pushnil(lua_State *L);
LUA_API void   lua_pushnumber(lua_State *L, lua_Number n);
LUA_API void   lua_pushinteger(lua_State *L, lua_Integer n);
LUA_API void   lua_pushboolean(lua_State *L, int b);
LUA_API void   lua_pushstring(lua_State *L, const char *s);
LUA_API void   lua_pushlstring(lua_State *L, const char *s, size_t len);
LUA_API void   lua_pushcfunction(lua_State *L, lua_CFunction fn);
LUA_API void   lua_pushlightuserdata(lua_State *L, void *p);
LUA_API int    lua_pushfstring(lua_State *L, const char *fmt, ...);
LUA_API void   lua_pushthread(lua_State *L);

LUA_API void   lua_createtable(lua_State *L, int narr, int nrec);
LUA_API void  *lua_newuserdata(lua_State *L, size_t sz);
LUA_API int    lua_getmetatable(lua_State *L, int objindex);
LUA_API int    lua_getfenv(lua_State *L, int idx);

LUA_API void   lua_settable(lua_State *L, int idx);
LUA_API void   lua_setfield(lua_State *L, int idx, const char *k);
LUA_API void   lua_rawset(lua_State *L, int idx);
LUA_API void   lua_rawseti(lua_State *L, int idx, int n);
LUA_API int    lua_setmetatable(lua_State *L, int objindex);
LUA_API int    lua_setfenv(lua_State *L, int idx);

LUA_API void   lua_gettable(lua_State *L, int idx);
LUA_API void   lua_getfield(lua_State *L, int idx, const char *k);
LUA_API void   lua_rawget(lua_State *L, int idx);
LUA_API void   lua_rawgeti(lua_State *L, int idx, int n);

LUA_API int    lua_call(lua_State *L, int nargs, int nresults);
LUA_API int    lua_pcall(lua_State *L, int nargs, int nresults, int errfunc);
LUA_API int    lua_cpcall(lua_State *L, lua_CFunction func, void *ud);
LUA_API int    lua_load(lua_State *L, lua_Reader reader, void *data, const char *chunkname);
LUA_API int    lua_next(lua_State *L, int idx);
LUA_API void   lua_concat(lua_State *L, int n);
LUA_API int    lua_yield(lua_State *L, int nresults);
LUA_API int    lua_resume(lua_State *L, int nargs);
LUA_API int    lua_status(lua_State *L);

LUA_API lua_Number  lua_tonumber(lua_State *L, int idx);
LUA_API lua_Integer lua_tointeger(lua_State *L, int idx);
LUA_API const char *lua_tostring(lua_State *L, int idx);

LUA_API lua_Number  lua_checknumber(lua_State *L, int arg);
LUA_API lua_Integer lua_checkinteger(lua_State *L, int arg);
LUA_API const char *lua_checklstring(lua_State *L, int arg, size_t *len);
LUA_API lua_Number  lua_optnumber(lua_State *L, int arg, lua_Number def);
LUA_API lua_Integer lua_optinteger(lua_State *L, int arg, lua_Integer def);

LUA_API void *luaL_checkudata(lua_State *L, int ud, const char *tname);
LUA_API int   luaL_error(lua_State *L, const char *fmt, ...);

/* Convenience macros */
#define lua_pop(L,n) lua_settop(L, -(n)-1)
#define lua_newtable(L) lua_createtable(L, 0, 0)
#define lua_getglobal(L,s) lua_getfield(L, LUA_GLOBALSINDEX, (s))
#define lua_setglobal(L,s) lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_isfunction(L,idx) (lua_type(L, (idx)) == LUA_TFUNCTION)
#define lua_istable(L,idx) (lua_type(L, (idx)) == LUA_TTABLE)
#define lua_islightuserdata(L,idx) (lua_type(L, (idx)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,idx) (lua_type(L, (idx)) == LUA_TNIL)
#define lua_isnoneornil(L, n) (lua_type(L, (n)) <= LUA_TNIL)
#define lua_register(L,n,f) (lua_pushcfunction(L, f), lua_setglobal(L, (n)))
#define lua_pushliteral(L, s) lua_pushlstring(L, s, sizeof(s)-1)
#define lua_tothread(L,idx) ((lua_State*)lua_touserdata(L, idx))

/* Registry reference constants */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

/* Hook constants */
#define LUA_MASKCALL    (1 << 0)
#define LUA_MASKRET     (1 << 1)
#define LUA_MASKLINE    (1 << 2)
#define LUA_MASKCOUNT   (1 << 3)

/* Debug structure */
typedef struct lua_Debug {
    int event;
    const char *name;
    const char *namewhat;
    const char *what;
    const char *source;
    int currentline;
    int nups;
    int linedefined;
    int lastlinedefined;
    char short_src[60];
    int istailcall;
    char short_src_padding[32]; /* extra padding for safety */
} lua_Debug;

typedef struct lua_VarInfo {
    const char *name;
    const char *namewhat;
} lua_VarInfo;

/* Debug API */
LUA_API int   lua_getstack(lua_State *L, int level, lua_Debug *ar);
LUA_API int   lua_getinfo(lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *lua_getlocal(lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_setlocal(lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *lua_getupvalue(lua_State *L, int funcindex, int n);
LUA_API const char *lua_setupvalue(lua_State *L, int funcindex, int n);
LUA_API int   lua_sethook(lua_State *L, int (*func)(lua_State *, lua_Debug *), int mask, int count);
LUA_API int   lua_gethook(lua_State *L);
LUA_API int   lua_gethookmask(lua_State *L);
LUA_API int   lua_gethookcount(lua_State *L);

/* Standard library openers */
LUALIB_API lua_State *luaL_newstate(void);
LUALIB_API void luaL_openlibs(lua_State *L);
