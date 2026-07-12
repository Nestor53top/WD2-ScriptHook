#pragma once

#include "lua.h"

#define LUA_COMPAT_ALL

/* Error codes */
#define LUA_ERRFILE (LUA_ERRERR+1)

/* extra pseudo-file for luaL_ref */
#define LUA_SIGNATURE "\27Lua"

/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE "_LOADED"

/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE "_PRELOAD"

typedef struct luaL_Reg {
    const char *name;
    lua_CFunction func;
} luaL_Reg;

#define luaL_checkstring(L,n)	luaL_checklstring(L, n, NULL)
#define luaL_optstring(L,n,d)	luaL_optlstring(L, n, d, NULL)
#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))
#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#define luaL_typerror(L,n,t) luaL_error(L, "bad argument #%d to %s (%s expected, got %s)", \
			(int)(n), "function", (t), luaL_typename(L,(n)))

#define luaL_argcheck(L,cond,narg,exp) \
		((void)((cond) || luaL_argerror(L, (narg), (exp))))
#define luaL_checkstring(L,n)	luaL_checklstring(L, (n), NULL)
#define luaL_optstring(L,n,d)	luaL_optlstring(L, (n), (d), NULL)

#define luaL_getmetatable(L,n) (lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_newlib(L,l) \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_newlibtable(L,l) \
	lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

LUALIB_API lua_State *luaL_newstate(void);
LUALIB_API void luaL_close(lua_State *L);

LUALIB_API void luaL_openlibs(lua_State *L);

LUALIB_API int luaL_getmetafield(lua_State *L, int obj, const char *e);
LUALIB_API int luaL_callmeta(lua_State *L, int obj, const char *e);
LUALIB_API int luaL_argerror(lua_State *L, int narg, const char *extramsg);
LUALIB_API int luaL_argerror(lua_State *L, int narg, const char *extramsg);
LUALIB_API const char *luaL_checklstring(lua_State *L, int numArg, size_t *l);
LUALIB_API const char *luaL_optlstring(lua_State *L, int numArg, const char *def, size_t *l);
LUALIB_API lua_Number luaL_checknumber(lua_State *L, int numArg);
LUALIB_API lua_Number luaL_optnumber(lua_State *L, int numArg, lua_Number def);

LUALIB_API lua_Integer luaL_checkinteger(lua_State *L, int numArg);
LUALIB_API lua_Integer luaL_optinteger(lua_State *L, int numArg, lua_Integer def);

LUALIB_API void luaL_checkstack(lua_State *L, int sp, const char *msg);
LUALIB_API void luaL_checktype(lua_State *L, int narg, int t);
LUALIB_API void luaL_checkany(lua_State *L, int narg);

LUALIB_API int   luaL_newmetatable(lua_State *L, const char *tname);
LUALIB_API void *luaL_checkudata(lua_State *L, int ud, const char *tname);

LUALIB_API void luaL_where(lua_State *L, int level);
LUALIB_API int luaL_error(lua_State *L, const char *fmt, ...);

LUALIB_API int luaL_ref(lua_State *L, int t);
LUALIB_API void luaL_unref(lua_State *L, int t, int ref);

LUALIB_API int luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name);
LUALIB_API int luaL_loadbufferx(lua_State *L, const char *buff, size_t sz, const char *name, const char *mode);
LUALIB_API int luaL_loadfile(lua_State *L, const char *filename);
LUALIB_API int luaL_loadfilex(lua_State *L, const char *filename, const char *mode);

LUALIB_API void luaL_traceback(lua_State *L, lua_State *L1, const char *msg, int level);

LUALIB_API void *luaL_testudata(lua_State *L, int ud, const char *tname);
LUALIB_API void *luaL_checkudata(lua_State *L, int ud, const char *tname);

LUALIB_API void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);
LUALIB_API void luaL_register(lua_State *L, const char *libname, const luaL_Reg *l);

#define luaL_putchar(L,c)	lua_pushcfunction((L), ((c) ? writer_s : NULL)),  \
		lua_pushinteger((L), (c)), lua_rawseti((L), -2, 0)

#define luaL_dostring(L,s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnone(L,(n)) ? (d) : f(L,(n)))

#define luaL_check(L,n)		luaL_checkany(L, (n))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_newmetatable(L,tname) (luaL_newmetatable(L,tname))

/* pre-defined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

#define lua_newuserdata(L,s)	lua_newuserdata(L,s)

/* open/close helpers */
LUALIB_API void luaL_openlib(lua_State *L, const char *libname, const luaL_Reg *l, int nup);

#define luaL_setglobal(L,s) \
	(lua_pushstring(L, s), lua_insert(L, -2), lua_setglobal(L, s))

LUALIB_API const char *luaL_gsub(lua_State *L, const char *s, const char *p, const char *r);

/* Buffer */
#define LUAL_BUFFERSIZE 1024

typedef struct luaL_Buffer {
    char *p;
    int lvl;
    lua_State *L;
    char buffer[LUAL_BUFFERSIZE];
} luaL_Buffer;

LUALIB_API char *luaL_buffinit(lua_State *L, luaL_Buffer *B);
LUALIB_API char *luaL_prepbuffer(luaL_Buffer *B);
LUALIB_API void luaL_addlstring(luaL_Buffer *B, const char *s, size_t l);
LUALIB_API void luaL_addstring(luaL_Buffer *B, const char *s);
LUALIB_API void luaL_addvalue(luaL_Buffer *B);
LUALIB_API void luaL_pushresult(luaL_Buffer *B);

LUALIB_API int luaL_loadstring(lua_State *L, const char *s);
