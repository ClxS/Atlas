if not toolsDirectory then
    error("Solution must set toolsDirectory variable")
end

project "AssetBuilder"
	platforms { "Windows" }
	kind "ConsoleApp"
	targetdir(toolsDirectory)
	targetname "AssetBuilder"
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
		"ToolsCore",

		"AtlasTrace",
	}
	flags { "FatalWarnings" }
	cppdialect "C++latest"
