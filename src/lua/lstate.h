#pragma once

#include "lua.h"
#include "luaconf.h"

#define LUA_MAXCSTACK 8000

typedef struct CallInfo {
    StkId base;
    StkId func;
    StkId top;
    const Instruction *savedpc;
    unsigned short nresults;
    unsigned short callstatus;
} CallInfo;

typedef struct StackState {
    StkId top;
    StkId base;
    global_State *l_G;
    CallInfo *ci;
    StkId stack_last;
    StkId stack;
    int stacksize;
    unsigned short nny;
    unsigned short status;
    int errorJmp;
    CallInfo base_ci;
} StackState;

typedef struct global_State {
    lua_Alloc frealloc;
    void *ud;
    size_t totalbytes;
    size_t gcdept;
    int gcpause;
    int gcstepmul;
    int gcstep;
    lu_mem GCmemtrav;
    lu_mem GCestimate;
    stringtable strt;
    TValue l_registry;
    struct lua_State *mainthread;
    UpVal *uvhead;
    int gcrunning;
    int gcstp;
    lu_byte gcemergency;
    lu_byte gckind;
    lu_byte gmode;
    MObject *finobj;
    MObject *tobefnz;
} global_State;

struct lua_State {
    StkId top;
    StkId base;
    global_State *l_G;
    CallInfo *ci;
    StkId stack_last;
    StkId stack;
    int stacksize;
    unsigned short nny;
    unsigned short status;
    int errorJmp;
    CallInfo base_ci;
};

void luaE_freethread(lua_State *L, lua_State *L1);
CallInfo *luaE_extendCI(lua_State *L);
void luaE_freeCI(lua_State *L);
