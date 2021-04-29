add_requires("fmt 7.*")
add_requires("nlohmann_json 3.9.*", { alias="json" })

is_mode("debug")
	set_symbols("debug")


target("cpkg")

	set_kind("binary")
	set_languages("c++20")
	add_headerfiles("include/CppPkg/**.hpp")
	add_files("src/**.cpp")
	add_includedirs("include")
	add_packages("fmt", "json")
	