function lib(name)
    project(name)
        kind "StaticLib"
    	language "C++"
    	files {
    		"*.lua",
    		"include/**",
    		"src/**",
    		"data/**",
    		"components/**",
    		"platform/Windows/**",
    		"natvis/**"
    	}
        includedirs {
            ".",
            "**",
        }

    	flags { "FatalWarnings" }
    	pchsource("src/" .. name .. "PCH.cpp")
    	pchheader(name .. "PCH.h")
        exports {
            ["includedirs"] = {
                path.getabsolute("include"),
            }
        }

        filter { "platforms:Windows" }
            includedirs {
                "platform/Windows/src",
            }
            exports {
                ["includedirs"] = {
                    path.getabsolute("platform/Windows/include"),
                }
            }
        filter {}
end


