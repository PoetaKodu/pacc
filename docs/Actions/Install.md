# Installing dependencies (`pacc install`)

Packages often depend on other packages. Pacc searches for them in the following directories:

Local packages:
- `./pacc_packages` (`pacc_packages` in the root folder)
- `../pacc_packages` (the parent directory to the root one)

Global packages (shared across the disk):
- On Windows:
  - `AppData Folder/pacc/packages`
- On Linux:
  - `User Folder/pacc/packages`

> **Note:** it is recommended to install packages **globally** to save disk space.

## Installing packages

To install all missing dependencies locally:
```
pacc install
```

To install package locally:

```
pacc install PackagePattern
```
To install package globally:

```
pacc install PackagePattern --global
```

## Uninstalling packages


To uninstall package locally:

```
pacc uninstall PackagePattern
```
To uninstall package globally:

```
pacc uninstall PackagePattern --global
```


## Where to get packages from?

You can get any package from following sources:

### Our [official repo](https://github.com/pacc-repo)

It is a [GitHub account](https://github.com/pacc-repo) with forked repositories, made conformant to use with Pacc.

Package pattern:

```
Name@VersionRequirement
```

Example:  

Install `fmt` library [version](../PackageVersioning.md) `7.1.3` (locally)
```
pacc install fmt@7.1.3
```

List available `fmt` versions:
```
pacc list-versions fmt

// or:

pacc lsver fmt
```

### Any GitHub/GitLab account

Package pattern:

For [GitHub](https://github.com):
```
github:UserName/PackageName@VersionRequirement
```

For [GitLab](https://gitlab.com):
```
gitlab:UserName/PackageName@VersionRequirement
```

Example:  

Install `DummyLibrary` library, from user `DummyUser`, [version](../PackageVersioning.md) `2.1.3` (locally)
```
pacc install github:DummyUser/DummyLibrary@2.1.3
```

List available `DummyLibrary` versions:
```
pacc list-versions github:DummyUser/DummyLibrary

// or:

pacc lsver github:DummyUser/DummyLibrary
```