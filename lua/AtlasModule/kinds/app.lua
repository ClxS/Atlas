function app(name)
    project(name)
        kind "WindowedApp"
    	language "C++"
    	files {
    		"*.lua",
    		"include/**",
    		"src/**",
    		"platform/Windows/**",
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


