lib "AtlasRender"
    exports {
        ["links"] = {
            "bgfx",
            "imgui",
            "eigen",
            "AtlasResource",
        }
    }

    generateAssetSpec("engine", "atlas::render::resources")
