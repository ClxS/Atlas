lib "AtlasGame"
    exports {
        ["links"] = {
            "AtlasAppHost",
            "AtlasCore",
            "AtlasInput",
            "AtlasUI",
            "AtlasScene",
            "AtlasRender",
            "AtlasResource",
            "AtlasRpc",
            "AtlasAppHost",
            "AtlasTrace",

            "bgfx",
            "RmlUI",
        }
    }

    generateComponents("engine", "atlas::game::components");
