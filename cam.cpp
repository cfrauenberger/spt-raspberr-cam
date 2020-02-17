#include <iostream>
#include <aruco/aruco.h>
#include <opencv2/highgui.hpp>
#include <stdlib.h>

using namespace cv;
using namespace std;



int main(int argc,char **argv)
{
  int status = system("./sudo python3 set_np.py white");
	
  try
  {
    VideoWriter vid;
    bool record_vid = false;
    int cam_index = 0;
    string param_file = "calibration.yml";
    string smiley_file = "Smiley.png";

    // switches 
    for (int i = 1; i<argc; i++) {
      string arg(argv[i]);
      if (arg == "-v") record_vid = true;
      if (arg.find("-c=") != string::npos) 
        cam_index = stoi(arg.substr(arg.find("-c=")+3));
      if (arg.find("-p=") != string::npos)
        param_file = arg.substr(arg.find("-p=")+3);
      if (arg.find("-s=") != string::npos)
        smiley_file = arg.substr(arg.find("-s=")+3);
    }
    cout << "Camera: " << cam_index << endl;
    cout << "Parameter file: " << param_file << endl;
    
    // Initialize camera
    VideoCapture cap(cam_index);
    if (!cap.isOpened()) {
      cerr << "Couldn't open video capture device" << endl;
      return -1;
    }

    int cam_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int cam_height = cap.get(CAP_PROP_FRAME_HEIGHT);
    if (record_vid) 
      vid = VideoWriter("cam_video.mp4", VideoWriter::fourcc('M', 'J', 'P', 'G'), 16.0, Size(cam_width,cam_height),true);

    // ARUCO Marker detection
    aruco::MarkerDetector MDetector;
    MDetector.setDictionary("ARUCO_MIP_36h12");
    MDetector.setDetectionMode(aruco::DM_FAST);
    //MDetector.setDetectionMode(aruco::DM_VIDEO_FAST);

    // camera calibration for pose estimation
    aruco::CameraParameters camera;
    camera.readFromXMLFile(param_file);
    
    // read smiley file 
    Mat smiley = imread(smiley_file, CV_LOAD_IMAGE_COLOR);
    
    // for saving pngs
    vector<int> compression_params;
    compression_params.push_back(IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    
    //read the input image
    Mat inImage;
    Mat outImage;
    Mat freezeImage;
    Mat overlay;
    Mat kal1, kal2, kal_add, rotated, fliped_add;
    Point last_pt(-1,-1);
    Scalar lineColor = Scalar(rand() % 255, rand() % 255,rand() % 255);
    int lineThickness = 5;
     
    cap >> overlay; // for it to be the right dimensions
    kal1 = overlay.clone();
    kal2 = kal1.clone();
    kal_add = kal2.clone();
    overlay=0; // clear mat
    kal1 = 0;
    kal2 = 0;
    kal_add = 0;
      
    double fps = 20;
    double snapshot_timer = 0;
    double stroke_timer = 0;
    bool freeze = false;
    double old_fps=20;
    bool new_color_set = false;
    bool new_color_present = false;
    
    while(true) {
      int64 start = getTickCount();

      cap >> inImage;
      //cvtColor(inImage, outImage, COLOR_BGR2HSV);
      add(inImage,overlay,outImage);

      new_color_present = false;

      //detect markers and for each one, draw info and its boundaries in the image
      for(auto m:MDetector.detect(inImage,camera,0.039)){
        cout<<m.id<<endl;
        //aruco::CvDrawingUtils::draw3dAxis(outImage,m,camera);
        //m.draw(outImage);

		    // ------------------ INVERT FRAME
        if (m.id == 113) {
          bitwise_not(outImage,outImage);
        }
		    // ------------------ BLACK AND WHITE
        if (m.id == 94) {
          cvtColor(outImage, outImage, COLOR_BGR2GRAY);
          threshold( outImage, outImage, 140, 255,THRESH_BINARY );
		    }        
		    // ------------------ CLEAR OVERLAY
        if (m.id == -1) {
          overlay = 0;
          last_pt.x = -1;
          last_pt.y = -1;
        }
		    // ------------------ EREASER
        if (m.id == 170) {
          circle(overlay, m.getCenter(), 50, Scalar(0,0,0,0), FILLED);
        }
        // ------------------ CHANGE BRUSH COLOR
        if (m.id == 187) {
          new_color_present = true;
          if (!new_color_set) {
            lineColor = Scalar(rand() % 255, rand() % 255,rand() % 255);
            lineThickness = 5;
            cout << lineColor << endl;
            new_color_set = true;
          }
        }            
		    // ------------------ DRAW WITH BRUSH
        if (m.id == 173) {
          if (last_pt.x != -1 && last_pt.y !=-1 && stroke_timer/fps < 1)
            line(overlay, last_pt, m.getCenter(), lineColor, lineThickness);
          last_pt = m.getCenter();
          stroke_timer = 0;
        }
		    // ------------------ SNAPSHOT
        if (m.id == 239) {
          if (freeze == false) {
            freeze = true;
            freezeImage = outImage.clone();			
            // get a filename with a time stamp
            char buff[70];
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_c);
            strftime(buff, sizeof buff, "%F-%T", &now_tm);
            // write the image file out
            string filename(buff, strlen(buff));
            filename = filename + ".png";
            cout << "Outputfile:" << filename << endl;
            imwrite(filename, freezeImage, compression_params);
            // draw a yellow frame 
            line(freezeImage, Point(0,0), Point(cam_width,0), Scalar(0,250,250), 50);
            line(freezeImage, Point(cam_width,0), Point(cam_width,cam_height), Scalar(0,250,250), 50);
            line(freezeImage, Point(cam_width,cam_height), Point(0,cam_height), Scalar(0,250,250), 50);
            line(freezeImage, Point(0,cam_height), Point(0,0), Scalar(0,250,250), 50);
          }
        }
		    // ------------------ ROTATE
        if (m.id == 193) {
          Mat matRotation = getRotationMatrix2D( 
            Point(cam_width/2, cam_height/2), 
            m.Rvec.at<float>(1, 0)*180/3.1415, 1 );
          Mat imgRotated;
          warpAffine( outImage, outImage, matRotation, outImage.size() );
		    }
		    // ------------------ FLIP
        if (m.id == 102) {
          flip(outImage, outImage, 0);
		    }
		    // ------------------ KALEIDOSCOPE
        if (m.id == 110) {
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
		    // ------------------ SMILEY
        if (m.id == 97) {
          Point center = m.getCenter();
          int radius = floor(m.getRadius());
          Mat res_smiley;
          resize(smiley, res_smiley, Size(2*radius,2*radius), 0,0,INTER_LINEAR);
          res_smiley.copyTo(outImage(Rect(center.x-radius, center.y-radius, res_smiley.cols, res_smiley.rows)));
        }
		    // ------------------ SECOND LOOP
        if (m.id == 7) {

		    }
		    // ------------------ HIGHLIGHT COLOUR
        if (m.id == 8) {

		    }
		    // ------------------ FACE WARP
        if (m.id == 10) {

		    }
      }
        
        
      if (!new_color_present) new_color_set = false;
        
      fps = (old_fps + getTickFrequency() / (getTickCount() - start))/2;
      stroke_timer++;

      cvNamedWindow("in", CV_WINDOW_NORMAL);
      //cvSetWindowProperty("in", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
        
      putText(outImage, 
        to_string(fps),
        Point(10,10), // Coordinates
        FONT_HERSHEY_PLAIN, // Font
        1, // Scale. 2.0 = 2x bigger
        Scalar(255,255,255), // BGR Color
        1); // Line Thickness (Optional)
  
        
      if (freeze == true) {
        imshow("in",freezeImage); // simply show the old outImage again
   	    if (record_vid) vid.write(freezeImage);
		   	snapshot_timer++;
        if (snapshot_timer/fps > 5) {
          snapshot_timer = 0;
          freeze = false;
			  }
      }
      else {
        imshow("in",outImage);
        if (record_vid) vid.write(outImage);
      }
        
      if (waitKey(5) == 27) break;
    }
      
    if (record_vid) vid.release();
    destroyAllWindows();

  } catch (exception &ex)
  {
      cout<<"Exception :"<<ex.what()<<endl;
  }
  
}
