# Third-party dependencies

Provide headers and binaries of libraries into corresponding subfolders.

## Table of dependencies:

<table>
	<tr>
		<th>Dependency</th>
		<th>Link</th>
		<th>Version/Tag</th>
	</tr>
	<tr>
		<td><code>tiny-process-library</code></td>
		<td><a href="https://gitlab.com/eidheim/tiny-process-library">git repo</a></td>
		<td>latest</td>
	</tr>
	<tr>
		<td><code>fmt</code></td>
		<td><a href="https://github.com/fmtlib/fmt">git repo</a></td>
		<td><a href="https://github.com/fmtlib/fmt/tree/7.1.3">7.1.3</a></td>
	</tr>
	<tr>
		<td><code>json</code></td>
		<td><a href="https://github.com/nlohmann/json">git repo</a></td>
		<td><a href="https://github.com/nlohmann/json/tree/v3.9.1">v3.9.1</a></td>
	</tr>
</table>

## Instructions

Put:
- header files into `include/LibName` folder:
- binary files into `lib`**/**`<platform (x86|x64)>`**/**`<mode (debug|release)>`

<br/>

<hr/>
<center><b>Note:</b> You can use symlinks</center>
<hr/>

On Windows:
```batch
mklink /D "lib-name" "path/to/existing/folder" 
```

On Linux:

```bash
ln -s /path/to/folder/link /path/to/existing/folder
```


## Goals

Make Blocc powerful enough to handle its dependencies by itself.