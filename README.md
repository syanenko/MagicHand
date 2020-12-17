# MagicHand
Who:  Sergey Yanenko  
What: "Magic hand" - hand pose detection with OpenCV DNN module and applying shaders effects with OpenGL/GLUT/GLSL  
Why:  Exploring OpenCV, DNN, OpenGL, GLSL interoperability  
When: 12/2020  
How: OpenGL/GLUT/GLSL part based on Peter R. Elespuru code: https://github.com/elespuru/elespuru-cvglsl  
     OpenCV hand pose detection based on this example:      https://github.com/opencv/opencv/blob/master/samples/dnn/openpose.cpp  
  
Requirements:  
Hardware: NVidia RTX 2070 or higher  
Software: Windows 7 or higher 

How to run:  
Download DNN model: https://drive.google.com/file/d/1nms4M5plslVEG1RSzLQVydi8Df7KiaLs/view?usp=sharing  
and unzip to ...\MagicHand\model  
  
Download binaries (Win64): https://drive.google.com/file/d/1-nObusdSg19UWQVhH7SOuO40d_cJjhw4/view?usp=sharing  
and unzip to ...\MagicHand\bin  

Run: from ...\MagicHand folder: .\bin\MagicHand.exe  
  
Usage: MagicHand [--help|-h][-v [<file>]]  
-v: Save video to <file>, <file> will be 'MagicHand.avi' if not specified.  
  
PgUp/PgDn switch shaders  
'+'/'-' zooms in/out  
arrows pan  
'P'/'p' change number of passes  
'a' draw axes  
't', ',' decrease net threshold  
'T', '.' increase net threshold  
'd' show debug window  
'j' draw joints (debug window)  
'r' draw areas (debug window)  
'l' draw lines (debug window)  
'z' decrease area size (debug window)  
'x' increase area size (debug window)  
Enter sets default views  
Esc exit  
