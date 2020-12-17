#ifndef __NNET__
#define __NNET__

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>

using namespace std;
using namespace cv;
using namespace cv::dnn;

class nnet
{
public:

  void start( const int _frameWidth, const int _frameHeight )
  {
    frameWidth  = _frameWidth;
    frameHeight = _frameHeight;

    net = readNetFromCaffe(protoFile, weightsFile);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

    float aspect = (float) frameWidth/ (float) frameHeight;
    inHeight = 368;
    inWidth = (int(aspect * (float)inHeight) << 3) >> 3;
    
    state = READY;
    run = true;
    worker = thread (work);
  };

  
  void push( Mat _frame, float _threshold )
  {
    if(state != PULLED)
      return;

    inpBlob = blobFromImage(_frame, 1.0 / 255, Size(inWidth, inHeight), Scalar(0, 0, 0), false, false);
    threshold = _threshold;
    state = PUSHED;
  };

  
  static void work()
  {
    while (run)
    {
      if(state == PUSHED)
      {
        net.setInput(inpBlob);
        Mat out = net.forward();
        int outHeight = out.size[2];
        int outWidth  = out.size[3];
        
        for (int n=0; n < NUM_POINTS; n++)
        {
            Mat probMap(outHeight, outWidth, CV_32F, out.ptr(0,n));
            resize(probMap, probMap, Size(frameWidth, frameHeight));

            Point maxLoc;
            double prob;
            minMaxLoc(probMap, 0, &prob, 0, &maxLoc);

            if (prob > threshold)
              points[n] = maxLoc;
            else
              points[n].x = -1, points[n].y = -1;
        }

        state = READY;
      }
      
      this_thread::sleep_for(1ms);
    }
  };

  void pull(Point _points[])
  {
    if(state != READY)
      return;

    for(int i=0; i<NUM_POINTS; ++i)
      _points[i] = points[i];

    state = PULLED;
  }

  void stop()
  {
    run = false;
    worker.join();
  }

  static const int NUM_POINTS = 22;

private:

  enum {
    PUSHED,
    READY,
    PULLED
  };
  static inline atomic<int> state;

  static inline Mat inpBlob;
  static inline float threshold;

  static inline const string protoFile = "model/pose_deploy.prototxt";
  static inline const string weightsFile = "model/pose_iter_102000.caffemodel";
  static inline Net net;

  static inline int frameWidth;
  static inline int frameHeight;
  static inline int inHeight;
  static inline int inWidth;

  static inline Point points[NUM_POINTS];

  static inline thread worker;
  static inline atomic<bool> run;
};

#endif