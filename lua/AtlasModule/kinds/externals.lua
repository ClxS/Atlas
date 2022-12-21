local function external(name, fileLocation)
    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)

    externalproject(name)
        language "C#"
        location(fileLocation)
end

function externalapp(name, fileLocation)
    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)
    local outLocation = origin
    if fileLocation then
        outLocation = origin .. '/' .. fileLocation
    end

    external(name, outLocation)
    kind "WindowedApp"
end

function externalconsole(name, fileLocation)
    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)
    local outLocation = origin
    if fileLocation then
        outLocation = origin .. '/' .. fileLocation
    end

    external(name, outLocation)
    kind "ConsoleApp"
end

function externallib(name, fileLocation)
    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)
    local outLocation = origin
    if fileLocation then
        outLocation = origin .. '/' .. fileLocation
    end

    external(name, outLocation)
    kind "SharedLib"
end


