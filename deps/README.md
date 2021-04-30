# Third-party dependencies

Provide headers and binaries of libraries into corresponding subfolders.

Put:
- header files into `include/LibName` folder:
- binary files into `lib`**/**`<platform (x86|x64)>`**/**`<mode (debug|release)>`

<br/>

> **Note:** You can use symlinks:

On Windows:
```batch
mklink /D "lib-name" "path/to/existing/folder" 
```

On Linux:

```bash
ln -s /path/to/folder/link /path/to/existing/folder
```

## List of dependencies

- `tiny-process-library` ([git repo](https://gitlab.com/eidheim/tiny-process-library))
  