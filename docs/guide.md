# Forge Guide

## Project Layout

Forge does not require a specific directory structure. Packages can be placed anywhere in the project.

You can define packages directly inside the root `forge.json`, or place them in separate directories with their own `forge.json`.

Only the root package can have a `version.txt` file.

For example:

```text
my_workspace/
├── packages/
│   ├── networking/
│   │   └── forge.json
│   ├── gateway/
│   │   └── forge.json
│   └── unit_tests/
│       └── forge.json
├── forge.json
└── version.txt
```

Build files and cached data are stored in `.forge/`. Depending on the configuration, Forge may use `.forge_<profile>/` instead.

Forge also writes a `bootstrap.log` or build log to this directory while building the project.

---

## forge.json Reference

### Top-Level Keys

| Key            | Description                                                                                            |
| -------------- | ------------------------------------------------------------------------------------------------------ |
| `storage`      | Defines named values that can be reused anywhere in the file with `$(NAME)`.                           |
| `defines`      | Defines preprocessor macros that are applied to every package.                                         |
| `includes`     | Defines include directories that are available to every package.                                       |
| `dependencies` | Lists external modules managed through `forge-link`. Each module has an `id`.                          |
| `packages`     | Defines local packages. They can be declared directly in the file or loaded from another `forge.json`. |
| `resources`    | Lists files and directories that are copied to the output directory by `forge copy`.                   |
| `tests`        | Lists package IDs that should be run by `forge test`.                                                  |

### Package Types

| Type         | Output                                         |
| ------------ | ---------------------------------------------- |
| `executable` | Runnable program                               |
| `library`    | Static library, created with `ar` or `llvm-ar` |
| `shared`     | Shared library                                 |

### Package Properties

| Property    | Description                                                                                       |
| ----------- | ------------------------------------------------------------------------------------------------- |
| `id`        | Identifies the package. This property is required.                                                |
| `type`      | Defines the package type. See the package types above. The default is `executable`.               |
| `target`    | Defines the output name. Forge automatically adds the appropriate extension for the package type. |
| `output`    | Defines the exact output filename. No extension is added automatically.                           |
| `sources`   | Lists the files and directories to compile. Paths are relative to the package directory.          |
| `includes`  | Lists directories used when searching for header files.                                           |
| `libpaths`  | Lists directories used when searching for libraries.                                              |
| `libraries` | Lists libraries to link. Each library is passed to the linker with `-l`.                          |
| `defines`   | Defines preprocessor macros for the package.                                                      |
| `requires`  | Lists the packages that this package depends on. These can be local packages or external modules. |
| `flags`     | Defines flags used during both compilation and linking or archive creation.                       |
| `cflags`    | Defines flags used only during compilation.                                                       |
| `bflags`    | Defines flags used only during linking or archive creation.                                       |

All array properties can also be defined separately for each environment. If no matching environment is found, Forge uses the `default` value.

For example:

```json
{
    "defines": {
        "windows": [ "_CRT_SECURE_NO_WARNINGS" ],
        "default": []
    }
}
```

---

## Splitting Packages Across Files

Packages can be declared directly inside the root `forge.json`, or they can be placed in separate files.

For example:

```text
my_workspace/
├── packages/
│   ├── networking/
│   │   └── forge.json
│   ├── gateway/
│   │   └── forge.json
│   └── unit_tests/
│       └── forge.json
└── forge.json
```

The root `forge.json` can reference these packages by their paths:

```json
{
    "packages": [
        "packages/networking",
        "packages/gateway",
        "packages/unit_tests"
    ]
}
```

Each path points to a directory containing its own `forge.json`.

That file uses the same package definition format as an inline package, but contains only one package.

Both approaches can be used together. Some packages can be declared directly in the root file while others are stored in separate files.

---

## Special Storage Keys

Two `storage` keys have a special meaning inside Forge. They control which tools Forge uses for the build.

| Key        | Description                                               |
| ---------- | --------------------------------------------------------- |
| `compiler` | Defines the compiler and linker executable used by Forge. |
| `archiver` | Defines the archiver used to create static libraries.     |

The default tools are `clang` and `llvm-ar`.

On Linux, the defaults are `gcc` and `ar`.

For example:

```json
{
    "storage": {
        "compiler": {
            "linux": "clang++-20",
            "default": "clang"
        }
    }
}
```

Here, Forge uses `clang++-20` on Linux and `clang` on other environments.

These values can also be used like normal storage values and referenced with `$(NAME)`.

---

## Environment-Conditional Values

String and array properties can have different values depending on the current environment.

For example:

```json
{
    "defines": {
        "windows": [ "_CRT_SECURE_NO_WARNINGS" ],
        "default": []
    },
    "libraries": {
        "windows": [ "Advapi32" ],
        "default": []
    }
}
```

In this example, `_CRT_SECURE_NO_WARNINGS` and `Advapi32` are used when building on Windows.

Forge automatically creates environment tags based on the current system.

Some examples are:

```text
windows
posix
linux
darwin
bsd
win32
win64
```

Forge can also provide tags for the system, platform, vendor, ABI, and other parts of the build environment.

Additional tags can be added with `--env`.

If you want to ignore the automatically generated environment and start with an empty set of tags, use `--bare-env`.

---

## External Modules

External modules are declared in the `dependencies` array.

For example:

```json
{
    "dependencies": [
        {
            "id": "SpookLibrary",
            "includes": [ "include" ],
            "libpaths": [ "lib" ],
            "libraries": [ "SpookLib" ]
        }
    ]
}
```

A package can use an external module by adding its ID to `requires`:

```json
{
    "requires": [
        "stdext",
        "SpookLibrary"
    ]
}
```

External modules must be installed with `forge-link` before Forge can resolve them.

```text
forge-link <id> <path>
```

Links a module from a local directory.

```text
forge-link remove <id>
```

Removes an installed module.

```text
forge-link list
```

Lists all installed modules.

Installed modules are stored under:

```text
.forge/.dependencies/<id>/
```

---

## Resources

The `resources` property defines files and directories that should be copied to the output directory.

Each entry can be either a path or an object with an explicit source and destination.

For example:

```json
{
    "resources": [
        "license.txt",
        {
            "from": "packages/stdext/include",
            "to": "include"
        }
    ]
}
```

A plain path is copied while keeping its relative path.

When using `from` and `to`, you can choose exactly where the resource should be placed in the output directory.

Resources are copied with:

```text
forge copy
```

---

## Getting Help

Every Forge command has its own help page.

Use:

```text
forge help <command>
```

For example:

```text
forge help init
forge help build
forge help test
```

This shows the options and examples available for that command.

---

## version.txt

The `version.txt` file is located at the root of the project and is used by OrbitMVD to identify repository versions when creating tags and releases.

It must contain exactly three version numbers. The first two numbers can be used for any versioning scheme that fits the project. The third number is always used as the build number.

Forge is responsible for managing this file during builds. In incremental build mode, Forge automatically updates the build number and the build timestamp.

The version information is also exposed to the project as global preprocessor macros:

```text
FORGE_VERSION_FIRST
FORGE_VERSION_SECOND
FORGE_VERSION_BUILD
```

These macros contain the first, second, and third version numbers respectively, allowing project source code to access the current build version.
