Python-like (x)range object for Lua.

Has similar semantics to Python's (x)range:

	range(stop)
	range(start, stop)
	range(start, stop, step)

Comes with a Lua implementation that uses a table.

Library functions:
	range() -->  same as range.new()
	new([start, ] stop [, step])  -->  new range object
	bless(table)  -->  (Lua-only) turn table into range object

Methods:
	[Metamethods]
	tostring(obj)  -->  "range(...)"
	#obj  -->  number of items in range
	ipairs(obj)  -->  iterator over range
	obj[index]  -->  same as obj:get(index)
	obj:method(...)  -->  call one of the below methods

	[Regular methods]
	(The first three are fields in the Lua implementation.)
	start  -->  first number in the range
	stop  -->  last number in the range
	step  -->  interval size of the range
	get(index)  -->  get range item at index
	totable  -->  convert range to a table sequence/list

C source coded for Lua 5.2.

--benpop--
