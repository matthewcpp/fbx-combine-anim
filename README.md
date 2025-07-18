# fbx-combine-anim
Combine FBX files that were exported with a single animation each into a resulting file which contains all of them.
For Example you could may upload a model to mixamo and download a number of fbx files with your animations.  This tool can turn those multiple files into a single file containing your character with all animations.

### Setup
- Set `FBX_SDK_DIR` Environment variable to the root of your FBX SDK installation directory
- Alternatively pass `-DFBX_SDK_DIR=/path/to/fbx_sdk` to CMake

### Supported platforms

This has currently been tested on the following platforms with FBX SDK 2020.3.1:
- Windows x64
- MacOS