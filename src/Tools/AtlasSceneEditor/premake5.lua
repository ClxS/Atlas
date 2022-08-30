if not toolsDirectory then
    error("Solution must set toolsDirectory variable")
end

app "AtlasSceneEditor"
    links {
        "AtlasAppHost",
        "AtlasGame",
        "AtlasScene",
        "AtlasResource",
        "AtlasRender",

        "grpc"
    }
    generateRpcServices()