local p = premake
local vstudio = p.vstudio
local project = p.project
local config = p.config

function generateRpcServices()
    local origin = debug.getinfo(2).source
    origin = origin:sub(2)
    origin = getDirectory(origin)
    
    local rpcFolder = path.getabsolute(origin .. 'rpc')
	local protoFiles = os.matchfiles(rpcFolder .. '/**.proto')
    local protoOut = rpcFolder .. '/generated'

    local protocPath = path.getabsolute('../../../tools/protoc/bin/protoc.exe')
	local pluginPath = path.getabsolute('../../../tools/protoc/plugins/grpc_cpp_plugin.exe')
	local pluginArg = '--plugin=protoc-gen-grpc="' .. pluginPath .. '"'

    local outKind = {
        "cpp_out",
        "grpc_out",
    }

    for _,kind in ipairs(outKind) do
        for _,v in ipairs(protoFiles) do
            local command = string.format('%s "%s" --%s="%s" --proto_path="%s" %s',
                            protocPath,
                            v,
                            kind,
                            protoOut,
                            rpcFolder,
                            pluginArg)                            
            os.execute('mkdir -p "' .. protoOut .. '" 2> NUL')
            os.execute(command)
        end
    end

    files {
        rpcFolder .. "/**"
    }
    includedirs {
        protoOut
    }
    filter { "files:**.pb.cc" }
        flags {
            "NoPCH",
        }
    filter {}
end