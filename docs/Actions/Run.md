# Running your package project (`pacc run`)

## Introduction

When your package is built, you can run the [startup project](../CreatingCPackageJSON.md#StartupProject) by using the following command:

```
pacc run
```

It will run the startup project with the default configuration and platform (`Debug`, `x64`).

## Running other projects from the package

If you want to run another project, not the startup one, simply use:

```
pacc run ProjectName
```

> **Note:** project name is case sensitive

## Running other configurations

Use following switches to manipulate run settings:

<table>
	<tr>
		<th>Switch</th>
		<th>Aliases</th>
		<th>Description</th>
	</tr>
	<tr>
		<td><pre>--platform</pre></td>
		<td><pre>--plat<br>-p</pre></td>
		<td>Changes the run platform to the specified one.
		For now only <code>x64</code> and <code>x86</code> are supported (by default <code>x64</code>).</td>
	</tr>
	<tr>
		<td><pre>--configuration</pre></td>
		<td><pre>--config<br>--cfg<br>-c</pre></td>
		<td>Changes the run configuration to the specified one. 
		For now only <code>Debug</code> and <code>Release</code> are supported (by default <code>Debug</code>).</td>
	</tr>
</table>

## Examples


### 1. Run project `ProjName` from the current package

```
pacc run ProjName
```

### 2. Run the package startup project (Release, x86)

```
pacc run -p=x86 -c=Release
```

or:

```
pacc run --platform=x86 --configuration=Release
```

### 3. Run project `MyProject` (Release, x86)


```
pacc run MyProject -p=x86 -c=Release
```

or:

```
pacc run MyProject --platform=x86 --configuration=Release
```