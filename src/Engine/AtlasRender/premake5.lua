lib "AtlasRender"
    exports {
        ["links"] = {
            "bgfx",
            "eigen",
            "AtlasResource",
        }
    }

    generateAssetSpec("engine", "atlas::render::resources")
