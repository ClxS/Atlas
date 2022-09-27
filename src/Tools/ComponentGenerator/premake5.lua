if not toolsDirectory then
    error("Solution must set toolsDirectory variable")
end

project "ComponentGenerator"
	platforms { "Windows" }
	kind "ConsoleApp"
	targetdir(toolsDirectory)
	targetname "ComponentGenerator"
	language "C++"
	debugdir "$(TargetDir)"
	files {
		"**",
	}
	includedirs {
	    "**",
	}
	links {
		"tomlcpp",
		"tinyxml2",
		"ToolsCore",

		"AtlasTrace",
	}
	flags { "FatalWarnings" }
	cppdialect "C++latest"
