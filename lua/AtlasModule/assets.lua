local p = premake
local vstudio = p.vstudio
local project = p.project
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

function generateAssetSpecCore(group, namespace, dataFolder, generatedFolder)
    dependson {
        "AssetBuilder"
    }

    local project = p.api.scope.project.name

    local builderPath = path.getabsolute(toolsDirectory .. '/AssetBuilder.exe')
    local command = string.format('%s -r "%s" -d -ns "%s" -o "%s" -g %s',
                    builderPath,
                    dataFolder,
                    namespace,
                    generatedFolder .. '/' .. project .. '/AssetRegistry.h',
                    group)

    print("Generating AssetSpec for " .. p.api.scope.project.name)
    --print("Generating AssetSpec for " .. p.api.scope.project.name .. "(" .. command .. ")")
    os.execute(command)

    files {
        generatedFolder .. "/**"
    }
    exports {
        ["includedirs"] = {
            generatedFolder,
        }
    }
end

premake.override(premake.main, 'preBake', function(base, prj)
    base(prj)
end)
