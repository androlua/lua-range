local _M = {
  _AUTHORS = "benpop",
  _VERSION = "1.1",
  _DESCRIPTION = "range generator (cf. Python (x)range, Ruby \"..\" operator)",
}


local _mt = {}  -- metatable


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


-- see Python-2.7.3/Objects/rangeobject.c:19:5
local function _len (lo, hi, step)
  if step > 0 and lo <= hi then
    return 1 + (hi - lo) / step
  elseif step < 0 and lo >= hi then
    return 1 + (lo - hi) / -step
  else
    return 0
  end
end


local BAD_RANGE_ARGS = "'range' needs 1-3 integer arguments"


function _M.new (a, b, c)
  a = assert(toint(a), BAD_RANGE_ARGS)
  if b then
    b = assert(toint(b), BAD_RANGE_ARGS)
  end
  if c then
    c = assert(b and toint(c), BAD_RANGE_ARGS)
    assert(c ~= 0, "'step' argument must not be zero")
  end
  if not b then
    assert(a >= 1, "single argument 'stop' must be positive")
  end
  local lo, hi, step
  if b then
    lo = a
    hi = b
    if c then
      step = c
    elseif lo <= hi then
      step = 1
    else
      step = -1
    end
  else
    lo = 1
    hi = a
    if lo <= hi then
      step = 1
    else
      step = -1
    end
  end
  return setmetatable({start=lo, step=step, len=_len(lo, hi, step)}, _mt)
end


function _M:__call (a, b, c)
  return self.new(a, b, c)
end


function _mt:get (i)
  return (i - 1) * self.step + self.start
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
  if 1 <= i and i <= self.len then
    return i, self:get(i)
  end
end


function _mt:__ipairs ()
  return _next, self, 0
end


function _mt:type ()
  return "range"
end


return setmetatable(_M, _M)

