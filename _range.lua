local _M = {
  _AUTHORS = "benpop",
  _VERSION = "1.0",
  _DESCRIPTION = "range generator (cf. Python (x)range, Ruby \"..\" operator)",
  _TYPENAME = "generator.range",
}


local _metatable = {}


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


function _M.new (a, b, c)
  a = assert(toint(a), "bad argument #1 to 'range.new' (expected integer)")
  if b then
    b = assert(toint(b), "bad argument #2 to 'range.new' (expected integer)")
  end
  if c then
    c = assert(toint(c), "bad argument #3 to 'range.new' (expected integer)")
    assert(b, "bad argument #2 to 'range.new' (missing \"stop\" argument)")
    assert(c ~= 0,
      "bad argument #3 to 'range.new' (\"step\" argument must not be zero)")
  end
  local self = {start=0, stop=0, step=0, cursor=0}
  if b then
    self.start = a
    self.stop = b
    if c then
      self.step = c
    elseif self.start <= self.stop then
      self.step = 1
    else
      self.step = -1
    end
  else
    self.start = 1
    self.stop = a
    if self.start <= self.stop then
      self.step = 1
    else
      self.step = -1
    end
  end
  self.cursor = self.start
  return setmetatable(self, _metatable)
end


function _M:__call (a, b, c)
  return self.new(a, b, c)
end


function _M:tostring ()
  return string.format("range(%d, %d, %d)", self.start, self.stop, self.step)
end


function _M:reset ()
  self.cursor = self.start
  return self
end


function _M:peek ()
  if self:cursorInBounds() then
    return self.cursor
  end
end


function _M:cursorInBounds ()
  if self.step > 0 then
    return self.cursor <= self.stop
  else
    return self.cursor >= self.stop
  end
end


function _M:next ()
  local cursor = self.cursor
  local retval
  if self:cursorInBounds() then
    retval = cursor
  end
  self.cursor = cursor + self.step
  return retval
end


function _M:iter (reset)
  if reset then self:reset() end
  return self.next, self, nil
end


function _M:type ()
  if type(self) == "table" and self.start and self.stop and
      self.step and self.cursor then
    return self._TYPENAME
  end
end


_metatable.__index = _M
_metatable.__call = _M.iter
_metatable.__tostring = _M.tostring


return setmetatable(_M, _M)

