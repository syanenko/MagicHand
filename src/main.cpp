/*
 * Who:  Sergey Yanenko
 * What: "Magic hand" - hand pose detection with OpenCV DNN module and applying shaders effects with OpenGL/GLUT/GLSL
 * Why:  Exploring OpenCV, DNN, OpenGL, GLSL interoperability
 * When: 12/2020
 *
 * How: OpenGL/GLUT/GLSL part based on Peter R. Elespuru code: https://github.com/elespuru/elespuru-cvglsl
 *      OpenCV hand pose detection based on this example:      https://github.com/opencv/opencv/blob/master/samples/dnn/openpose.cpp
 *
 *  Usage: MagicHand [--help|-h][-v [<file>]]
 *  -v: Save video to <file>, <file> will be 'MagicHand.avi' if not specified.
 *
 *  PgUp/PgDn switch shaders
 *  '+'/'-' zooms in/out
 *  arrows pan
 *  'P'/'p' change number of passes
 *  'a' draw axes
 *  't', ',' decrease net threshold 
 *  'T', '.' increase net threshold
 *  'd' show debug window
 *  'j' draw joints (debug window)
 *  'r' draw areas (debug window)
 *  'l' draw lines (debug window)
 *  'z' decrease area size (debug window)
 *  'x' increase area size (debug window)
 *  Enter sets default views
 *  Esc exit
 */

#include <opencv2/core/core_c.h>
#include <opencv2/core/types_c.h>
#include <opencv2/highgui.hpp>
#include "core.h"
#include "nnet.hpp"

#include <string.h>
#include <stdio.h>

using namespace std;
using namespace cv;

string filters_path = "shaders/filters/";
string effects_path = "shaders/effects/";
#define FILTERS_PATH(S) (char*)((filters_path + string(S)).c_str())
#define EFFECTS_PATH(S) (char*)((effects_path + string(S)).c_str())

int camera_index = 0;      //  OpenCV camera index (0-based)
cv::VideoCapture cap;      //  OpenCV capture
int capture_fps = 60;      //  OpenCV capture frame rate
double video_fps = 30;     //  OpenCV video writer frame rate
unsigned int cvtex;        //  OpenCV texture
unsigned int imtex;        //  Image texture
 
int cap_width, cap_height; //  Capture dimensions
double cap_aspect = 1.0;   //  Capture aspect ratio

Mat frame;
Mat debug_frame;
VideoWriter video;
string video_file = "MagicHand.avi";
bool write_video = false;

nnet net;
float net_threshold = 0.4;
Point points[nnet::NUM_POINTS];
const int POSE_PAIRS[20][2] =
{
    {0,1}, {1,2}, {2,3}, {3,4},         // thumb
    {0,5}, {5,6}, {6,7}, {7,8},         // index
    {0,9}, {9,10}, {10,11}, {11,12},    // middle
    {0,13}, {13,14}, {14,15}, {15,16},  // ring
    {0,17}, {17,18}, {18,19}, {19,20}   // small
};
int nPairs = sizeof(POSE_PAIRS) / sizeof(POSE_PAIRS[0]);

float orig_x=0, orig_y=0;  //  Initial position   
int win_width, win_height; //  Window dimensions
double win_aspect = 1.0;   //  Window aspect ratio

// Controls
int mode=29;                //  Shader to use
bool draw_axes = false;    //  Draw crosshairs
double zoom=1;             //  Zoom factor
int passes=1;              //  Number of passes

// Gizmos
bool debug_window = false;
bool draw_joints  = true;
bool draw_areas   = true;
bool draw_numbers = true;
bool draw_lines   = true;
int  point_size   = 6;
int  area_size    = 22;

float frame_time = 0;
int shader_start_time = 0;

#define MAX_MODE 30
int shader[MAX_MODE] = {0}; //  Shader programs
char* text[] = {"No Shader","Copy","Sharpen","Blur","Erosion","Dilation","Laplacian Edge Detection","Prewitt Edge Detection","Sobel Edge Detection","Vignette", "Cross-Hatch","Colored-Cross-Hatch", "NightVision", "ThermalVision", "Posterized", "DreamVision","Pixelation","Toon","Billboard","Grayscale","Sepia","Negative","Halftone","HexPixelation","LineDrawing","LineDrawing2","Scanlines","Worhol","GeenLine","HandLight"};

int mouse_x = 0;
int mouse_y = 0;

//
//  OpenGL (GLUT) calls this routine to display the scene
//
void display()
{
  int k;
  //  Erase the window and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT);

  //  Set projection
  glLoadIdentity();
  Project(0, win_aspect, cap_aspect, 1.0);

  //  Set up for drawing
  glColor3f(1, 1, 1);
  glEnable(GL_TEXTURE_2D);

  //  Set shader
  if ( mode > 0 )
  {
    glUseProgram(shader[mode]);
    int loc = glGetUniformLocation(shader[mode], "u_texture");
    if (loc >= 0)
      glUniform1i(loc, 0);

    loc = glGetUniformLocation(shader[mode], "u_resolution");
    if (loc >= 0)
    {
      glUniform2f( loc, (GLfloat)win_width, (GLfloat)win_height );
    }
    
    loc = glGetUniformLocation(shader[mode], "u_time");
    if (loc >= 0)
    {
      glUniform1f( loc, (GLfloat)(glutGet(GLUT_ELAPSED_TIME) - shader_start_time) / (GLfloat)1000 );
    }

    loc = glGetUniformLocation(shader[mode], "u_mouse");
    if (loc >= 0)
    {
      glUniform2f( loc, (GLfloat)mouse_x, (GLfloat)(win_height - mouse_y) );
    }

    loc = glGetUniformLocation(shader[mode], "u_hand");
    if (loc >= 0)
    {
      net.pull(points);

      float arrp[net.NUM_POINTS * 2];
      for(int i = 0; i < net.NUM_POINTS; i++)
      {
        arrp[i << 1]       = points[i].x;
        arrp[(i << 1) + 1] = points[i].y;
      }

      glUniform2fv( loc, net.NUM_POINTS, arrp );
    }
  }

  //  Draw shader passes to a quad
  for (k=0; k<passes; k++)
  {
    //  Quad width
    float w = (k == 0) ? 1 : win_aspect;
    glPushMatrix();
    //  Initial pass - camera image
    if (k == 0)
    {
        glBindTexture(GL_TEXTURE_2D,cvtex);
        glScaled(zoom,zoom,1);
        glTranslated(orig_x,orig_y,0);
        glRotatef(180, 0, 0, 1);
    }
    //  Repeat pass - screen mage
    else
    {
        glBindTexture(GL_TEXTURE_2D, imtex);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, win_width,win_height, 0);
    }

    //  Set offsets
    if (mode > 0)
    {
        float dX = (k == 0) ? 1.0 / cap_width : zoom / win_width;
        float dY = (k == 0) ? 1.0 / cap_height : zoom / win_height;
        int id = glGetUniformLocation(shader[mode],"dX");
        if (id >= 0) glUniform1f(id,dX);
        id = glGetUniformLocation(shader[mode],"dY");
        if (id >= 0) glUniform1f(id,dY);
    }

    //  Redraw the texture
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-w, -1);
    glTexCoord2f(0, 1); glVertex2f(-w, +1);
    glTexCoord2f(1, 1); glVertex2f(+w, +1);
    glTexCoord2f(1, 0); glVertex2f(+w, -1);
    glEnd();
    glPopMatrix();
  }
  glDisable(GL_TEXTURE_2D);

  //  Shader off
  glUseProgram(0);

  //  Draw crosshairs
  if (draw_axes)
  {
    glBegin(GL_LINES);
    glVertex2f(-0.1, 0);
    glVertex2f(+0.1, 0);
    glVertex2f(0, -0.1);
    glVertex2f(0, +0.1);
    glEnd();
  }

  // Display parameters
  glWindowPos2i(5, win_height - 15);
  // Print("Zoom=%.1f Offset=%f,%f Shader=%s Passes=%d", zoom, orig_x, orig_y, text[mode], passes);
  Print("Shader: %s", text[mode]);

  //  Render the scene and make it visible
  ErrCheck("display");

  // Save video
  if (write_video)
  {
    glReadPixels (0, 0, cap_width, cap_height, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
    flip(frame, frame, 0);
    video.write( frame );
  }

  // glFlush();
  glutSwapBuffers();
}

//
//  GLUT calls this routine when an arrow key is pressed
//
void special_keys(int key, int x, int y)
{
  switch(key)
  {
    case GLUT_KEY_RIGHT:     orig_x -= 0.03/zoom; break;
    case GLUT_KEY_LEFT:      orig_x += 0.03/zoom; break;
    case GLUT_KEY_UP:        orig_y -= 0.03/zoom; break;
    case GLUT_KEY_DOWN:      orig_y += 0.03/zoom; break;
    case GLUT_KEY_PAGE_DOWN: ++mode %= MAX_MODE;
                             shader_start_time = glutGet(GLUT_ELAPSED_TIME);
                                                 break;
    case GLUT_KEY_PAGE_UP:   mode = (mode + MAX_MODE-1) % MAX_MODE;
                             shader_start_time = glutGet(GLUT_ELAPSED_TIME);
                                                 break;
  }

  if (zoom < 1)
  {
    zoom = 1;
    orig_x = orig_y = 0;
  }
  
  glutPostRedisplay();
}

//
// Quit
// 
void quit()
{
  net.stop();
  cap.release();
  if (write_video)
    video.release();
  
  exit(0);
}

//
//  GLUT calls this routine when a key is pressed
//
void common_keys(unsigned char key, int x,int y)
{
  switch(key)
  {
    case '-': zoom /= 1.1;                   break;
    case '+': zoom *= 1.1;                   break;
    case 'p': passes++;                      break;
    case 'P': if(passes > 1) passes--;       break;
    case 'a': draw_axes = !draw_axes;        break;
    case 13:  orig_x = orig_y = 0; zoom = 1; break;
    case 'n': draw_numbers = !draw_numbers;  break;
    case 'j': draw_joints  = !draw_joints;   break;
    case 'l': draw_lines   = !draw_lines;    break;
    case 'r': draw_areas   = !draw_areas;    break;
    case 'd': debug_window = !debug_window;
              if( !debug_window ) cv::destroyAllWindows(); break;
    case 'z': area_size > point_size ? area_size-- : 0;    break;
    case 'x': area_size++; break; 
    case '.':
    case 'T': net_threshold += 0.01;
              // cout << "Threshold: " << net_threshold << endl;
              break;
    case ',':
    case 't': net_threshold -= 0.01;
              net_threshold < 0 ? net_threshold = 0 : 0;
              // cout << "Threshold: " << net_threshold << endl;
              break;

    case 27:  quit();
  }

  if (zoom < 1)
  {
    zoom = 1;
    orig_x = orig_y = 0;
  }

  glutPostRedisplay();
}

//
// GLUT calls this routine when the window is resized
//
void reshape(int width, int height)
{
   //  Ratio of the width to the height of the window
   win_aspect = (height > 0) ? (double)width / height : 1;
   //  Set the viewport to the entire window
   glViewport(0, 0, width, height);
   //  Set image dimensions
   win_width  = width; 
   win_height = height;
}

//
//  Capture
//
void capture(int k)
{
  frame_time = (double) cv::getTickCount();

  cap >> frame;
  net.push(frame, net_threshold);

  // Debug with gizmos
  if( debug_window )
  {
    frame.copyTo(debug_frame);
    
    // Draw lines
    if (draw_lines)
    {
      for (int n = 0; n < nPairs; n++)
      {
        Point2f partA = points[POSE_PAIRS[n][0]];
        Point2f partB = points[POSE_PAIRS[n][1]];
        if (partA.x<=0 || partA.y<=0 || partB.x<=0 || partB.y<=0)
          continue;
        line(debug_frame, partA, partB, Scalar(0,255,255), 8);
      }
    }
         
    // Draw numbers / points
    for (int n = 0; n < nnet::NUM_POINTS; n++)
    {
      if((points[n].x < 0) ||  (points[n].y < 0))
        continue;

      if (draw_joints)
        circle(debug_frame, points[n], point_size, Scalar(0, 180, 0), -1);

      if (draw_areas)
        circle(debug_frame, points[n], area_size, Scalar(0, 180, 0), 1);

      if (draw_numbers)
        cv::putText(debug_frame, cv::format("%d", n), points[n], cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
    }
  
    frame_time = ((float)cv::getTickCount() - frame_time) / cv::getTickFrequency();
    cv::putText(debug_frame, cv::format("FT: %5.3f, TO: %4.2f", frame_time, net_threshold), cv::Point(20, 20), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);

    imshow("Magic hand - DEBUG", debug_frame);
  }

   
  //  Copy image to texture
  glBindTexture(GL_TEXTURE_2D, cvtex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.size().width, frame.size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
 
  ErrCheck("Capture");

  glutTimerFunc(1, capture, 0);
  glutPostRedisplay();
} 

//
// Print usage
//
void printUsage()
{
  printf("Usage: MagicHand [--help|-h][-v [<file>]]\n-v: Save video to <file>, <file> will be 'MagicHand.avi' if not specified.\n\
\n\
PgUp/PgDn switch shaders\n\
'+'/'-' zooms in/out\n\
arrows pan\n\
'P'/'p' change number of passes\n\
'a' draw axes\n\
't', ',' decrease net threshold\n\
'T', '.' increase net threshold\n\
'd' show debug window\n\
'j' draw joints (debug window)\n\
'r' draw areas (debug window)\n\
'l' draw lines (debug window)\n\
'z' decrease area size (debug window)\n\
'x' increase area size (debug window)\n\
Enter sets default views\n\
Esc exit");
}

//
// Process comand line parameter
//
void procCommandLLine(int argc, char* argv[])
{
  if(argc == 2)
  {
    if(!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
    {
      printUsage();
      exit(0);
    } else if( !strcmp(argv[1], "-v") )
    {
      write_video = true;
    }
  } else if(argc == 3)
  {
    if( !strcmp(argv[1], "-v") )
    {
      video_file = argv[2];
      write_video = true;
    } else
    {
      printUsage();
      exit(0);
    }
  }
}

//
// Mouse
//
void mouse(int x, int y)
{
  mouse_x = x;
  mouse_y = y;
}

//
// Main
//
int main(int argc,char* argv[])
{
  procCommandLLine(argc, argv);

  //  Initialize OpenCV capture
  cap.set(CAP_PROP_FPS, capture_fps);
  cap.open(camera_index, CAP_DSHOW);
  win_width  = cap_width  = cap.get(CAP_PROP_FRAME_WIDTH);
  win_height = cap_height = cap.get(CAP_PROP_FRAME_HEIGHT);
  cap_aspect = (double)cap_width / (double)cap_height;
   
  if (!cap.isOpened())
  {
      Fatal("Could not initialize OpenCV\n");
  }
  
  if (write_video)
    video.open(video_file, VideoWriter::fourcc('M','J','P','G'), video_fps, Size(cap_width, cap_height) );

  //  Initialize GLUT
  glutInit(&argc, argv);

  //  Request double buffered, true color window with Z buffering at 600x600
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(cap_width, cap_height);
  glutCreateWindow("CVGLSL");
  
  #ifdef USEGLEW
  //  Initialize GLEW
  if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
  if (!GLEW_VERSION_2_0) Fatal("OpenGL 2.0 not supported\n");
  #endif

  //  Set callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(common_keys);
  glutSpecialFunc(special_keys);
  glutPassiveMotionFunc(mouse);

  //  Texture to store image
  glGenTextures(1, &cvtex);
  glBindTexture(GL_TEXTURE_2D, cvtex);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
   
  //  Texture to post-process image
  glGenTextures(1, &imtex);
  glBindTexture(GL_TEXTURE_2D, imtex);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  capture(0); // Start capture
   
  shader[1]  = CreateShaderProg(NULL,  FILTERS_PATH("copy.frag"));
  shader[2]  = CreateShaderProg(NULL,  FILTERS_PATH("sharpen.frag"));
  shader[3]  = CreateShaderProg(NULL,  FILTERS_PATH("blur.frag"));
  shader[4]  = CreateShaderProg(NULL,  FILTERS_PATH("erosion.frag"));
  shader[5]  = CreateShaderProg(NULL,  FILTERS_PATH("dilation.frag"));
  shader[6]  = CreateShaderProg(NULL,  FILTERS_PATH("laplacian.frag"));
  shader[7]  = CreateShaderProg(NULL,  FILTERS_PATH("prewitt.frag"));
  shader[8]  = CreateShaderProg(NULL,  FILTERS_PATH("sobel.frag"));
  shader[9]  = CreateShaderProg(NULL,  FILTERS_PATH("vignette.frag"));
  shader[10] = CreateShaderProg(NULL,  FILTERS_PATH("crosshatch.frag"));
  shader[11] = CreateShaderProg(NULL,  FILTERS_PATH("crosshatch2.frag"));
  shader[12] = CreateShaderProg(NULL,  FILTERS_PATH("nvscope.frag"));
  shader[13] = CreateShaderProg(NULL,  FILTERS_PATH("thermal.frag"));
  shader[14] = CreateShaderProg(NULL,  FILTERS_PATH("posterize.frag"));
  shader[15] = CreateShaderProg(NULL,  FILTERS_PATH("dream.frag"));
  shader[16] = CreateShaderProg(NULL,  FILTERS_PATH("pixelation.frag"));
  shader[17] = CreateShaderProg(FILTERS_PATH("toon.vert"),      FILTERS_PATH("toon.frag"));
  shader[18] = CreateShaderProg(FILTERS_PATH("billboard.vert"), FILTERS_PATH("billboard.frag"));
  shader[19] = CreateShaderProg(FILTERS_PATH("grayscale.vert"), FILTERS_PATH("grayscale.frag"));
  shader[20] = CreateShaderProg(FILTERS_PATH("sepia.vert"),     FILTERS_PATH("sepia.frag"));
  shader[21] = CreateShaderProg(FILTERS_PATH("invert.vert"),    FILTERS_PATH("invert.frag"));
  shader[22] = CreateShaderProg(NULL, FILTERS_PATH("halftone.frag"));
  shader[23] = CreateShaderProg(NULL, FILTERS_PATH("hexpix.frag"));
  shader[24] = CreateShaderProg(FILTERS_PATH("line.vert"),      FILTERS_PATH("line.frag"));
  shader[25] = CreateShaderProg(FILTERS_PATH("line2.vert"),     FILTERS_PATH("line2.frag"));
  shader[26] = CreateShaderProg(FILTERS_PATH("scanlines.vert"), FILTERS_PATH("scanlines.frag"));
  shader[27] = CreateShaderProg(FILTERS_PATH("worhol.vert"),    FILTERS_PATH("worhol.frag"));
  shader[28] = CreateShaderProg(NULL, EFFECTS_PATH("green_line.frag"));
  shader[29] = CreateShaderProg(NULL, EFFECTS_PATH("hand_light.frag"));
  
  ErrCheck("init");

  net.start((const int) cap.get(CAP_PROP_FRAME_WIDTH),
            (const int) cap.get(CAP_PROP_FRAME_HEIGHT));
  
  shader_start_time = glutGet(GLUT_ELAPSED_TIME);

  glutMainLoop();
  quit();

  return 0;
}
