project "imgui"
	kind "None"
	filter {"platforms:Windows"}
		kind "StaticLib"
		includedirs {
			"imgui",
			"SDL/include",
		}
		files {
			"imgui/*.cpp",
			"imgui/*.h",
			"imgui/misc/cpp/*.cpp",
			"imgui/misc/cpp/*.h",
			"imgui/misc/debuggers/*.natvis",
			"imgui/backends/*_impl_sdl*",
			"ImGuizmo/*.h",
			"ImGuizmo/*.cpp",
		}
		flags { "MultiProcessorCompile" }

		exports {
			["includedirs"]	= {
				path.getabsolute("imgui"),
				path.getabsolute("ImGuizmo"),
			}
		}
	filter {}
	unitybuild(true)
