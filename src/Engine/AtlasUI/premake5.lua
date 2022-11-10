lib "AtlasUI"
    links {
        "SDL",
    }
    exports {
        ["links"] = {
            "AtlasCore",
            "AtlasRender",
            "AtlasResource",
            "AtlasAppHost",
            "AtlasTrace",

            "bgfx",
            "RmlUI",
        },
        ["defines"] = {
            "__STDC_FORMAT_MACROS",
        }
    }
