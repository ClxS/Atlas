project "tinyxml2"
	kind "StaticLib"
	files {
		"tinyxml2.lua",
		"tinyxml2/tinyxml2.cpp",
		"tinyxml2/tinyxml2.h",
	}
	exports {
		["includedirs"]	= path.getabsolute("tinyxml2"),
	}
