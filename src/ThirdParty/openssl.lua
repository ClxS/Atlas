project "OpenSSL"
	kind "Utility"
	exports {
		["includedirs"]	= {
		    path.getabsolute("openssl/include"),
		    path.getabsolute("../../prebuilt/openssl/Win64/include")
        },
        ["links"] = {
            "libcrypto",
            "libssl"
        }
	}
	filter{"configurations:Debug"}
	    exports {
            ["libdirs"] = {
                path.getabsolute("../../prebuilt/openssl/Win64/lib/debug")
            }
        }
    filter{"configurations:Release"}
	    exports {
            ["libdirs"] = {
                path.getabsolute("../../prebuilt/openssl/Win64/lib/release")
            }
        }
    filter{}

	files {
	    "openssl.lua",
	}
