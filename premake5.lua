require "AtlasModule"

function includeEngine()
	group("Engine")
	include("src/Engine/AtlasAppHost")
    include("src/Engine/AtlasCore")
    include("src/Engine/AtlasGame")
    include("src/Engine/AtlasInput")
    include("src/Engine/AtlasMaths")
    include("src/Engine/AtlasRender")
    include("src/Engine/AtlasResource")
    include("src/Engine/AtlasScene")
end

function includeEngineThirdParty()
	group("ThirdParty")
	include('src/ThirdParty/benchmark.lua')
	include('src/ThirdParty/eigen.lua')
	include('src/ThirdParty/fixed_string.lua')
	include('src/ThirdParty/freetype.lua')
	include('src/ThirdParty/gtest.lua')
	include('src/ThirdParty/imgui.lua')
	include('src/ThirdParty/rmlui.lua')
	include('src/ThirdParty/sdl.lua')
	include('src/ThirdParty/sdlimage.lua')
	include('src/ThirdParty/tomlcpp.lua')
	include('src/ThirdParty/fmt.lua')

	group("ThirdParty/bgfx")
	include('src/ThirdParty/bgfx.lua')
	include('src/ThirdParty/bimg.lua')
	include('src/ThirdParty/bx.lua')
end

function includeEngineTests()
	group("Engine/Tests")
	include("src/Engine/Tests/AtlasSceneTests")
end