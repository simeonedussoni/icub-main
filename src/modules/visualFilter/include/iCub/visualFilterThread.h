// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#ifndef _VISUAL_FILTER_THREAD_H_
#define _VISUAL_FILTER_THREAD_H_

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <iostream>


class visualFilterThread : public yarp::os::Thread
{
private:

   /* class variables */

   int      x, y;
   yarp::sig::PixelRgb rgbPixel;
   yarp::sig::ImageOf<yarp::sig::PixelRgb> *image;
  	    
   /* thread parameters: they are pointers so that they refer to the original variables in myModule */

   yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *imagePortIn;
   yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *imagePortOut;   
   int *thresholdValue; 
   

public:

    bool interrupted;

   /* class methods */

   visualFilterThread(yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *imageIn,  yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > *imageOut, int *threshold );
   bool threadInit();     
   void threadRelease();
   void run(); 
};

#endif  //_VISUAL_FILTER_THREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
