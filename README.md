# Forge Build

Forge is a build tool for C and C++ projects. It uses a simple `forge.json` manifest to describe projects and packages, while keeping the build process straightforward and easy to customize.

Forge is an experimental project created as a hobby. Although it is primarily developed for experimentation and personal use, it is currently used in some of my projects, including [Nougat](https://github.com/Ton123x4/Nougat) and [Dreiton](https://github.com/Ton123x4/Dreiton).

### Documentation

The [Forge Guide](docs/guide.md) covers the project structure, `forge.json` configuration, package types, environment-specific settings, external modules, resources, versioning, and build options.

### `stdext`

Forge currently uses the `stdext` library internally, and some projects that depend on Forge use it as well.

`stdext` is currently experimental and does not have documentation yet. Its API and implementation may evolve as the library develops.

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
