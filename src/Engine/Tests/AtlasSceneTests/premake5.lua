test "AtlasSceneTests"
	kind "ConsoleApp"
	files {
		"premake5.lua",
	}
    links {
        "AtlasScene",
    }
    includedirs {
        "src",
    }
    linkoptions {
        "/IGNORE:4217", -- ignore linker warning about empty object file"
    }
