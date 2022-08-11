if not toolsDirectory then
    error("Solution must set toolsDirectory variable")
end

project "AssetBuilder"
	platforms { "Tools_Win64" }
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
		"tomlcpp_tools",
		"ToolsCore",
	}
	flags { "FatalWarnings" }
	cppdialect "C++latest"
