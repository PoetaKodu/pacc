# Available actions

Pacc can do various things. Just use:

```
pacc [action]
```

Whenever you find yourself lost, just use:

```
pacc help
```

to list available actions.

## Action list

<table>
	<tr>
		<th>Name</th>
		<th>Aliases</th>
		<th>Description</th>
	</tr>
	<tr>
		<td><a href="Actions/Build.md">Build</a></td>
		<td><pre>build</pre></td>
		<td>Builds package or a project.
			<br/><br/><small>Use <code>-cc</code> to export <a href="https://clang.llvm.org/docs/JSONCompilationDatabase.html"><code>compile_commands.json</code></a> (experimental)</small>
		</td>
	</tr>
	<tr>
		<td><a href="Actions/Run.md">Run</a></td>
		<td><pre>run</pre></td>
		<td>Runs package project (startup by default).
			<br/><br/><small><code>pacc run ProjectName</code> to run another one.</small>
		</td>
	</tr>
	<tr>
		<td><a href="Actions/Generate.md">Generate</a></td>
		<td><pre>generate</pre></td>
		<td>Generates <a href="https://premake.github.io">Premake5</a> build files. It is automatically run everytime you run <code>build</code> action.<br/><br/><small>Use <code>-cc</code> to export <a href="https://clang.llvm.org/docs/JSONCompilationDatabase.html"><code>compile_commands.json</code></a> (experimental)</small>
		</td>
	</tr>
	<tr>
		<td><a href="Actions/Install.md">Install</a></td>
		<td><pre>install</pre></td>
		<td>Installs missing dependencies (no params) or specified package <br/><br/><small><code>--global</code> to install globally</small>
		</td>
	</tr>
	<tr>
		<td><a href="Actions/Install.md">Uninstall</a></td>
		<td><pre>uninstall</pre></td>
		<td>Uninstalls specified package <br/><br/><small><code>--global</code> to uninstall globally</small>
		</td>
	</tr>
	<tr>
		<td><a href="Actions/Toolchains.md">Toolchains</a></td>
		<td><pre>toolchains<br/>tc</pre></td>
		<td>Lists available toolchains (no params), or selects the active one (f.e.: <code>pacc tc 1</code>)</td>
	</tr>
	<tr>
		<td><a href="Actions/Link.md">Link / Unlink</a></td>
		<td><pre>link<br/>unlink</pre></td>
		<td>Links or unlinks local package from user environment (other packages, from other folders can use it as a dependency)</td>
	</tr>
	<tr>
		<td><a href="Actions/Logs.md">Logs</a></td>
		<td><pre>logs<br/>log</pre></td>
		<td>Lists recent build logs.<br/><br/><small><code>--last</code> to output last build log</td>
	</tr>
	<tr>
		<td><a href="Actions/ListVersions.md">List versions</a></td>
		<td><pre>list-versions<br/>lsver</pre></td>
		<td>Lists versions of specified package<br/><br/><small><code>--all</code> to display not compatible ones</td>
	</tr>
	<tr>
		<td><a href="Actions/Help.md">Help</a></td>
		<td><pre>help</pre></td>
		<td>Displays help message (with available actions)</td>
	</tr>
</table>
