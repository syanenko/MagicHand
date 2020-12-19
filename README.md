## Magic hand
#### What  
Hand pose detection with OpenCV DNN module, applying shaders-based effects with OpenGL/GLUT/GLSL  
#### Who  
Sergey Yanenko  
#### Why  
Explore OpenCV, DNN, OpenGL interoperability, play with GLSL  
#### When 
12/2020
#### How  
* OpenGL/GLUT/GLSL part based on Peter R. Elespuru code: https://github.com/elespuru/elespuru-cvglsl  
* OpenCV hand pose detection based on this example:      https://github.com/opencv/opencv/blob/master/samples/dnn/openpose.cpp  
  
#### Requirements  
* *Hardware:* NVidia RTX 2070 or higher  
* *Software:* Windows 7 or higher 

#### Compiling from sources
1. Install NVIDIA CUDA Toolkit 10.2: https://developer.nvidia.com/cuda-10.2-download-archive  
1. Install cuDNN for CUDA  10.2: https://developer.nvidia.com/rdp/cudnn-download  
1. Download and build OpenCV: https://github.com/opencv/opencv with options WITH_CUDA=ON, OPENCV_DNN_CUDA=ON,  
   detailed instructions are here: https://www.pyimagesearch.com/2020/02/03/how-to-use-opencvs-dnn-module-with-nvidia-gpus-cuda-and-cudnn/  
1. Download and install FreeGlut: http://freeglut.sourceforge.net/  
1. Download and install Glew: http://glew.sourceforge.net/
1. Open MagicHand solution in Visual Studio 2017 or later, then open MagicHand project's properties.
1. Change appropriate include directories in section `C/C++/General/Additional Include Directories` and library directories in section `Linker/General/Additional Library Directories` according to locations of above dependencies.
1. Build desired configuration
1. Be sure that `freeglut.dll` and `glew32.dll` are in the path or place them in `<your_location>\MagicHand\bin`.
1. Run: `Ctrl+F5` or
```
cd <your_location>\MagicHand
.\bin\MagicHand.exe
```

#### Running from pre-built binaries

1. Download DNN model: https://drive.google.com/file/d/1nms4M5plslVEG1RSzLQVydi8Df7KiaLs/view?usp=sharing (or use getModels.sh)  
and unzip to `<your_location>\MagicHand\model`  
  
1. Download required DLLs: https://drive.google.com/file/d/115ty4cpAiCNTgYNGquw9NbH4c7g5JztJ/view?usp=sharing 
and unzip to `<your_location>\MagicHand\bin`  

1. Run
```
cd <your_location>\MagicHand
.\bin\MagicHand.exe
```

#### Help
```
Usage: MagicHand [-h][-v [`<file`>]]  
-h: Show this help  
-v: Save video to <file>, <file> will be 'MagicHand.avi' if not specified  
```

#### Control

| Keys | Actions |
|------|---------|
| PgUp/PgDn | switch shaders | 
| +/- | zooms in/out |
|Arrows|pan|
|P / p|change number of passes|
|a|draw axes|
|t or , | decrease net threshold|
|T or . | increase net threshold| 
|d|show debug window|
|j| draw joints (focus on debug window)|
|r| draw areas (focus on debug window)|
|l| draw lines (focus on debug window)|
|z| decrease area size (focus on debug window)|
|x| increase area size (focus on debug window)|
|Enter| set default views|  
|Esc|exit|

#### Technical details

Following uniforms are accessible in shaders:
```
uniform sampler2D u_texture;     // - Image, captured from camera
uniform vec2      u_resolution;  // - Image size
uniform float     u_time;        // - Shader running time in seconds
uniform vec2      u_mouse_move;  // - Free mouse move coordinates as retuned by glutPassiveMotionFunc()
uniform vec4      u_mouse_click; // - Mouse click position and state as retuned by glutMouseFunc()
uniform vec2      u_hand[22];    // - Hand points, detected by DNN module
```

Glut functions reference: https://www.opengl.org/resources/libraries/glut/spec3/node51.html  
                          https://www.opengl.org/resources/libraries/glut/spec3/node50.html  
  
General idea: https://www.learnopencv.com/hand-keypoint-detection-using-deep-learning-and-opencv/  
  
Detailed description: "Hand Keypoint Detection in Single Images using Multiview Bootstrapping" by Tomas, Simon Hanbyul Joo, Iain Matthews, Yaser Sheikh from Carnegie Mellon University: https://arxiv.org/pdf/1704.07809.pdf.
