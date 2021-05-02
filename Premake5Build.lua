function platformDirectory(base)
	return path.join(base, "%{cfg.platform}/%{cfg.buildcfg}")
end

function addCustomDep(name)

	if name == "tiny-process-lib" then
		includedirs	( { path.join("deps", name, "include") } )
		libdirs		( { platformDirectory(path.join("deps", name, "lib")) } )
		links		( { "tiny-process-library" } )

	elseif name == "fmt" then
		includedirs ( { "deps/fmt/include" } )
		defines		( { "FMT_HEADER_ONLY" } )

	elseif name == "json" then
		includedirs ( { "deps/nlohmann_json/include" } )

	else
		printf("Unknown custom dependency %s\n", name)
	end
end

function configureWorkspace()
	platforms { "x86", "x64" }
	configurations { "Debug", "Release" }

	location ("build")
	targetdir(path.join(os.getcwd(), "bin/%{cfg.platform}/%{cfg.buildcfg}"))
	
	if os.host() == "macosx" then
		removeplatforms { "x86" }
		defines { "BLOCC_SYSTEM_MACOSX" }
	elseif os.host() == "linux" then
		defines { "BLOCC_SYSTEM_LINUX" }
	elseif os.host() == "windows" then 
		defines { "BLOCC_SYSTEM_WINDOWS" }
	else
		defines { "BLOCC_SYSTEM_UNKNOWN" }
	end


	filter "platforms:*32"
		architecture "x86"

	filter "platforms:*64"
		architecture "x86_64"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

	filter {}
end

function defaultProjectConfiguration()
	location (path.join(os.getcwd(), "build/%{prj.name}"))
end


workspace("Blocc")

	configureWorkspace()
	
	-- Main Project:
	project("Blocc")

		defaultProjectConfiguration()

		kind("ConsoleApp")
		language("c++")
		cppdialect("C++17")
		files( {
			"include/Blocc/**.hpp",
			"src/**.cpp"
		})
		includedirs("include")
		addCustomDep("fmt")
		addCustomDep("json")
		addCustomDep("tiny-process-lib")




