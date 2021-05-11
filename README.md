# NPGR019
NPGR019 is a source code repository for [lab practices](https://cgg.mff.cuni.cz/~kahoun/labs.npgr019.php)
of the subject [Real-time graphics on GPU](https://is.cuni.cz/studium/predmety/index.php?do=predmet&kod=NPGR019).

## Build instructions
Examples from this suite require that you have at least OpenGL 3.3 installed.
The code should be portable to Linux but currently only the Windows MSVS 2017 Solution is provided.
Also, not all examples have been updated with with OpenGL 3.3 in mind as originally they were developed against 4.6 Core profile.
In order to successfully build and run the examples several prerequisite steps need to be taken:

1. Generate [glad](https://github.com/Dav1dde/glad) loader and put `glad.c` to the `src` directory, `glad` and `KHR` folders to the `include` directory.
2. Build or get [GLFW](https://www.glfw.org/) library (>= 3.3), put its `GLFW` folder into the `include` directory, `glfw3dll.lib` to the `lib` directory, and `glfw3.dll` to the `bin` directory.
3. Get [glm](https://github.com/g-truc/glm) library (>= 0.9.9) and put its `glm` folder to the `include` directory.
4. Get [stb image](https://github.com/nothings/stb) and put it to `include/stb` (it's a single header file).

The project is configured to look for additional includes in the `include` directory and link the GLFW static lib from the `lib` directory.
When in doubt, look at the root `.gitignore` file, it lists all the ignored libraries.
Binaries will be then copied to the `bin` directory which is also used as working directory.
If everything went well, you should be able to build the whole solution and run most of the examples.

Some examples need textures, you can supply either your own (and change data paths accordingly) or you can download
the [Terracotta tiles](https://3dtextures.me/2019/02/27/terracotta-tiles-002a/) pack I'm using.
Textures should be put to the `bin/data` directory where the code is expecting them.
Any texture set with diffuse map, normal map and specular map (I'm abusing supplied roughness map for that purpose) will do.

## Notes
All the basic projets (`01-Introduction`, `02-3dScene`, `03-DepthBuffer`, `04-Texturing`, `05-Instancing`, `06-Shading`, and `07-ShadowVolumes`)
are converted to OpenGL 3.3 because of compatibility with older embedded GPU's.
`05-Instancing` contains a modern SSBO based version in addition to 2 older ways of instancing.
The only difference from past years is that depth is mapped in the traditional
[OpenGL way of [-1, 1]](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_clip_control.txt)
which prevents better utilization of the depth buffer precision.
Project `08-Flocking` uses compute shaders which require OpenGL 4.3, though, so I'll keep the sources as they are.
