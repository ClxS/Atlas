lib "AtlasRpc"
    exports {
        ["links"] = {
            "grpc",

            "AtlasCore",
            "AtlasTrace",
        }
    }
    defines {
        "_SILENCE_CXX20_CISO646_REMOVED_WARNING",
        "_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS"
    }
