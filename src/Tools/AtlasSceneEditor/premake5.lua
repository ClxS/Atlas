if not toolsDirectory then
    error("Solution must set toolsDirectory variable")
end

lib "AtlasSceneEditor"
    links {
        "AtlasAppHost",
        "AtlasGame",
        "AtlasScene",
        "AtlasResource",
        "AtlasRender",

        "grpc"
    }
    generateRpcServices()
    generateCoreComponentRegistry("atlas::scene_editor", { 'engine', 'scene_editor' });
