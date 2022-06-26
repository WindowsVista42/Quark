# Quark
## Required Tools
vulkan-sdk >=1.2.x  
ninja  
clang >=10.0  
cmake >=3.16  
lld  
python3 >=3.8  
make  
glsLangValidator  
git  
git-lfs  

## Building and Running
Clone using  
```git clone git@github.com:WindowsVista42/Quark.git --recursive```

Setup build environment using  
```make b```

Compile and run  
```make d```

## Future Directory Structure
```
content/     - Current Game Assets
  game/      - Put Assets for Target Game Here (if making a mod)
  my_mod/    - Put Assets for Target Mod Here (if making a mod)
  */         - Put Other Required Assets Here
examples/    - Example Mods and Games
my_game/     - Working Directory for Current Game
publish/     - Most Recent Publish
quark/       - Engine/API
  core/      - Core Module
  engine/    - Engine Module
  physics/   - 3D Physics Module
  platform/  - Platform Module
scripts/     - Build Scripts
target/      - Build Target
  debug/     - Debug Build Files
  release/   - Release Build Files
third_party/ - Third Party Code and Dependencies
wiki/        - Wiki
./           - Current Directory
```
