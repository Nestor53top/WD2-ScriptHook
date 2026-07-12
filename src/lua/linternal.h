#pragma once

#include "lua.h"
#include "lobject.h"

typedef union {
    void *gc;
    void *p;
    lua_Number n;
    int i;
} Value;

typedef struct GCheader {
    lu_byte tt;
    lu_byte marked;
} GCheader;

typedef struct Table {
    GCheader h;
    lu_byte flags;
    lu_byte lsizenode;
    unsigned int size;
    Node *node;
    Node *lastfree;
    GCObject *gclist;
    int sizearray;
    TValue *array;
} Table;

typedef struct LClosure {
    GCheader h;
    lu_byte nupvalues;
    GCObject *gclist;
    struct Proto *p;
    UpVal *upvals[1];
} LClosure;

typedef struct CClosure {
    GCheader h;
    lua_CFunction f;
    GCObject *gclist;
    int nupvalues;
    UpValue *upval[1];
} CClosure;

typedef union Closure {
    LClosure l;
    CClosure c;
} Closure;

typedef unsigned int StkId;
typedef unsigned int Instruction;
typedef void *MObject;

#define LUA_TLCL	(TFunction - 0)
#define LUA_TLCF	(TFunction - 1)
#define LUA_TCC		(TFunction - 2)

#define LUA_TLUTABLE (LUA_TTABLE | (0 << 4))
#define LUA_TLUSERDATA (LUA_TUSERDATA | (0 << 4))
#define LUA_TLSTR (LUA_TSTRING | (0 << 4))

#define LUA_TSHRSTR (LUA_TSTRING | (0 << 4))
#define LUA_TLNGSTR (LUA_TSTRING | (1 << 4))

#define LUA_TNUMINT (LUA_TNUMBER | (0 << 4))
#define LUA_TNUMFLT (LUA_TNUMBER | (1 << 4))

#define LUA_TLCL	(TFunction - 0)
#define LUA_TLCF	(TFunction - 1)
#define LUA_TCC		(TFunction - 2)

#define LUA_VFALSE	0
#define LUA_VTRUE	1
#define LUA_VNIL	0

#define ttisnumber(o) (ttisinteger(o) || ttisfloat(o))
#define ttisinteger(o) ((o)->tt == LUA_TNUMINT)
#define ttisfloat(o) ((o)->tt == LUA_TNUMFLT)
#define ttisstring(o) (ttisshrstring(o) || ttislngstring(o))
#define ttisshrstring(o) ((o)->tt == LUA_TSHRSTR)
#define ttislngstring(o) ((o)->tt == LUA_TLNGSTR)
#define ttistable(o) ((o)->tt == LUA_TTABLE)
#define ttisfunction(o) ((o)->tt == LUA_TFUNCTION)
#define ttisboolean(o) ((o)->tt == LUA_TBOOLEAN)
#define ttisnil(o) ((o)->tt == LUA_TNIL)
#define ttispointer(o) ((o)->tt == LUA_TLIGHTUSERDATA)
#define ttisuserdata(o) ((o)->tt == LUA_TUSERDATA)
#define ttisthread(o) ((o)->tt == LUA_TTHREAD)

#define tonumber(o,n) (ttisinteger(o) ? (*(n)=(lua_Number)(o)->value.i, 1) : \
                       ttisfloat(o) ? (*(n)=(o)->value.n, 1) : 0)

#define geti(o,n) (ttisinteger(o) ? (*(n)=(o)->value.i) : \
                   ttisfloat(o) ? (*(n)=(lua_Integer)(o)->value.n) : 0)

#define seti(o,n) ((o)->value.i = (n), (o)->tt = LUA_TNUMINT)
#define setf(o,n) ((o)->value.n = (n), (o)->tt = LUA_TNUMFLT)

typedef struct Table *MObjectTable;

#define LUA_SIGNATURE "\27Lua"
#define LUA_NUMTAGS 9

#define LUA_TNONE	(-2)
#define LUA_TNIL	0
#define LUA_TBOOLEAN	1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER	3
#define LUA_TSTRING	4
#define LUA_TTABLE	5
#define LUA_TFUNCTION	6
#define LUA_TUSERDATA	7
#define LUA_TTHREAD	8

#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)

typedef double lua_Number;
typedef long long lua_Integer;

typedef struct {
    Value value;
    int tt;
} TValue;

typedef struct {
    GCObject *next;
    lu_byte tt;
    lu_byte marked;
} GCObject;

typedef struct String {
    GCObject gc;
    lu_byte tt;
    lu_byte marked;
    lu_byte extra;
    unsigned int hash;
    size_t len;
} String;

typedef struct {
    GCObject *next;
    GCObject **toclose;
    TValue k;
    int val;
    int t;
} MObject;

#define GCS_PROPAGATE	0
#define GCS_SWEEPSTR	1
#define GCS_SWEEPUDATA	2
#define GCS_SWEEPZ	3
#define GCS_CALLGC	4
#define GCS_ENTER	5
#define GCS_COLLECT	6
#define GCS_EMERGENCY	7

typedef struct UpVal {
    TValue tv;
    union {
        struct {
            UpVal *next;
            UpVal **prev;
        } open;
    } u;
} UpVal;

typedef struct LocVar {
    String *varname;
    int startpc;
    int endpc;
} LocVar;

typedef struct Proto {
    GCObject gc;
    lu_byte numparams;
    lu_byte nups;
    lu_byte is_vararg;
    lu_byte maxstacksize;
    int sizecode;
    int sizek;
    int sizep;
    int sizecode;
    int sizeabslineinfo;
    int sizelineinfo;
    int sizeLocvars;
    int sizeupvalues;
    Instruction *code;
    TValue *k;
    Proto **p;
    int *lineinfo;
    LocVar *locvars;
    UpValueDesc *upvalues;
    int linedefined;
    int lastlinedefined;
    String *source;
} Proto;

typedef struct UpValueDesc {
    String *name;
    int idx;
} UpValueDesc;

typedef struct CallInfoK {
    int k;
    int pc;
    int nresults;
    struct CallInfoK *next;
    short ntransfer;
    StkId transfer;
} CallInfoK;

#define MULTRET	(-1)

typedef struct global_State global_State;

struct LClosure;

typedef struct lua_State {
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
} lua_State;

typedef struct CallInfo {
    StkId func;
    StkId base;
    StkId top;
    const Instruction *savedpc;
    unsigned short nresults;
    short callstatus;
    StkId extra;
    union {
        struct {
            int k;
            int pc;
            int nresults;
            struct CallInfo *next;
            short ntransfer;
            StkId transfer;
        } l;
    } u;
} CallInfo;

struct global_State {
    stringtable strt;
    lua_Alloc frealloc;
    void *ud;
    lu_byte currentwhite;
    lu_byte gcstate;
    int sweepstrgc;
    GCObject *rootgc;
    GCObject *sweepgc;
    GCObject *finobj;
    GCObject *tobefnz;
    GCObject *fixedgc;
    GCObject *Reallymark;
    GCObject *allgc;
    GCObject ** sweepgc;
    lu_mem totalbytes;
    lu_mem GCestimate;
    int gcstop;
    int gcemergency;
    int gcpause;
    int gcstepmul;
    int gcstep;
    int gcstepsize;
    int gckind;
    int gcmode;
    int lastatomic;
    lu_mem GCmemtrav;
    lu_mem GCmemtravestimate;
    int gcstp;
    int gcfinrunning;
    struct lua_State *mainthread;
    UpVal *uvhead;
    MObject *firstatomic;
};

typedef struct stringtable {
    GCObject **hash;
    unsigned int nuse;
    int size;
} stringtable;

#define lua_lock(L) ((void)0)
#define lua_unlock(L) ((void)0)

#define api_incr_top(L) (L->top++)

#define api_check(L,e,msg) lua_assert(e)

#define setobj(L,obj1,obj2) \
	{ TValue *o1=(obj1); *o1 = *(obj2); \
	  (void)L; checkliveness(L,o1); }

#define setobj2n  setobj
#define setobj2t  setobj

#define checkliveness(L,obj) ((void)L)

#define luaC_checkGC(L) {condchangemem(L); luaC_step(L);}

#define condchangemem(L) ((void)0)

#define G(L) (L->l_G)

#define luaC_step(L) ((void)L)

#define luaC_upvalbarrier(L,uv) ((void)L)

#define luaC_upvdeccount(L,uv) ((void)L)

#define valuetype(o) 0
