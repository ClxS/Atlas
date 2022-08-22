project "ToolsCore"
	platforms { "Windows" }
	kind "StaticLib"
	targetdir "bin/tools/lib"
	language "C++"
	files {
		"**",
	}
	defines {
	    "_CRT_SECURE_NO_WARNINGS",
	}
	includedirs {
		"**",
	}
	flags { "FatalWarnings" }
	cppdialect "C++latest"

	exports {
		["includedirs"]	= path.getabsolute("include"),
	}
