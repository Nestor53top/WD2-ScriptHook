#pragma once

#include "lua.h"
#include "luaconf.h"

#define BITS_INT ((int)(sizeof(int) * CHAR_BIT))

#define LUAI_MAXSTACK 1000000
#define LUAI_MAXCCALLS 200
#define LUAI_MAXCSTACK 8000

#define cast(t, exp) ((t)(exp))
#define cast_num(i) cast(lua_Number, (i))
#define cast_int(i) cast(int, (i))
#define cast_uint(i) cast(unsigned int, (i))

#define cast_byte(i) cast(lu_byte, (i))
#define cast_char(i) cast(char, (i))

#define cast_sizet(i) cast(size_t, (i))

#define cast_void(i) cast(void, (i))

#define cast_luamem(i) cast(lu_mem, (i))

typedef unsigned char lu_byte;
typedef long long l_sizet;
typedef long long lu_mem;
typedef long long l_uacNumber;
typedef unsigned int l_uint32;

#define INTFRMLEN "l"
#define LUAI_INTFRMLEN "l"

#define LUAI_MAXNUMCASTS 10
#define LUAI_MAXSHORTLEN 40

#define MAXUPVAL 255

#define lua_assert(c) ((void)0)

#define cast2unsigned(i) cast(unsigned int, (i))

#define NUM_TAGS 9

#define LUA_COMPAT_ALL 0

#define LUA_COMPAT_FLOAT32 0

#define LUA_MAXINTEGER ((lua_Integer)(~((unsigned)0) >> 1))
#define LUA_MININTEGER (-LUA_MAXINTEGER - 1)

#define check_exp(c,e) (e)

#define api_check(l,e,msg) lua_assert(e)

#define UNUSED(x) ((void)(x))

#define api_incr_top(L) (L->top++)

#define setnilvalue(obj) ((obj)->tt= LUA_TNIL)

#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }

#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject*, (x)); i_o->tt=LUA_TSTRING | valuetype(x); \
    checkliveness(L,obj); }

#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject*, (x)); i_o->tt=LUA_TUSERDATA | valuetype(x); \
    checkliveness(L,obj); }

#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject*, (x)); i_o->tt=LUA_TFUNCTION | valuetype(x); \
    checkliveness(L,obj); }

#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject*, (x)); i_o->tt=LUA_TTABLE | valuetype(x); \
    checkliveness(L,obj); }

#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject*, (x)); i_o->tt=LUA_TTHREAD | valuetype(x); \
    checkliveness(L,obj); }

#define setdeadvalue(obj) ((obj)->tt = LUA_TNIL)

#define setobj(L,obj1,obj2) \
	{ TValue *o1=(obj1); *o1 = *(obj2); \
	  (void)L; checkliveness(L,o1); }

#define setobjs2s  setobj
#define setobj2s  setobj
#define setsobj2s setobj
#define setobj2n  setobj
#define setobj2t  setobj

#define luaC_checkGC(L) {condchangemem(L); luaC_step(L);}

#define condchangemem(L) \
  ((void)(check_exp(lua_isGc(L), 0), \
    (G(L)->GCestimate < G(L)->totalbytes/100*G(L)->gcpause)))

#define lua_isGc(L) 1

#define G(L) (L->l_G)

#define luaC_step(L) ((void)L)

#define checkliveness(L,obj) ((void)L)

#define valuetype(o) 0

#define LUA_TLCL 0
#define LUA_TLCF 1
#define LUA_TCC 2
#define LUA_TLC 3

#define luaC_upvalbarrier(L,uv) ((void)L)

#define luaC_upvdeccount(L,uv) ((void)L)
