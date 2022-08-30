project "zlib"
	kind "StaticLib"
	files {
		"zlib.lua",
		"zlib/*.c",
		"zlib/*.h",
	}
	exports {
		["includedirs"]	= path.getabsolute("zlib"),
	}
