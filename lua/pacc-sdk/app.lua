local class				= require("pacc-sdk/middleclass")
local EventDispatcher	= require("pacc-sdk/event-dispatcher")

local PaccApp = class('PaccApp', EventDispatcher)

function PaccApp:dump()
	local result = ""
	result = result .. fmt.string("=== pacc info ===\n\n")
	result = result .. fmt.string("- version: {}\n", self.version:toString())
	result = result .. fmt.string("\n=================")
	return result
end

pacc = PaccApp:new()
