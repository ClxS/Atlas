local p = premake
local vstudio = p.vstudio
local config = p.config

function generateRootAssetSpec(group, namespace)
    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)
    local generatedFolder = origin .. '/generated/include'

    generateAssetSpecCore(group, namespace, dataDirectory, generatedFolder)
end


function generateAssetSpec(group, namespace)
    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)
    local dataFolder = path.getabsolute(origin .. 'data')
    local generatedFolder = origin .. '/generated/include'
    generateAssetSpecCore(group, namespace, dataFolder, generatedFolder)
end

function generateAssetSpecCore(project, group, namespace, dataFolder, generatedFolder)
    dependson {
        "AssetBuilder"
    }

    local builderPath = path.getabsolute(toolsDirectory .. '/AssetBuilder.exe')
    local command = string.format('%s -r "%s" -d -ns "%s" -o "%s" -g %s',
                    builderPath,
                    dataFolder,
                    namespace,
                    generatedFolder .. '/' .. project .. '/AssetRegistry.h',
                    group)

    print("Generating AssetSpec " .. group .. " - " .. namespace)
    --print("Generating AssetSpec " .. group .. " - " .. namespace .. "(" .. command .. ")")
    os.execute(command)
    
end

local function locateAssetRoots()
    print('Checking from ' .. rootDirectory .. ' for asset roots')
    return os.matchfiles(rootDirectory .. '/**/assets.root')
end

local function generateAssetSpecs()
    for _, file in pairs(locateAssetRoots()) do
        local f = io.open(file, "rb")
        local content = f:read("*all")
        f:close()

        local assetSpec = TOML.parse(content)
        local origin = getDirectory(file)
        local generatedFolder = origin .. '../generated/include'
        generateAssetSpecCore(
            assetSpec.assetRoot.project,
            assetSpec.assetRoot.group,
            assetSpec.assetRoot.namespace,
            origin,
            generatedFolder
        )

        project(assetSpec.assetRoot.project)
        files {
            generatedFolder .. "/**"
        }
        exports {
            ["includedirs"] = {
                generatedFolder,
            }
        }
    end
end

premake.override(premake.main, 'preBake', function(base, prj)
    generateAssetSpecs();
    base(prj)
end)
