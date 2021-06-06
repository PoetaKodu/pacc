# pacc  

An easy to use and powerful C++ 📦 package manager  with integrated 🛠 build tools, written in C++.


<hr/>
<table>
	<tr>
		<th><a href="docs/Installation.md">🗃 Installation</a>
		<th><a href="https://github.com/PoetaKodu/pacc/releases">💾 Download</a>
		<th><a href="docs/GettingStarted.md">🚀 Getting started</a>
		<th><a href="docs/Actions.md">🧱 Actions</a></th>
		<th><a href="#-contributing">👨‍🔧 Contributing</a></th>
	</tr>
</table>
<hr/>

## 👀 Quick overview (early 06.2021)


### Configure projects easily with [JSON](https://en.wikipedia.org/wiki/JSON)



```json
{
	"name": "HelloWorldProject",
	"type": "app",
	"files": "src/Main.hpp"
}
```
### Build with ease

All you need is a single command.

```
pacc build
```

<p align="center">
	<img src="res/img/build.gif" alt="Build"/>
</p>

### Install remote packages

Use:

```
pacc install
```

to install missing dependencies, and:

```
pacc install PackageName
```

to install specific package

<p align="center">
	<img src="res/img/install_and_build.gif" alt="Installing packages"/>
</p>

### Automatic toolchain detection

Installed toolchains are detected automatically (but [manual setup](res/docs/ManualToolchainSetup.md) is also supported):

```
pacc tc
```

<p align="center">
	<img src="res/img/toolchains.gif" alt="Toolchains"/>
</p>

### Rich editor support

- Self-explainatory syntax and naming
- Good docs
- Rapid development
- Editor does most of the job for you

<p align="center">
	<img src="res/img/support.gif" alt="Support"/>
</p>

Set up libraries and applications with few clicks or keystrokes.

### Insanely fast precompiled header setup

Use `"pch"` field inside `cpackage.json` to set up Precompiled Headers easily:

<p align="center">
	<img src="res/img/pch.gif" alt="Precompiled Headers"/>
</p>

### Example package

Folder contents:
```
- cpackage.json
- src/
	- Main.cpp
```

`cpackage.json` contents:

```json
{
	"name": "HelloWorld",
	"type": "app",
	"files": "src/*.cpp"
}
```

`Main.cpp` contents:
```cpp
#include <iostream>

int main() {
	std::cout << "Hello, World!";
}
```


## 👨‍🔧 Contributing

Contributions are appreciated.

`// TODO: contributing instructions`

## ⚽ Goals

pacc is hugely inspired by [**npm**](https://github.com/npm/cli) and rust's [**cargo**](https://github.com/rust-lang/cargo)

The main goal is to create a very easy to use, powerful and extensible package manager for C++, something that should've been created a long time ago. I know that there are other package managers. Their main flaw is that they use `CMake` as a (meta)build system, which is a **horrible** tool. I won't rant on that here... man, I hate it, but this time I will pass.

*\*Insert xkcd competing standards meme here\**

I won't give up until there is a decent and easy to use package manager with integrated build tools.

## 🖋 Credits

Written by Paweł Syska (@PoetaKodu).

For complete credit information, visit [this page](docs/Credits.md).
