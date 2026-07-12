#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <string.h>

lua_State *luaL_newstate(void) { return NULL; }
void luaL_openlibs(lua_State *L) { (void)L; }

int luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name) {
    (void)L; (void)buff; (void)sz; (void)name; return 1;
}
int luaL_loadstring(lua_State *L, const char *s) {
    (void)L; (void)s; return 1;
}
int luaL_loadfilex(lua_State *L, const char *fn, const char *m) {
    (void)L; (void)fn; (void)m; return 1;
}
int luaL_error(lua_State *L, const char *fmt, ...) {
    (void)L; (void)fmt; return 0;
}
int luaL_argerror(lua_State *L, int n, const char *m) {
    (void)L; (void)n; (void)m; return 0;
}
int luaL_typerror(lua_State *L, int n, const char *t) {
    (void)L; (void)n; (void)t; return 0;
}
const char *luaL_checklstring(lua_State *L, int n, size_t *len) {
    (void)L; (void)n; if (len) *len = 0; return "";
}
const char *luaL_optlstring(lua_State *L, int n, const char *d, size_t *len) {
    (void)L; (void)n; if (len) *len = d ? strlen(d) : 0; return d;
}
lua_Number luaL_checknumber(lua_State *L, int n) { (void)L; (void)n; return 0.0; }
lua_Number luaL_optnumber(lua_State *L, int n, lua_Number d) { (void)L; (void)n; return d; }
lua_Integer luaL_checkinteger(lua_State *L, int n) { (void)L; (void)n; return 0; }
lua_Integer luaL_optinteger(lua_State *L, int n, lua_Integer d) { (void)L; (void)n; return d; }
void luaL_checkstack(lua_State *L, int sp, const char *m) { (void)L; (void)sp; (void)m; }
void luaL_checktype(lua_State *L, int n, int t) { (void)L; (void)n; (void)t; }
void luaL_checkany(lua_State *L, int n) { (void)L; (void)n; }
int luaL_newmetatable(lua_State *L, const char *t) { (void)L; (void)t; return 0; }
void *luaL_checkudata(lua_State *L, int u, const char *t) {
    (void)L; (void)u; (void)t; return NULL;
}
void luaL_where(lua_State *L, int l) { (void)L; (void)l; }
int luaL_ref(lua_State *L, int t) { (void)L; (void)t; return LUA_NOREF; }
void luaL_unref(lua_State *L, int t, int r) { (void)L; (void)t; (void)r; }
void luaL_openlib(lua_State *L, const char *ln, const luaL_Reg *l, int n) {
    (void)L; (void)ln; (void)l; (void)n;
}
void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int n) { (void)L; (void)l; (void)n; }
int luaL_getmetafield(lua_State *L, int o, const char *e) {
    (void)L; (void)o; (void)e; return 0;
}
int luaL_callmeta(lua_State *L, int o, const char *e) {
    (void)L; (void)o; (void)e; return 0;
}
char *luaL_buffinit(lua_State *L, luaL_Buffer *B) {
    (void)L; B->p = B->buffer; B->level = 0; B->L = L; return B->buffer;
}
char *luaL_prepbuffer(luaL_Buffer *B) { (void)B; return B->buffer; }
void luaL_addlstring(luaL_Buffer *B, const char *s, size_t l) { (void)B; (void)s; (void)l; }
void luaL_addstring(luaL_Buffer *B, const char *s) { (void)B; (void)s; }
void luaL_addvalue(luaL_Buffer *B) { (void)B; }
void luaL_pushresult(luaL_Buffer *B) { (void)B; }
const char *luaL_gsub(lua_State *L, const char *s, const char *p, const char *r) {
    (void)L; (void)s; (void)p; (void)r; return "";
}

/* ===== Core Lua API stubs ===== */
lua_State *lua_newstate(lua_Alloc f, void *ud) { (void)f; (void)ud; return NULL; }
void lua_close(lua_State *L) { (void)L; }
lua_State *lua_newthread(lua_State *L) { (void)L; return NULL; }
lua_CFunction lua_atpanic(lua_State *L, lua_CFunction pf) { (void)L; (void)pf; return NULL; }

int lua_gettop(lua_State *L) { (void)L; return 0; }
void lua_settop(lua_State *L, int idx) { (void)L; (void)idx; }
void lua_pushvalue(lua_State *L, int idx) { (void)L; (void)idx; }
void lua_remove(lua_State *L, int idx) { (void)L; (void)idx; }
void lua_insert(lua_State *L, int idx) { (void)L; (void)idx; }
void lua_replace(lua_State *L, int idx) { (void)L; (void)idx; }
int lua_checkstack(lua_State *L, int e) { (void)L; (void)e; return 0; }

int lua_type(lua_State *L, int idx) { (void)L; (void)idx; return LUA_TNIL; }
const char *lua_typename(lua_State *L, int tp) { (void)L; (void)tp; return "nil"; }
int lua_isnumber(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
int lua_isstring(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
int lua_iscfunction(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
int lua_isuserdata(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
int lua_rawequal(lua_State *L, int a, int b) { (void)L; (void)a; (void)b; return 0; }
int lua_equal(lua_State *L, int a, int b) { (void)L; (void)a; (void)b; return 0; }
int lua_lessthan(lua_State *L, int a, int b) { (void)L; (void)a; (void)b; return 0; }

lua_Number lua_tonumberx(lua_State *L, int idx, int *isnum) {
    (void)L; (void)idx; if (isnum) *isnum = 0; return 0.0;
}
lua_Integer lua_tointegerx(lua_State *L, int idx, int *isnum) {
    (void)L; (void)idx; if (isnum) *isnum = 0; return 0;
}
int lua_toboolean(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
const char *lua_tolstring(lua_State *L, int idx, size_t *len) {
    (void)L; (void)idx; if (len) *len = 0; return "";
}
size_t lua_objlen(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
lua_CFunction lua_tocfunction(lua_State *L, int idx) { (void)L; (void)idx; return NULL; }
void *lua_touserdata(lua_State *L, int idx) { (void)L; (void)idx; return NULL; }

void lua_pushnil(lua_State *L) { (void)L; }
void lua_pushnumber(lua_State *L, lua_Number n) { (void)L; (void)n; }
void lua_pushinteger(lua_State *L, lua_Integer n) { (void)L; (void)n; }
void lua_pushboolean(lua_State *L, int b) { (void)L; (void)b; }
void lua_pushstring(lua_State *L, const char *s) { (void)L; (void)s; }
void lua_pushlstring(lua_State *L, const char *s, size_t len) { (void)L; (void)s; (void)len; }
void lua_pushlightuserdata(lua_State *L, void *p) { (void)L; (void)p; }
int lua_pushfstring(lua_State *L, const char *fmt, ...) { (void)L; (void)fmt; return 0; }
void lua_pushcfunction(lua_State *L, lua_CFunction fn) { (void)L; (void)fn; }
void lua_pushcclosure(lua_State *L, lua_CFunction fn, int n) { (void)L; (void)fn; (void)n; }
void lua_pushthread(lua_State *L) { (void)L; }

void lua_createtable(lua_State *L, int n, int r) { (void)L; (void)n; (void)r; }
void *lua_newuserdata(lua_State *L, size_t sz) { (void)L; (void)sz; return NULL; }
int lua_getmetatable(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
int lua_getfenv(lua_State *L, int idx) { (void)L; (void)idx; return 0; }

void lua_settable(lua_State *L, int idx) { (void)L; (void)idx; }
void lua_setfield(lua_State *L, int idx, const char *k) { (void)L; (void)idx; (void)k; }
void lua_rawset(lua_State *L, int idx) { (void)L; (void)idx; }
void lua_rawseti(lua_State *L, int idx, int n) { (void)L; (void)idx; (void)n; }
int lua_setmetatable(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
int lua_setfenv(lua_State *L, int idx) { (void)L; (void)idx; return 0; }

void lua_gettable(lua_State *L, int idx) { (void)L; (void)idx; }
void lua_getfield(lua_State *L, int idx, const char *k) { (void)L; (void)idx; (void)k; }
void lua_rawget(lua_State *L, int idx) { (void)L; (void)idx; }
void lua_rawgeti(lua_State *L, int idx, int n) { (void)L; (void)idx; (void)n; }

int lua_call(lua_State *L, int n, int r) { (void)L; (void)n; (void)r; return 0; }
int lua_pcall(lua_State *L, int n, int r, int ef) { (void)L; (void)n; (void)r; (void)ef; return 0; }
int lua_cpcall(lua_State *L, lua_CFunction f, void *ud) { (void)L; (void)f; (void)ud; return 0; }
int lua_load(lua_State *L, lua_Reader r, void *d, const char *cn) {
    (void)L; (void)r; (void)d; (void)cn; return 0;
}
int lua_next(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
void lua_concat(lua_State *L, int n) { (void)L; (void)n; }
int lua_yield(lua_State *L, int n) { (void)L; (void)n; return 0; }
int lua_resume(lua_State *L, int n) { (void)L; (void)n; return 0; }
int lua_status(lua_State *L) { (void)L; return 0; }

lua_Number lua_tonumber(lua_State *L, int idx) { (void)L; (void)idx; return 0.0; }
lua_Integer lua_tointeger(lua_State *L, int idx) { (void)L; (void)idx; return 0; }
const char *lua_tostring(lua_State *L, int idx) { return lua_tolstring(L, idx, NULL); }

lua_Number lua_checknumber(lua_State *L, int arg) { (void)L; (void)arg; return 0.0; }
lua_Integer lua_checkinteger(lua_State *L, int arg) { (void)L; (void)arg; return 0; }
const char *lua_checklstring(lua_State *L, int arg, size_t *len) {
    return luaL_checklstring(L, arg, len);
}
lua_Number lua_optnumber(lua_State *L, int arg, lua_Number d) { (void)L; (void)arg; return d; }
lua_Integer lua_optinteger(lua_State *L, int arg, lua_Integer d) { (void)L; (void)arg; return d; }

/* Debug API */
int lua_getstack(lua_State *L, int l, lua_Debug *a) { (void)L; (void)l; (void)a; return 0; }
int lua_getinfo(lua_State *L, const char *w, lua_Debug *a) {
    (void)L; (void)w; (void)a; return 0;
}
const char *lua_getlocal(lua_State *L, const lua_Debug *a, int n) {
    (void)L; (void)a; (void)n; return NULL;
}
const char *lua_setlocal(lua_State *L, const lua_Debug *a, int n) {
    (void)L; (void)a; (void)n; return NULL;
}
const char *lua_getupvalue(lua_State *L, int f, int n) { (void)L; (void)f; (void)n; return NULL; }
const char *lua_setupvalue(lua_State *L, int f, int n) { (void)L; (void)f; (void)n; return NULL; }
int lua_sethook(lua_State *L, int (*f)(lua_State *, lua_Debug *), int m, int c) {
    (void)L; (void)f; (void)m; (void)c; return 0;
}
int lua_gethook(lua_State *L) { (void)L; return 0; }
int lua_gethookmask(lua_State *L) { (void)L; return 0; }
int lua_gethookcount(lua_State *L) { (void)L; return 0; }
