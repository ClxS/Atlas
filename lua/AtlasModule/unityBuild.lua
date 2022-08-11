require "vstudio"
local p = premake
local vstudio = p.vstudio
local project = p.project
local config = p.config
local fileconfig = p.fileconfig
local tree = p.tree
local m = p.vstudio.vc2010

premake.api.register {
    name = "unitybuild",
    scope = "project",
    kind = "boolean",
    tokens = false,
}


local function addUnitySupport(prj)
    if prj.unitybuild then
        m.element("EnableUnitySupport", nil, 'true')
    end
end

premake.override(premake.vstudio.vc2010.elements, "configurationProperties", function(base, prj)
    local calls = base(prj)
    table.insert(calls, addUnitySupport)
    return calls
end)