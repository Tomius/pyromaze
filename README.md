Pyromaze
=============

Pyromaze is a universal OpenGL 4.2 game engine, with a simple game implemented in it.

It features voxel cone tracing global illumination. The game tries to show how much difference it makes in especially dark scenes. A screenshot with global global illumation:
![screenshot](screenshots/with_vct.png)
And a screenshot without:
![screenshot](screenshots/without_vct.png)

By the way, the game is about blowing stuff up (it is not finished yet):
![screenshot](screenshots/dynamite.png)
![screenshot](screenshots/explosion.png)
![screenshot](screenshots/after.png)

Windows:
--------
[Download Windows Executable](https://dl.dropboxusercontent.com/u/37129007/pyromaze.zip)


External dependencies:
----------------------
* C++11 compiler
* OpenGL 4.2 with ```GL_ARB_shader_image_load_store```
* CMake
* Python 2

Recommended build:
------------------
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
<Open IDE project file and build> or make -j4
```

Controls:
----------------------------------------------------
* WASD keys: position
* mouse: camera direction
* space: put down dynamite


----------------------
If you have any problem, please post in the issues tab or mail me at icyplusplus@gmail.com. Any feedback would be appreciated.

