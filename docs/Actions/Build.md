# Building your package (`pacc build`)

## Introduction

Your package most likely has one or many projects that you want to build.

Use the following command to build them all:

```
pacc build
```

It will trigger build with default configuration and platform (`Debug`, `x64`).

## Building other configurations

Use following switches to manipulate build settings:

<table>
	<tr>
		<th>Switch</th>
		<th>Aliases</th>
		<th>Description</th>
	</tr>
	<tr>
		<td><pre>--target</pre></td>
		<td><pre>-t</pre></td>
		<td>Forces to build only single, specified target (project)</td>
	</tr>
	<tr>
		<td><pre>--platform</pre></td>
		<td><pre>--plat<br>-p</pre></td>
		<td>Changes the build platform to the specified one.
		For now only <code>x64</code> and <code>x86</code> are supported (by default <code>x64</code>).</td>
	</tr>
	<tr>
		<td><pre>--configuration</pre></td>
		<td><pre>--config<br>--cfg<br>-c</pre></td>
		<td>Changes the build configuration to the specified one. For now only <code>Debug</code> and <code>Release</code> are supported (by default <code>Debug</code>).</td>
	</tr>
	<tr>
		<td><pre>--verbose</pre></td>
		<td></td>
		<td>Enables the <b>verbose</b> mode. 
		Build logs will be printed directly to the output. <b>Verbose mode is disabled by default</b></td>
	</tr>
</table>

## Important notes

Pacc detects the absence of dependency binaries. If any dependency is not built, it will try to build it at the specified platform and configuration.

`// TODO: automatic change in dependency source code detection`

Note: it does not (yet) detect change in dependency source code.

## Examples


### 1. Build project `ProjName` from the current package

```
pacc build -t=ProjName
```

or:

```
pacc build --target=ProjName
```

> **Note:** name of the project is case sensitive

### 2. Build the package (Release, x86), show build output

```
pacc build -p=x86 -c=Release --verbose
```

or:

```
pacc build --platform=x86 --configuration=Release --verbose
```

### 3. Build project `MyProject`, show build output

```
pacc build -t=MyProject --verbose
```

or:

```
pacc build --target=MyProject --verbose
```