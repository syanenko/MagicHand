## Magic hand
#### What  
Hand pose detection with OpenCV DNN module, applying shaders-based effects with OpenGL/GLUT/GLSL  
#### Who  
Sergey Yanenko  
#### Why  
Exploring OpenCV, DNN, OpenGL, GLSL interoperability  
#### When 
12/2020
#### How  
OpenGL/GLUT/GLSL part based on Peter R. Elespuru code: https://github.com/elespuru/elespuru-cvglsl  
OpenCV hand pose detection based on this example:      https://github.com/opencv/opencv/blob/master/samples/dnn/openpose.cpp  
  
#### Requirements  
Hardware: NVidia RTX 2070 or higher  
Software: Windows 7 or higher 

#### Running  
Download DNN model: https://drive.google.com/file/d/1nms4M5plslVEG1RSzLQVydi8Df7KiaLs/view?usp=sharing  
and unzip to ...\MagicHand\model  
  
Download binaries (Win64): https://drive.google.com/file/d/1-nObusdSg19UWQVhH7SOuO40d_cJjhw4/view?usp=sharing  
and unzip to ...\MagicHand\bin  

```
cd <your_location>\MagicHand
.\bin\MagicHand.exe
```

#### Help
```
Usage: MagicHand [-h][-v [`<file`>]]  
-h: Show this help  
-v: Save video to `<file`>, `<file`> will be 'MagicHand.avi' if not specified  
```

#### Control

| Keys | Actions |
|------|---------|
| PgUp / PgDn | switch shaders | 
| '+' / '-' | zooms in/out |
|arrows|pan|
|'P'/'p'|change number of passes|
|'a'|draw axes|
|'t' or ',' | decrease net threshold|
|'T' or '.' | increase net threshold| 
|'d'|show debug window|
|'j'| draw joints (focus on debug window)|
|'r'| draw areas (focus on debug window)|
|'l'| draw lines (focus on debug window)|
|'z'| decrease area size (focus on debug window)|
|'x'| increase area size (focus on debug window)|
|Enter| set default views|  
|Esc|exit|
