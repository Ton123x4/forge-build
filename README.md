# Forge Build

Forge is a build tool for C and C++ projects. It uses a simple `forge.json` manifest to describe projects and packages, while keeping the build process straightforward and easy to customize.

### Documentation

The [Forge Guide](docs/guide.md) covers the project structure, `forge.json` configuration, package types, environment-specific settings, external modules, resources, versioning, and build options.

### Building From Source

Forge can compile itself. If a working version of Forge is already available, it can be used to build the project normally.

For a fresh checkout, Forge also provides bootstrap scripts. The bootstrap process first builds a temporary version of Forge, then immediately runs that temporary version to perform the requested build.

For example:

```text
# Unix
./scripts/bootstrap.sh build

# Windows
.\scripts\bootstrap.cmd build
```

The `build` argument is passed to the temporary Forge instance after it has been compiled.

Building Forge requires Clang 19 or newer.

### License

Forge is distributed under the BSD 3-Clause License. See the license file for the full license text.
