# Gengine (it stands for Game Engine)
A code-oriented game engine designed to be efficient, extensible, and simple to use. It is being developed with the goal of making a specific game, so some features may be missing or lacking. 

## Features
- Voxel engine (see [the 3D Voxel Engine repo](https://github.com/JuanDiegoMontoya/3D_Voxel_Engine) for more detail)
- Batch renderer
- ECS
- Entity scripting
- Entity transformation hierarchy
- Model loading
- Dynamic physics and collision API leveraging PhysX

## Graphics
- OpenGL 4.6-based renderer for minimal driver overhead
- Arbitrarily many animated and GPU-accelerated particles
- Flood fill voxel lighting with four color channels (red, green, blue, sunlight)
- HDR, tonemapping, and automatic exposure adjustment

## To Do
- Extend physics API with collision callbacks, filtering, etc.
- More graphical features like skeletal animation, better transparency, etc.
- ECS improvements based on gameplay needs

## Dependencies
- assimp
- cereal
- Dear ImGui
- entt
- FastNoise2
- glew
- glfw
- glm
- PhysX
- shaderc
- std
- zlib
