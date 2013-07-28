local _M = {
  _AUTHORS = "benpop",
  _VERSION = "1.2",
  _DESCRIPTION = "range generator (cf. Python (x)range, Ruby \"..\" operator)"
}


local _mt = {}  -- metatable


-- see Python-2.7.3/Objects/rangeobject.c:19
local function _len (lo, hi, step)
  if step > 0 and lo <= hi then
    return 1 + (hi - lo) / step
  elseif step < 0 and lo >= hi then
    return 1 + (lo - hi) / -step
  else
    return 0
  end
end


local function toint (x)
  local n = tonumber(x)
  if n == nil then
    return nil
  end
  -- truncate toward zero
  if n > 0 then
    return math.floor(n)
  elseif n < 0 then
    return math.ceil(n)
  else
    return 0
  end
end


local function checkint (name, argn, ...)
  local arg = select(argn, ...)
  return toint(arg) or
      error(string.format(
          "bad argument #%d to 'range': '%s' (expected integer, got %s)",
          argn, name, type(arg)), 2)
end


function _M.new (...)
  local n = select("#", ...)
  local lo, hi, step
  if n == 1 then
    hi = checkint("stop", 1, ...)
    assert(hi >= 1, "single argument 'stop' must be positive")
    lo, step = 1, 1
  elseif n == 2 then
    lo = checkint("start", 1, ...)
    hi = checkint("stop", 2, ...)
    step = lo <= hi and 1 or -1
  elseif n == 3 then
    lo = checkint("start", 1, ...)
    hi = checkint("stop", 2, ...)
    step = checkint("step", 3, ...)
  else
    error("'range' needs 1-3 arguments")
  end
  return setmetatable({start=lo, step=step, len=_len(lo, hi, step)}, _mt)
end


function _M:__call (...)
  return self.new(...)
end


function _mt:get (i)
  if 1 <= i and i <= self.len then
    return (i - 1) * self.step + self.start
  end
end


function _mt:stop ()
  return self:get(self.len)
end


function _mt:__tostring ()
  local lo, hi, step = self.start, self:stop(), self.step
  if lo == 1 and step == 1 then
    return string.format("range(%d)", hi)
  elseif (step == 1 and lo <= hi) or (step == -1 and lo >= hi) then
    return string.format("range(%d, %d)", lo, hi)
  else
    return string.format("range(%d, %d, %d)", lo, hi, step)
  end
end


function _mt:__len ()
  return self.len
end


function _mt:__index (k)
  local i = tonumber(k)
  if i then
    return self:get(i)
  else
    return _mt[k]
  end
end


local function _next (self, i)
  i = i + 1  -- next index
  local v = self:get(i)
  if v then
    return i, v
  end
end


function _mt:__ipairs ()
  return _next, self, 0
end


function _mt:type ()
  return "range"
end


return setmetatable(_M, _M)

