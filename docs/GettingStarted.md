# Getting started | Pacc

Table of contents:

- [Getting started | Pacc](#getting-started--pacc)
	- [Creating a package script](#creating-a-package-script)
	- [Adding more projects](#adding-more-projects)
		- [A library within the same package](#a-library-within-the-same-package)
		- [An interface project](#an-interface-project)
	- [Dependency handling](#dependency-handling)
		- [Installing remote dependencies](#installing-remote-dependencies)
		- [Adding raw file dependencies](#adding-raw-file-dependencies)
		- [Using projects from local packages as a dependency](#using-projects-from-local-packages-as-a-dependency)
	- [Versioning your package](#versioning-your-package)

<hr/>

The main purpose of **pacc** is to provide easy package management and build tools to your project.

Everything is done by creating **package script** in the root directory of your project. A Package script is either a static JSON file (`cpackage.json`) or a dynamic Lua script (`cpackage.lua`, not supported yet).

For most things, JSON is enough. If you want to create more advanced scripts, use `cpackage.lua`, or bind Lua functions to your `cpackage.json` file (not supported yet).


## Creating a package script

Example `cpackage.json` contents:

```json
{
	"name": "MyApp",
	"type": "app",
	"files": "src/Main.cpp"
}
```

Create a simple "Hello World" program inside `src/Main.cpp`:

```cpp
#include <iostream>

int main() {
	std::cout << "Hello, World!";
}
```

then run:

```cpp
pacc build
```

to [build your project](Actions/Build.md). If everything went ok, you can [run your project](Actions/Run.md) with the following command:

```cpp
pacc run
```

## Adding more projects

If your workspace has more projects, you can change `cpackage.json` to contain a workspace definition.

This is a **single project** definition:

```json
{
	"name": "MyApp",
	"type": "app",
	"files": "src/Main.cpp"
}
```

This is a **workspace** definition with a single project:
```json
{
	"name": "MyWorkspace",
	"projects": [
		{
			"name": "MyApp",
			"type": "app",
			"files": "src/Main.cpp"
		}
	]	
}
```

### A library within the same package

`// TODO: ` [YouTube video tutorial](https://youtube.com)

Libraries contain reusable code across other projects. To create a library within the same package, add another project to the workspace:

```json
{
	"name": "MyWorkspace",
	"projects": [
		{
			"name": "MyApp",
			"type": "app",
			"files": "MyApp/src/Main.cpp",
			"dependencies": [ "self:MyLibrary" ]
		},

		{
			"name": "MyLibrary",
			"type": "static lib",

			"files": [
				"MyLibrary/include/MyLibrary/**.hpp",
				"MyLibrary/src/**.cpp"
			],
			
			"includeFolders": {
				"public": "MyLibrary/include"
			}
		}
	]	
}
```

Explanation:
1. When creating multiple projects inside a workspace, it is recommended to place them in separate folders. That's why **we changed `files` field from `MyApp` project to `MyApp/src/Main.cpp`**.
2. We added `MyLibrary` as the dependency to the `MyApp` project (`self:` means it's from the safe package)
3. We added a `static lib` project
4. A recommended folder structure for a library is:  
   1. Header files: `include/LibraryName/` 
   2. Source files: `src/`  
   
   And these folders are inside project folder (`MyLibrary/`), so the final `files` field look like this (`**.hpp` means every `.hpp` file inside the folder, or its subfolder):
   ```json
   "files": [
		"MyLibrary/include/MyLibrary/**.hpp",
		"MyLibrary/src/**.cpp"
	]
   ```

5. We added `MyLibrary/include` folder as a **public** include folder. Include folders are used by a compiler to locate header files. When you add `MyLibrary/include` as a include folder, you can later use:
   ```cpp
   #include <MyLibrary/HeaderFileName.hpp>
   ```
   to include a header file from the library. Because we added it as a **public** include folder, this will be available both inside the library and every project that use it as a dependency.

### An interface project

An interface project is a project that does not produce any binaries and can't be built. Instead, it provides configuration to projects that use it as a dependency.

An example interface project:

```json
{
	"name": "MyWorkspace",
	"projects": [
		{
			"name": "MyApp",
			"type": "app",
			"files": "src/Main.cpp",
			"dependencies": [ "self:AddPiDefine" ]
		},

		{
			"name": "AddPiDefine",
			"type": "interface",
			"defines": "PI_VALUE=3.1415926535"
		}
	]	
}
```

For interface projects, by default, all fields have `interface` access. That means that it affect only projects that use it as a dependency.

**Non-interface project** also can contain interface configuration (propagated to projects that use it as a dependency), for example:

```json
{
	"name": "MyLibrary",
	"type": "static lib",

	"files": [
		"MyLibrary/include/MyLibrary/**.hpp",
		"MyLibrary/src/**.cpp"
	],
	
	"includeFolders": {
		"public": "MyLibrary/include"
	},

	"defines": {
		"interface": "USES_MYLIBRARY_AS_DEPENDENCY=1"
	}
}
```

**For non-interface projects**, by default, every configuration has **private** access. That means that it's used only by the project itself.

## Dependency handling

Writing everything by yourself is unnecessary, tiring, and boring.

### Installing remote dependencies

You can use packages you can find on the internet as a dependency.

```json
{
	"name": "MyApp",
	"type": "app",
	"files": "src/Main.cpp",
	"dependencies": [ "fmt@7.1.3" ]
}
```

This will add `fmt` library [**from official pacc repository**](https://github.com/pacc-repo), version 7.1.3 as a dependency to your project.

Install missing dependencies with:

```json
pacc install
```

> Note: `install` can do a lot more, see [Installing packages](Actions/Install.md).

### Adding raw file dependencies

Sometimes you just want to link a specific library file as a dependency (`.a`, `.lib`). You can do that using `file:` prefix:

```json
{
	"name": "MyApp",
	"type": "app",
	"files": "src/Main.cpp",
	"dependencies": [ "file:someLibraryFile" ]
}
```

Make sure that the folder where `someLibraryFile` exists can be located by the linker (use `linkerFolders` field):

```json
{
	"name": "MyApp",
	"type": "app",
	"files": "src/Main.cpp",

	"linkerFolders": "path/to/someLibraryFileFolder",

	"dependencies": [ "file:someLibraryFile" ]
}
```

### Using projects from local packages as a dependency

When you develop a library, you often want your projects to use your local version of the library.

Let's say that you have `MyLibrary` project in a separate package somewhere on your disk:

```json
{
	"name": "MyLibrary",
	"type": "static lib",

	"files": [
		"include/MyLibrary/**.hpp",
		"src/**.cpp"
	],
	
	"includeFolders": {
		"public": "include"
	}
}
```

You actively develop it and want to test your changes. You create other package in totally different location, with a test `MyProject` app:

```json
{
	"name": "MyApp",
	"type": "app",
	"files": "src/Main.cpp"
}
```

Now you add `MyLibrary` as a dependency to `MyApp`

```json
"dependencies": [ "MyLibrary" ]
```
and **it doesn't work**!

`MyLibrary` is not visible from the perspective of `MyApp` by default.
To fix this:
1. Go to the `MyLibrary` project root:
2. Run the following command:
   ```
   pacc link
   ```

This will **link** the package to the user environment, making it visible from anywhere across your disk, as if it was installed globally.


## Versioning your package

You can add an optional `version` field to your `cpackage.json` to specify its version.

```json
{
	"name": "MyLibrary",
	"type": "static lib",

	"files": [
		"include/MyLibrary/**.hpp",
		"src/**.cpp"
	],
	
	"includeFolders": {
		"public": "include"
	},

	"version": "3.1.0"
}
```

Make sure that the version you set conforms to [semantic versioning standard](https://semver.org).

Later, when you use your library as a dependency, you can add a version requirement:

```json
"dependencies": [ "MyLibrary@^3.1.0" ]
```

`^3.1.0` means that the library is required to be at least of version `3.1.0` and have the same **major field** (here: `3`).

`// TODO: tell more about version requirements`
