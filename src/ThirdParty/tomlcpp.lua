project "tomlcpp"
	kind "StaticLib"
	files {
		"tomlcpp/toml.h",
		"tomlcpp/toml.c",
		"tomlcpp/tomlcpp.hpp",
		"tomlcpp/tomlcpp.cpp",
		"tomlcpp.lua",
	}

	exports {
		["includedirs"]	= path.getabsolute("tomlcpp"),
	}
	unitybuild(true)
