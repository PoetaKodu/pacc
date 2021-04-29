# cpp-pkg
An easy to use and powerful C++ package manager with an integrated build system.

## 1. Quick Start

Download the package manager and then do the following:

### 1. To create a new project:

Open a terminal inside an empty folder and type the following command:

```
cpkg init
```

This will create a default package inside the current folder (see [init templates](docs/InitTemplates.md)).

### 2. To build a package

Run following command

```
cpkg build
```

This will regenerate build files and build the application with default mode (see [build modes](docs/BuildModes.md))

## 2. Example package

Folder contents:
```
- package.json
- src/
	- Main.cpp
```

`package.json` contents:

```json
{
	"name": "HelloWorld",
	"type": "app",
	"files": [ "src/*.cpp" ]
}
```

`Main.cpp` contents:
```cpp
#include <iostream>

int main() {
	std::cout << "Hello, World!";
}
```