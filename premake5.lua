TOML = require "lua/ThirdParty/lua-toml"
require "lua/AtlasModule"

local engineRoot = ''

function setEngineRoot(path)
	engineRoot = path
end

function includeEngine()
	group("Engine")
	include(engineRoot .. "/src/Engine/AtlasAppHost")
    include(engineRoot .. "/src/Engine/AtlasCore")
    include(engineRoot .. "/src/Engine/AtlasGame")
    include(engineRoot .. "/src/Engine/AtlasInput")
    include(engineRoot .. "/src/Engine/AtlasMaths")
    include(engineRoot .. "/src/Engine/AtlasRender")
    include(engineRoot .. "/src/Engine/AtlasResource")
    include(engineRoot .. "/src/Engine/AtlasRpc")
    include(engineRoot .. "/src/Engine/AtlasScene")
    include(engineRoot .. "/src/Engine/AtlasTrace")
    include(engineRoot .. "/src/Engine/AtlasUI")

    group("Engine/Tools")
    include(engineRoot .. "/src/Tools/AtlasSceneEditor")
    include(engineRoot .. "/src/Tools/ComponentGenerator")
end

function includeEngineThirdParty()
	group("Engine/ThirdParty")
	include(engineRoot .. '/src/ThirdParty/benchmark.lua')
	include(engineRoot .. '/src/ThirdParty/eigen.lua')
	include(engineRoot .. '/src/ThirdParty/fixed_string.lua')
	include(engineRoot .. '/src/ThirdParty/freetype.lua')
	include(engineRoot .. '/src/ThirdParty/gtest.lua')
	include(engineRoot .. '/src/ThirdParty/imgui.lua')
	include(engineRoot .. '/src/ThirdParty/rmlui.lua')
	include(engineRoot .. '/src/ThirdParty/sdl.lua')
	include(engineRoot .. '/src/ThirdParty/sdlimage.lua')
	include(engineRoot .. '/src/ThirdParty/tomlcpp.lua')
	include(engineRoot .. '/src/ThirdParty/fmt.lua')
	include(engineRoot .. '/src/ThirdParty/tinyxml2.lua')
	include(engineRoot .. '/src/ThirdParty/zlib.lua')
	include(engineRoot .. '/src/ThirdParty/openssl.lua')

	group("Engine/ThirdParty/bgfx")
	include(engineRoot .. '/src/ThirdParty/bgfx.lua')
	include(engineRoot .. '/src/ThirdParty/bimg.lua')
	include(engineRoot .. '/src/ThirdParty/bx.lua')

	group("Engine/ThirdParty/gRPC")
	include(engineRoot .. '/src/ThirdParty/grpc.lua')
end

function includeEngineTests()
	group("Engine/Tests")
	include(engineRoot .. "/src/Engine/Tests/AtlasSceneTests")
end

function includeEngineTools()
	group("Engine/Tools")
    include(engineRoot .. "/src/Tools/Core")
    include(engineRoot .. "/src/Tools/AssetBuilder")

	include(engineRoot .. "/src/Tools/ProjectWizard")

	group("Engine/Tools/External")
	include(engineRoot .. "/src/External/Nephrite.lua")
end
