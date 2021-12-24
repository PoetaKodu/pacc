local class				= require("pacc-std/middleclass")
local EventDispatcher	= require("pacc-std/event-dispatcher")

local PaccApp = class('PaccApp', EventDispatcher)

function PaccApp:dump()
	local result = ""
	result = result .. fmt.string("=== pacc info ===\n\n")
	result = result .. fmt.string("- version: {}\n", self.version:toString())
	result = result .. fmt.string("\n=================")
	return result
end

pacc = PaccApp:new()
