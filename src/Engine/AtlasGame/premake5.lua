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

    generateComponents("engine", "atlas::game::components");

    filter {"platforms:Windows"}
        customprops {
            ["BuildPlatform"] = 'Windows',
        }
    filter {}

    dependson {
        "AssetBuilder"
    }
    customprops {
        ["ToolsDir"] = toolsDirectory,
        ["DataDir"] = path.getabsolute('data'),
        ["CodeDir"] = path.getabsolute('.'),
        ["DataNamespace"] = 'atlas::game::resources::post_process::registry',
    }
    if _ACTION == 'vs2022' then
        customtargets {
            [[
              <Target Name="GenerateAssetSpec" BeforeTargets="InitializeBuildStatus">
                  <Message Text="Generating Asset Specification" Importance="High" />
                  <Exec Command="$(ToolsDir)/AssetBuilder.exe -r $(DataDir) --platform $(BuildPlatform) -d --ns $(DataNamespace) -o $(CodeDir)/src/Generated/PostProcess_AssetRegistry.h" />
              </Target>
            ]]
        }
    end
