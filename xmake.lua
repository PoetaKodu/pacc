add_requires("fmt 7.*")

target("cpp-pkg")

	set_kind("binary")
	set_languages("c++20")
	add_headerfiles("include/CppPkg/**.hpp")
	add_files("src/**.cpp")
	add_includedirs("include")
	add_packages("fmt")
