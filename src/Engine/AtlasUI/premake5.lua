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
        ["DataNamespace"] = 'atlas::ui::resources::registry',
    }
    if _ACTION == 'vs2022' then
        customtargets {
            [[
              <Target Name="GenerateAssetSpec" BeforeTargets="InitializeBuildStatus">
                  <Message Text="Generating Asset Specification" Importance="High" />
                  <Exec Command="$(ToolsDir)/AssetBuilder.exe -r $(DataDir) --platform $(BuildPlatform) -d --ns $(DataNamespace) -o $(CodeDir)/src/Generated/AssetRegistry.h" />
              </Target>
            ]]
        }
    end
