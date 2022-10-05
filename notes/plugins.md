# Plugins
## Layout
Quarks plugin system will be based on git submodules  
A plugin for the engine will be a separate git repository that can then be pulled into the engine by doing

This can be done using:
```
quark plugin add WindowsVista42/quark_common_3d
```
or by simply copying the plugin folder into the `plugins/` directory.

This will clone the repo at `https://github.com/WindowsVista42/quark_common_3d` into `./plugins/quark_common_3d` and add it as a git submodule.

Plugins can then subsequently be removed using:
```
quark plugin remove quark_common_3d
```

Note that the removal of a plugin only requires the name of the folder to be removed (internally we will remove the submodule if the plugin was a git repo).

Plugins can also be created with:
```
quark plugin create my_custom_plugin
```
These will not initially be created as git repos, but will be created with a predefined layout + predefined cmake files.

The defined folder layout will be similar to:
```
my_custom_plugin/
  models/
  shaders/
  sounds/
  src/
    api.hpp
    main.cpp
    my_custom_plugin.hpp
    my_custom_plugin.cpp
    .../
  textures/
  .../
```

## Building
All folders in the plugins directory will be built when building a quark project.

Any shared libs + the quark_loader executable will be copied to a build/current folder.
As well, any plugins in the plugin folder will have their contents lazily copied into the build/current folder as well.

Specific plugins can be "exported" using the command:
```
quark plugin export my_custom_plugin
```
which will copy the associated shared library and plugin folder to build/export

## Remarks
- Make sure there is a way to specify non-github repositories
- Use .cache as a caching folder, and make caches!

Full list of cli calls is:
```
// print plugin help
quark plugin

// Add https://github.com/ABC/XYZ as a plugin
quark plugin add ABC/XYZ

// Remove a plugin named XYZ
quark plugin remove XYZ

// Create and initialize a plugin named XYZ
quark plugin create XYZ

// Build and copy shared lib and plugins/XYZ to build/export
quark plugin export XYZ
```
