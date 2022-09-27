lib "AtlasGame"
    exports {
        ["links"] = {
            "AtlasAppHost",
            "AtlasCore",
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

    generateComponents("engine", "atlas::game");
