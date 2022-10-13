local p = premake
local vstudio = p.vstudio
local project = p.project
local config = p.config

local componentNamespaceGroups = {}
local outputRegistries = {}

function generateComponents(group, namespace)
    if componentNamespaceGroups[group] == nil then
        componentNamespaceGroups[group] = {}
    end

    table.insert(componentNamespaceGroups[group], namespace)

    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)

    local generatorPath = path.getabsolute(toolsDirectory .. '/ComponentGenerator.exe')
    local componentFolder = path.getabsolute(origin .. 'components')
    local command = string.format('%s -r "%s" -p "%s" -n "%s"',
                    generatorPath,
                    componentFolder,
                    p.api.scope.project.name,
                    namespace)

    print("Generating components for " .. p.api.scope.project.name)
    --print("Generating components for " .. p.api.scope.project.name .. "(" .. command .. ")")
    os.execute(command)

    files {
        componentFolder .. "/**"
    }
    exports {
        ["includedirs"] = {
            componentFolder .. '/generated',
        }
    }
end

function generateCoreComponentRegistry(namespace, groups)
    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)
    local componentFolder = path.getabsolute(origin .. 'components')

    local registration = {}
    registration.namespace = namespace
    registration.destination = componentFolder
    registration.project = p.api.scope.project.name
    registration.groups = groups

    table.insert(outputRegistries, registration)

    files {
        componentFolder .. "/**"
    }
    includedirs {
        componentFolder .. '/generated',
    }
end

function buildRegistries()
    for _, v in pairs(outputRegistries) do

        local groupString = ""
        local allGroups = {};

        for _, groupName in pairs(v.groups) do
            if componentNamespaceGroups[groupName] ~= nil then
                for _, groupNs in pairs(componentNamespaceGroups[groupName]) do
                    table.insert(allGroups, groupNs)
                end
            end
        end

        for i=1, #allGroups do
            if i > 1 then
                groupString = groupString .. ';'
            end

            groupString = groupString .. allGroups[i]
        end

        local generatorPath = path.getabsolute(toolsDirectory .. '/ComponentGenerator.exe')
        local command = string.format('%s registry -r "%s" -p "%s" -n "%s" -g "%s"',
                    generatorPath,
                    v.destination,
                    v.project,
                    v.namespace,
                    groupString)

        print("Generating component registry " .. v.project)
        --print("Generating component registry " .. v.project .. '(' .. command .. ')')
        os.execute(command)
    end
end

premake.override(premake.main, 'preBake', function(base, prj)
    buildRegistries()
    base(prj)
end)
