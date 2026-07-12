#pragma once

#include <stddef.h>

#define LUA_NUMBER double
#define LUAI_UINT32 unsigned int
#define LUAI_INT32 int
#define LUAI_UINT64 unsigned long long
#define LUAI_MAXINT32 2147483647
#define LUAI_UMAXINT 4294967295u
#define LUAI_MAXSHORTLEN 40

#define LUAI_BITSINT 32
#define LUAI_MAXCSTACK 8000
#define LUAI_MAXCCALLS 200
#define LUAI_MAXSTACK 1000000
#define LUAI_MAXCAPTURES 32

#define LUA_NUMBER_FMT "%.14g"
#define LUAI_UFMTPRINT "lu"

#define LUA_INTFRMLEN "l"

#define l_mathop(fn) fn

#define lua_number2integer(n, p) { lua_Number _n = (n); *(p) = (lua_Integer)_n; }
#define lua_number2unsigned(n, p) { LUAI_UINT32 _n = (LUAI_UINT32)(n); *(p) = _n; }
#define lua_unsigned2number(n) ((lua_Number)(n))

#define LUAI_MAXNUMCASTS 10

#define lua_numbertointeger(n, p) lua_number2integer(n, p)

#define LUAI_THROW(L, c) longjmp((c)->b, 1)
#define LUAI_TRY(L, c, a) if (setjmp((c)->b) == 0) { a }

#define LUAI_MAXCALLS 20000

typedef struct lua_longjmp {
    struct lua_longjmp *previous;
    jmp_buf b;
    volatile int status;
} lua_longjmp;

#include <setjmp.h>
