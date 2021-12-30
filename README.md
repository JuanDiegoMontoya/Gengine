# Gengine (Game Engine)
A code-oriented game engine designed to be efficient, extensible, and simple to use (not necessarily in that order). It is being developed mostly for fun, and maybe to eventually become a game. That means features will be lacking.

## Features
- Voxel engine (see [the Voxel Engine repo](https://github.com/JuanDiegoMontoya/3D_Voxel_Engine) for more detail)
- Batch renderer
- ECS using EnTT
- Entity scripting
- Entity transformation hierarchy
- Model and texture loading
- Dynamic physics and collision API leveraging PhysX
- Quake-style console, with support for variables and commands

## Graphics
- OpenGL 4.6-based renderer using modern overhead-reducing techniques
- GPU-driven particle system
- 4-channel flood fill voxel lighting
- HDR lighting, tonemapping, and automatic exposure adjustment
- FXAA
- Analytic fog
- Bloom inspired by CoD: AW
- Parallax-correct reflections

## To Do
- Extend physics API with collision callbacks, filtering, etc.
- More graphical features like skeletal animation, better transparency, etc.
- ECS improvements based on gameplay needs
- General code cleanup and better dependency management

## Dependencies
- assimp
- cereal
- Dear ImGui
- EnTT
- FastNoise2
- glad
- glfw
- glm
- PhysX
- shaderc
- stb_image
- zlib

## License
A suitable license is yet to be chosen.

## Gallery
### Animated (click to view videos)
10 million snow particles at 95 FPS. Many small moving things does not compress well!  
[<img src="https://i.imgur.com/htkPK3F.jpeg" width="100%">](https://gfycat.com/AptNiceEgg)  
Lighting transition  
[<img src="https://i.imgur.com/bWX7zkQ.png" width="100%">](https://gfycat.com/quarrelsomepeskyankole)  
10k individually animated objects  
[<img src="https://i.imgur.com/o3pqRLt.jpg" width="100%">](https://gfycat.com/vigilantmildflee)  
Physics + particles  
[<img src="https://i.imgur.com/UQarRWT.jpeg" width="100%">](https://gfycat.com/thesedishonestichthyostega)  


### Non-animated (click to see full resolution)
Foggy day  
[<img src="https://i.imgur.com/dabcd7d.jpeg" width="100%">](https://i.imgur.com/dabcd7d.jpeg)
Really foggy day  
[<img src="https://i.imgur.com/rblaEHo.jpeg" width="100%">](https://i.imgur.com/rblaEHo.jpeg)
Big render 1  
[<img src="https://i.imgur.com/u1rw3W8.jpg" width="100%">](https://i.imgur.com/u1rw3W8.jpg)  
Big render 2  
[<img src="https://i.imgur.com/UPmzarG.jpg" width="100%">](https://i.imgur.com/UPmzarG.jpg)  

