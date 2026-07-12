#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

lua_State *luaL_newstate(void) {
    return NULL;
}

void luaL_openlibs(lua_State *L) {
    (void)L;
}

int luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name) {
    (void)L; (void)buff; (void)sz; (void)name;
    return 1;
}

int luaL_loadstring(lua_State *L, const char *s) {
    (void)L; (void)s;
    return 1;
}

int luaL_error(lua_State *L, const char *fmt, ...) {
    (void)L; (void)fmt;
    return 0;
}

int luaL_argerror(lua_State *L, int numarg, const char *extramsg) {
    (void)L; (void)numarg; (void)extramsg;
    return 0;
}

int luaL_typerror(lua_State *L, int narg, const char *tname) {
    (void)L; (void)narg; (void)tname;
    return 0;
}

const char *luaL_checklstring(lua_State *L, int numArg, size_t *len) {
    (void)L; (void)numArg;
    if (len) *len = 0;
    return "";
}

const char *luaL_optlstring(lua_State *L, int numArg, const char *def, size_t *len) {
    (void)L; (void)numArg;
    if (len) *len = def ? strlen(def) : 0;
    return def;
}

lua_Number luaL_checknumber(lua_State *L, int numArg) {
    (void)L; (void)numArg;
    return 0.0;
}

lua_Number luaL_optnumber(lua_State *L, int numArg, lua_Number def) {
    (void)L; (void)numArg;
    return def;
}

lua_Integer luaL_checkinteger(lua_State *L, int numArg) {
    (void)L; (void)numArg;
    return 0;
}

lua_Integer luaL_optinteger(lua_State *L, int numArg, lua_Integer def) {
    (void)L; (void)numArg;
    return def;
}

void luaL_checkstack(lua_State *L, int sp, const char *msg) {
    (void)L; (void)sp; (void)msg;
}

void luaL_checktype(lua_State *L, int narg, int t) {
    (void)L; (void)narg; (void)t;
}

void luaL_checkany(lua_State *L, int narg) {
    (void)L; (void)narg;
}

int luaL_newmetatable(lua_State *L, const char *tname) {
    (void)L; (void)tname;
    return 0;
}

void *luaL_checkudata(lua_State *L, int ud, const char *tname) {
    (void)L; (void)ud; (void)tname;
    return NULL;
}

void luaL_where(lua_State *L, int lvl) {
    (void)L; (void)lvl;
}

int luaL_ref(lua_State *L, int t) {
    (void)L; (void)t;
    return LUA_NOREF;
}

void luaL_unref(lua_State *L, int t, int ref) {
    (void)L; (void)t; (void)ref;
}

void luaL_openlib(lua_State *L, const char *libname, const luaL_Reg *l, int nup) {
    (void)L; (void)libname; (void)l; (void)nup;
}

void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
    (void)L; (void)l; (void)nup;
}

int luaL_getmetafield(lua_State *L, int obj, const char *e) {
    (void)L; (void)obj; (void)e;
    return 0;
}

int luaL_callmeta(lua_State *L, int obj, const char *e) {
    (void)L; (void)obj; (void)e;
    return 0;
}

char *luaL_buffinit(lua_State *L, luaL_Buffer *B) {
    (void)L;
    B->p = B->buffer;
    B->level = 0;
    B->L = L;
    return B->buffer;
}

char *luaL_prepbuffer(luaL_Buffer *B) {
    (void)B;
    return B->buffer;
}

void luaL_addlstring(luaL_Buffer *B, const char *s, size_t l) {
    (void)B; (void)s; (void)l;
}

void luaL_addstring(luaL_Buffer *B, const char *s) {
    (void)B; (void)s;
}

void luaL_addvalue(luaL_Buffer *B) {
    (void)B;
}

void luaL_pushresult(luaL_Buffer *B) {
    (void)B;
}
