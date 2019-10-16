#include <iostream>
#include <aruco/aruco.h>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;



int main(int argc,char **argv)
{
  try
  {
    VideoWriter vid;
    if (argc > 1) {
      vid = VideoWriter("cam_video.mp4", VideoWriter::fourcc('M', 'J', 'P', 'G'), 16.0, Size(640,480),true);
    }
    
    // Initialize camera
      VideoCapture cap(0);
      if (!cap.isOpened()) {
          cerr << "Couldn't open video capture device" << endl;
          return -1;
      }

      aruco::MarkerDetector MDetector;
      MDetector.setDetectionMode(aruco::DM_FAST);
      //MDetector.setDetectionMode(aruco::DM_VIDEO_FAST);
      //read the input image
      Mat inImage;
      Mat outImage;
      Mat freezeImage;
      Mat overlay;
      Mat kal1, kal2, kal_add, rotated, fliped_add;
      Point last_pt(-1,-1);
      Scalar lineColor = Scalar(0, 0, 0, 0);
      int lineThickness = 5;
     
      cap >> overlay; // for it to be the right dimensions
      kal1 = overlay.clone();
      kal2 = kal1.clone();
      kal_add = kal2.clone();
      overlay=0; // clear mat
      kal1 = 0;
      kal2 = 0;
      kal_add = 0;
      
      double snapshot_timer = 0;
      bool freeze = false;
      double old_fps=20;

      while(true) {
        int64 start = getTickCount();

        cap >> inImage;
        //cvtColor(inImage, outImage, COLOR_BGR2HSV);
        outImage = inImage;

        //Ok, let's detect
        MDetector.setDictionary("ARUCO_MIP_36h12");
        //detect markers and for each one, draw info and its boundaries in the image
        for(auto m:MDetector.detect(inImage)){
            cout<<m<<endl;
            cout<<m.Rvec<<" "<<m.Tvec<<endl;
            m.draw(inImage);

		    // ------------------ INVERT FRAME
            if (m.id == -1) {
				bitwise_not(inImage,outImage);
            }
		    // ------------------ CLEAR OVERLAY
            if (m.id == -1) {
				overlay = 0;
				last_pt.x = -1;
				last_pt.y = -1;
            }
		    // ------------------ EREASER
            if (m.id == 170) {
                lineColor = Scalar(0,0,0,0);
                lineThickness = 20;
            }
            // ------------------ CHANGE BRUSH COLOR
            if (m.id == 187) {
                lineColor = Scalar(0,0,0,0);
                lineThickness = 20;
            }            
		    // ------------------ DRAW WITH BRUSH
            if (m.id == 173) {
				if (last_pt.x != -1 && last_pt.y !=-1)
					cv:line(overlay, last_pt, m.getCenter(), lineColor, lineThickness);
				last_pt = m.getCenter();
				add(inImage,overlay,outImage);
            }
		    // ------------------ SNAPSHOT
            if (m.id == 21) {
                if (freeze == false) {
    				freeze = true;
	    			freezeImage = outImage.clone();				        
	    		}
		    }
	
		    // ------------------ KALEIDOSCOPE
            if (m.id == 160) {
                for (int j = 0; j < outImage.cols/2; ++j) {   
                    for(int i=0;i<outImage.rows/2;i++) {
                        if(i < j) 
                            kal1.at<Vec3b>(i,j)=outImage.at<Vec3b>(i, j);            
                        else
                            kal2.at<Vec3b>(i,j)=outImage.at<Vec3b>(i, j);       
                    }
                }
                kal_add = 0;
                addWeighted(kal_add,1,kal1,1,0,kal_add);
                flip(kal1,kal1,0);
        		addWeighted(kal_add,1,kal1,1,0,kal_add);
		        flip(kal1,kal1,1);
        		addWeighted(kal_add,1,kal1,1,0,kal_add);       		
        		flip(kal1,kal1,0);
        		addWeighted(kal_add,1,kal1,1,0,kal_add);        		
        		
        		addWeighted(kal_add,1,kal2,1,0,kal_add);
        		flip(kal2,kal2,0);
		        addWeighted(kal_add,1,kal2,1,0,kal_add);
        		flip(kal2,kal2,1);
		        addWeighted(kal_add,1,kal2,1,0,kal_add);
        		flip(kal2,kal2,0);
		        addWeighted(kal_add,1,kal2,1,0,kal_add);		        
		        outImage = kal_add;
		        
		    }

	
		    // ------------------ TIME LAPS
            if (m.id == 5) {

		    }


		    // ------------------ INVISIBILITY
            if (m.id == 6) {
                
            }


		    // ------------------ SECOND LOOP
            if (m.id == 7) {

		    }


		    // ------------------ HIGHLIGHT COLOUR
   	        if (m.id == 8) {

		    }
	

		    // ------------------ COLOR FILTERS
            if (m.id == 9) {

		    }


		    // ------------------ FACE WARP
            if (m.id == 10) {

		    }

        }
        double fps = (old_fps + getTickFrequency() / (getTickCount() - start))/2;
        
        putText(outImage, 
            to_string(fps),
            Point(10,10), // Coordinates
            FONT_HERSHEY_PLAIN, // Font
            1, // Scale. 2.0 = 2x bigger
            Scalar(255,255,255), // BGR Color
            1); // Line Thickness (Optional)
        //cout << "  - FPS : " << fps << endl;
        
		if (freeze == true) {
    	    imshow("in",freezeImage); // simply show the old outImage again
    	    if (argc > 1) vid.write(freezeImage);
			snapshot_timer++;
			if (snapshot_timer/fps > 5) {
				snapshot_timer = 0;
			    freeze = false;
			}
        }
        else {
            imshow("in",outImage);
            if (argc > 1) vid.write(outImage);
        }
        
        if (waitKey(5) == 27) break;

      }
      
      if (argc > 1) {
        vid.release();
      }
      destroyAllWindows();

  } catch (exception &ex)
  {
      cout<<"Exception :"<<ex.what()<<endl;
  }
  
}
