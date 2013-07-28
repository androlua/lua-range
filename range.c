#include "lua.h"
#include "lauxlib.h"


#define AUTHORS "benpop"
#define VERSION "1.0"
#define DESC "range generator (cf. Python (x)range, Ruby \"..\" operator)"


#define RANGE_NAME "range"
#define RANGE_TYPE "generator.range"


typedef struct Range {
  int start, stop, step;
  int cursor;
} Range;


#define toRange(L) ((Range *)luaL_checkudata(L, 1, RANGE_TYPE))

#define testRange(L) ((Range *)luaL_testudata(L, 1, RANGE_TYPE))


static int r_new (lua_State *L) {
  Range *rng;
  int a, b, c;
  int hasB, hasC;
  a = luaL_checkint(L, 1);
  if ((hasB = !lua_isnoneornil(L, 2)))
    b = luaL_checkint(L, 2);
  if ((hasC = !lua_isnoneornil(L, 3))) {
    c = luaL_checkint(L, 3);
    luaL_argcheck(L, hasB, 2, "missing \"stop\" argument");
    luaL_argcheck(L, c != 0, 3, "\"step\" argument must not be zero");
  }
  rng = (Range *)lua_newuserdata(L, sizeof *rng);
  luaL_setmetatable(L, RANGE_TYPE);
  if (hasB) {
    rng->start = a;
    rng->stop = b;
    rng->step = hasC ? c : (rng->start <= rng->stop ? 1 : -1);
  }
  else {
    rng->start = 1;
    rng->stop = a;
    rng->step = 1;
  }
  rng->cursor = rng->start;
  return 1;
}


static int tostring (lua_State *L) {
  Range *rng = toRange(L);
  lua_pushfstring(L, "%s(%d, %d, %d)", RANGE_NAME, rng->start, rng->stop, rng->step);
  return 1;
}


static int reset (lua_State *L) {
  Range *rng = toRange(L);
  rng->cursor = rng->start;
  return 1;
}


static int start (lua_State *L) {
  Range *rng = toRange(L);
  lua_pushinteger(L, rng->start);
  return 1;
}


static int stop (lua_State *L) {
  Range *rng = toRange(L);
  lua_pushinteger(L, rng->stop);
  return 1;
}


static int step (lua_State *L) {
  Range *rng = toRange(L);
  lua_pushinteger(L, rng->step);
  return 1;
}


static int cursorInBounds (Range *rng) {
  if (rng->step > 0)
    return rng->cursor <= rng->stop;
  else
    return rng->cursor >= rng->stop;
}


static void pushCursor (lua_State *L, Range *rng) {
  if (cursorInBounds(rng))
    lua_pushinteger(L, rng->cursor);
  else
    lua_pushnil(L);
}


static int peek (lua_State *L) {
  Range *rng = toRange(L);
  pushCursor(L, rng);
  return 1;
}


static int next (lua_State *L) {
  Range *rng = toRange(L);
  pushCursor(L, rng);
  rng->cursor += rng->step;
  return 1;
}


/*
** rng([resetBeforeIterating?])
** rng:iter([resetBeforeIterating?])
**
** reset before iterating? : default false
**
** Return iterator function that iterates through all the elements
** of the range, starting at the current state of the range by default.
*/
static int iter (lua_State *L) {
  Range *rng = toRange(L);
  if (lua_toboolean(L, 2))  /* reset before iterating? */
    rng->cursor = rng->start;
  lua_pushcfunction(L, next);
  lua_pushvalue(L, 1);  /* range object */
  lua_pushnil(L);  /* dummy initial key */
  return 3;
}


static int remaining (lua_State *L) {
  Range *rng = toRange(L);
  if (cursorInBounds(rng)) {
    int nelems = (rng->stop - rng->start) / rng->cursor;
    lua_pushinteger(L, nelems);
  }
  else lua_pushinteger(L, 0);
  return 1;
}


static int size (lua_State *L) {
  Range *rng = toRange(L);
  int diff = rng->stop - rng->start;
  int rem = diff % rng->step;
  int nelems = (rng->stop - rng->start) / rng->step;
  lua_pushinteger(L, nelems);
  return 1;
}


static int type (lua_State *L) {
  Range *rng = testRange(L);
  if (rng != NULL)
    lua_pushliteral(L, RANGE_TYPE);
  else
    lua_pushnil(L);
  return 1;
}


static int libCallNew (lua_State *L) {
  lua_pushcfunction(L, r_new);
  lua_replace(L, 1);  /* put "new" function under args (replacing lib table) */
  lua_call(L, lua_gettop(L)-1, 1);
  return 1;
}


static const luaL_Reg Lib[] = {
  {"new", r_new},
  {"tostring", tostring},
  {"type", type},
  {"reset", reset},
  {"peek", peek},
  {"next", next},
  {"iter", iter},
  {"start", start},
  {"stop", stop},
  {"step", step},
  {"size", size},
  {"remaining", remaining},
  {NULL, NULL}
};


static const luaL_Reg Meta[] = {
  {"__tostring", tostring},
  {"__call", iter},
  {"__len", size},
#ifdef TM_ITER
  {"__iter", iter},
#endif
#ifdef TM_NEXT
  {"__next", next},
#endif
  {NULL, NULL}
};


static const luaL_Reg LibMeta[] = {
  {"__call", libCallNew},
  {NULL, NULL}
};


static void createRangeMetaTable (lua_State *L) {
  luaL_newmetatable(L, RANGE_TYPE);
  luaL_setfuncs(L, Meta, 0);
  lua_pushvalue(L, -2);  /* Lib table */
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);  /* pop metatable */
}


static void setLibMetaTable (lua_State *L) {
  luaL_newlib(L, LibMeta);
  lua_setmetatable(L, -2);
}


static void setLibMetaData (lua_State *L) {
  lua_pushliteral(L, AUTHORS);
  lua_setfield(L, -2, "_AUTHOR");
  lua_pushliteral(L, VERSION);
  lua_setfield(L, -2, "_VERSION");
  lua_pushliteral(L, DESC);
  lua_setfield(L, -2, "_DESCRIPTION");
  lua_pushliteral(L, RANGE_TYPE);
  lua_setfield(L, -2, "_TYPENAME");
}


int luaopen_range (lua_State *L) {
  luaL_newlib(L, Lib);
  setLibMetaTable(L);
  setLibMetaData(L);
  createRangeMetaTable(L);
  return 1;
}

