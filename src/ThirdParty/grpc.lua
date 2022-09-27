    project "grpc_cpp_plugin"
        kind "ConsoleApp"
        targetdir(path.getabsolute('../../tools/protoc/plugins'))
        targetname('grpc_cpp_plugin')
        files {
            'grpc/src/Compiler/config*.h',
            'grpc/src/Compiler/schema*.h',
            'grpc/src/Compiler/generator*.h',
            'grpc/src/Compiler/cpp*.h',
            'grpc/src/Compiler/cpp*.cc',
        }
        includedirs {
            path.getabsolute('grpc'),
            path.getabsolute('grpc/include'),
        }
        defines {
            'WIN32_LEAN_AND_MEAN',
        }
        links {
            'protobuf',
        }
        warnings "Off"

project "protobuf"
    kind "StaticLib"
    exports {
        ["defines"] = {
		    'GOOGLE_PROTOBUF_NO_RTTI',
        },
        ["includedirs"] = {
            path.getabsolute('grpc/third_party/protobuf/src'),
        }
    }
    files {
        'grpc/third_party/protobuf/src/google/**',
    }
    excludes {
        '**/*mock_*',
        '**/*test_*',
        '**/*unittest*',
        '**/*_test*',
        '**/testing/**',
        '**/ruby_**',
    }
    links {
        'zlib',
    }
    warnings "Off"

project "abseil-cpp"
    kind "StaticLib"
    exports {
        ["includedirs"] = {
            path.getabsolute('grpc/third_party/abseil-cpp'),
        }
    }
    files {
        'grpc/third_party/abseil-cpp/absl/**.cc',
        'grpc/third_party/abseil-cpp/absl/**.h',
    }
    excludes {
        '**/*_test.cc',
        '**/*_testing.cc',
        '**/*_benchmark.cc',
        '**/*_benchmark.cc',
        '**/mocking_*.cc',
        '**/benchmarks.cc',
        '**/*_test_*.cc',
        '**/*_nonprod.cc',
    }
    defines {
        'WIN32_LEAN_AND_MEAN',
        'NOMINMAX',
    }
    disablewarnings {
        '26495',
        '26812',
    }

    characterset 'ASCII'
    warnings "Off"

project "upb"
    kind "StaticLib"
    exports {
        ["includedirs"] = {
            path.getabsolute('grpc/third_party/upb'),
            path.getabsolute('grpc/third_party/upb/generated_for_cmake'),
            path.getabsolute('grpc/src/core/ext/upbdefs-generated'),
        },
        ["links"] = {
            'protobuf',
            'abseil-cpp',
        }
    }
    language "C++"
    files {
        'grpc/third_party/upb/**.c',
        'grpc/third_party/upb/**.h',
    }
    defines {
        'WIN32_LEAN_AND_MEAN',
        'NOMINMAX',
    }
    excludes {
        '**/bindings/lua/**',
    }
    includedirs {
        'grpc/src/core/ext/upbdefs-generated',
        'grpc/src/core/ext/upb-generated',
    }
    excludes {
        '**/*mock_*',
        '**/*test_*',
        '**/*unittest*',
        '**/*_test*',
        '**/tests/**',
        '**/testing/**',
        '**/ruby_**',
        '**/python/**',
        '**/upbc/**',
        '**/conformance_upb.c',
    }
    warnings "Off"
    linkoptions {
        '/ignore:4221'
    }

project "grpc"
    kind "StaticLib"
    exports {
        ["includedirs"] = {
            path.getabsolute('grpc/include'),
            path.getabsolute('grpc/src/core/ext/upb-generated'),
            path.getabsolute('grpc/third_party/address_sorting/include'),
            path.getabsolute('grpc/third_party/cares'),
            path.getabsolute('grpc/third_party/cares/cares'),
        },
        ["links"] = {
            'protobuf',
            'abseil-cpp',
            'upb',
            'zlib',
        },
        ["defines"] = {
            'GPR_WINDOWS',
        },
    }
    language "C++"
    files {
        'grpc.lua',
        'grpc/include/**',
        'grpc/src/cpp/**',
        'grpc/src/core/**',
    }
    excludes {
        '**/init_secure.cc',
		'**/secure_create_auth_context.cc',
		'**/grpc_cronet_plugin_registry.cc',
		'**/xds_channel_secure.cc',
		'**/grpclb_channel_secure.cc',
		'**/server/load_reporter/**',
		'**/load_reporting/**',
        '**/census/**',
    }
    files {
        '**/census/grpc_context.cc',
    }
    excludes {
        '**/google_c2p/**',
    }
    includedirs {
        path.getabsolute('grpc'),
        path.getabsolute('grpc/third_party/cares/cares/include'),
        path.getabsolute('grpc/third_party/xxhash'),
    }
    defines {
        'WIN32_LEAN_AND_MEAN',
        'NOMINMAX',
    }
    links {
        'Ws2_32.lib',
        'OpenSSL'
    }

    warnings "Off"
    linkoptions { "/ignore:4221" }
    files {
        'grpc/third_party/address_sorting/**',
    }

    files {
        'grpc/third_party/cares/**',
    }
    includedirs {
        'grpc/third_party/cares/cares/src/lib',
    }
    excludes {
        '**/test/**',
        '**ahost.c',
        '**adig.c',
    }
	defines {
		'CARES_STATICLIB',
	}

	files {
	    'grpc/third_party/re2/**',
	}
	excludes {
		'**/fuzz.cc',
		'**/re2_fuzzer.cc',
        '**.BUILD',
    }
	includedirs {
	    'grpc/third_party/re2',
	}

local function runCompiler(matches, include, outType, extraArgs)
	local protocPath = path.getabsolute('../../tools/protoc/bin/protoc.exe')
	local protoIncludes = path.getabsolute(include)
	local cppOut = path.getabsolute('grpc')

	for _,v in ipairs(matches) do
		if 	not string.find(v, "ruby") and
			not string.find(v, "test_") and
			not string.find(v, "_test") and
			not string.find(v, "PluginTest") and
			not string.find(v, "altscontext") and
			not string.find(v, "handshaker")
			then
			local command = string.format('%s "%s" --%s="%s" --proto_path="%s" %s',
					protocPath,
					v,
					outType,
					cppOut,
					protoIncludes,
					extraArgs)
			os.execute(command)
		end
	end
end

local function generateProtobufFiles()
	print('Generating proto files')

	local pluginPath = path.getabsolute('../../tools/protoc/plugins/grpc_cpp_plugin.exe')
	local pluginArg = '--plugin=protoc-gen-grpc="' .. pluginPath .. '"'

	local matches = os.matchfiles(path.getabsolute('grpc/third_party/protobuf/src/**.proto'))
	runCompiler(matches, 'grpc/third_party/protobuf/src', 'cpp_out', '')

	matches = os.matchfiles(path.getabsolute('grpc/third_party/upb/**.proto'))
	runCompiler(matches, 'grpc/third_party/upb', 'cpp_out', pluginArg)

	matches = os.matchfiles(path.getabsolute('grpc/src/**.proto'))
	runCompiler(matches, 'grpc', 'cpp_out', pluginArg)
	runCompiler(matches, 'grpc', 'grpc_out', pluginArg)
end

generateProtobufFiles()
