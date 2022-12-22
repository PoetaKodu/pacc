local class = require("pacc-sdk/middleclass")
local EventDispatcher = class('EventDispatcher')

function EventDispatcher:initialize()
	self.events = { }
end

function EventDispatcher:on(name, func)
	if type(self.events[name]) ~= "table" then
		self.events[name] = {}
	end

	table.insert(self.events[name], func)
end

function EventDispatcher:dispatch(name, ...)
	if type(self.events[name]) == "table" then
		for i, v in pairs(self.events[name]) do
			v(...)
		end
	end
end

return EventDispatcher
