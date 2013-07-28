#include "lua.h"
#include "lauxlib.h"


#define AUTHORS "benpop"
#define VERSION "1.1"
#define DESCRIPTION "range generator (based on Python xrange)"


#define RANGE_NAME "range"
#define RANGE_TYPE "generator.range"


typedef struct Range {
  lua_Integer start;
  lua_Integer step;
#if 0
  lua_Integer stop;
#endif
  lua_Integer len;
} Range;


#define toRange(L) luaL_checkudata(L, 1, RANGE_TYPE)

#define getItem(r,i) (((i) - 1) * (r)->step + (r)->start)

#define getStop(r) getItem((r), (r)->len)


/* see Python-2.7.3/Objects/rangeobject.c:19:5 */
static size_t range_len (lua_Integer lo, lua_Integer hi,
                         lua_Integer step) {
  if (step > 0 && lo <= hi)
    return (size_t)1 + (hi - lo) / step;
  else if (step < 0 && lo >= hi)
    return (size_t)1 + (lo - hi) / ((size_t)0 - step);
  else
    return 0;
}


static int range_new (lua_State *L) {
  Range *r;
  size_t n;
  lua_Integer lo, hi, step = 0;
  switch (lua_gettop(L)) {
    case 1:
      hi = luaL_checkinteger(L, 1);
      luaL_argcheck(L, hi >= 1, 1, "single-argument \"stop\" must be positive");
      lo = 1;
      break;
    case 2:
      lo = luaL_checkinteger(L, 1);
      hi = luaL_checkinteger(L, 2);
      break;
    case 3:
      lo = luaL_checkinteger(L, 1);
      hi = luaL_checkinteger(L, 2);
      step = luaL_checkinteger(L, 3);
      luaL_argcheck(L, step != 0, 3, "\"step\" must not be zero");
      break;
    default:
      return luaL_error(L, LUA_QL(RANGE_NAME) " needs 1-3 arguments");
  }
  if (step == 0)
    step = hi >= lo ? 1 : -1;
  n = range_len(lo, hi, step);
  /* XXX: LONG_MAX assumed for ptrdiff_t <= lua_Integer */
  if (n > (size_t)LONG_MAX)
    return luaL_error(L, LUA_QL(RANGE_NAME) " result has too many items");
  r = lua_newuserdata(L, sizeof *r);
  luaL_setmetatable(L, RANGE_TYPE);
  r->start = lo;
  r->step = step;
  r->len = (lua_Integer)n;
#if 0
  if (n != 0)
    r->stop = (r->len - 1) * step + lo;
#endif
  return 1;
}


static int tostring (lua_State *L) {
  const Range *r = toRange(L);
  const lua_Integer stop = getStop(r);
  if (r->start == 1 && r->step == 1)
    lua_pushfstring(L, RANGE_NAME "(%d)", stop);
  else if ((r->step == 1 && r->start <= stop)
           || (r->step == -1 && r->start >= stop))
    lua_pushfstring(L, RANGE_NAME "(%d, %d)", r->start, stop);
  else
    lua_pushfstring(L, RANGE_NAME "(%d, %d, %d)", r->start, stop, r->step);
  return 1;
}


static int start (lua_State *L) {
  const Range *r = toRange(L);
  lua_pushinteger(L, r->start);
  return 1;
}


static int stop (lua_State *L) {
  const Range *r = toRange(L);
  lua_pushinteger(L, getStop(r));
  return 1;
}


static int step (lua_State *L) {
  const Range *r = toRange(L);
  lua_pushinteger(L, r->step);
  return 1;
}


static int len (lua_State *L) {
  const Range *r = toRange(L);
  lua_pushinteger(L, r->len);
  return 1;
}


static void pushitem (lua_State *L, const Range *r, const lua_Integer i) {
  if (1 <= i && i <= r->len)
    lua_pushinteger(L, getItem(r, i));
  else
    lua_pushnil(L);
}


static int get (lua_State *L) {
  const Range *r = toRange(L);
  const lua_Integer i = luaL_checkinteger(L, 2);
  pushitem(L, r, i);
  return 1;
}


static int next (lua_State *L) {
  const Range *r = toRange(L);
  const lua_Integer i = luaL_checkinteger(L, 2) + 1;
  lua_pushinteger(L, i);  /* next index */
  pushitem(L, r, i);
  return lua_isnil(L, -1) ? 1 : 2;
}


static int iter (lua_State *L) {
  (void)toRange(L);
  lua_pushcfunction(L, next);
  lua_pushvalue(L, 1);  /* range object */
  lua_pushinteger(L, 0);  /* initial key */
  return 3;
}


static int index (lua_State *L) {
  const Range *r = toRange(L);
  int isnum;
  const lua_Integer i = lua_tointegerx(L, 2, &isnum);
  if (isnum)
    pushitem(L, r, i);
  else {  /* try method name */
    const char *name = luaL_checkstring(L, 2);
    lua_rawget(L, lua_upvalueindex(1));  /* Methods.method */
    if (lua_isnil(L, -1))
      return luaL_error(L, "invalid method name: " LUA_QS, name);
  }
  return 1;
}


static int totable (lua_State *L) {
  const Range *r = toRange(L);
  lua_Integer i;
  lua_createtable(L, r->len, 0);
  for (i = 1; i <= r->len; i++) {
    lua_pushinteger(L, getItem(r, i));
    lua_rawseti(L, -2, i);
  }
  return 1;
}


static int lib_call_new (lua_State *L) {
  lua_pushcfunction(L, range_new);
  lua_replace(L, 1);  /* put "new" function under args (replacing lib table) */
  lua_call(L, lua_gettop(L)-1, 1);
  return 1;
}


static const luaL_Reg Lib[] = {
  {"new", range_new},
  {"__call", lib_call_new},
  {NULL, NULL}
};


static const luaL_Reg Meta[] = {
  {"totable", totable},
  {"start", start},
  {"stop", stop},
  {"step", step},
  {"get", get},
  {"__tostring", tostring},
  {"__ipairs", iter},
  {"__len", len},
  {NULL, NULL}
};


#define setField(field) \
  lua_pushliteral(L, field); \
  lua_setfield(L, -2, "_" #field)


#if 0
static void setLibMetaData (lua_State *L) {
  lua_pushliteral(L, AUTHORS);
  lua_setfield(L, -2, "_AUTHORS");
  lua_pushliteral(L, VERSION);
  lua_setfield(L, -2, "_VERSION");
  lua_pushliteral(L, DESCRIPTION);
  lua_setfield(L, -2, "_DESCRIPTION");
}
#endif


static void ownMetaTable (lua_State *L) {
  lua_pushvalue(L, -1);
  lua_setmetatable(L, -2);
}


int luaopen_range (lua_State *L) {
  /* Range type metatable */
  luaL_newmetatable(L, RANGE_TYPE);
  luaL_setfuncs(L, Meta, 0);
  ownMetaTable(L);
  lua_pushvalue(L, -1);
  lua_pushcclosure(L, index, 1);  /* enclose metatable in index function */
  lua_setfield(L, -2, "__index");
  /* Range library module */
  luaL_newlib(L, Lib);
  setField(AUTHORS);
  setField(VERSION);
  setField(DESCRIPTION);
  ownMetaTable(L);
  return 1;
}

