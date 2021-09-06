#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream> 
#include <cstring>
#include <time.h>
using namespace std;
using namespace cv;
int main( int argc, char** argv ){
  // show help
  if(argc<2){
    cout<<
      " Usage: tracker <video_name>\n"
      " examples:\n"
      " example_tracking_kcf Bolt/img/%04d.jpg\n"
      " example_tracking_kcf faceocc2.webm\n"
      << endl;
    return 0;
  }
  // declares all required variables
  // Rect2d roi(415, 125, 65, 70); // for 1.avi
  Rect2d roi(487, 100, 105, 122); // for 2.avi
  Mat frame;

  // create a tracker object
  Ptr<Tracker> tracker = TrackerCSRT::create();
  // set input video
  std::string video = argv[1];
  VideoCapture cap(video);

  // for output .avi
  Size videoSize = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH), (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));
  VideoWriter writer;
  writer.open("output2.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, videoSize);

  // get bounding box
  cap >> frame;
  // roi=selectROI("tracker",frame);

  //quit if ROI was not selected
  if(roi.width==0 || roi.height==0)
    return 0;
  // initialize the tracker
  cout << "Init ROI Loc: " << roi << endl;
  tracker->init(frame,roi);

  std::string gttxt = argv[2];
  ifstream fin(gttxt);

  if (!fin) 
  { 
   cout<<"\n Couldn't find file \n"; 
   return 0; 
  } 

  // init Groundtruth first
  int gt_x=1, gt_y=2, gt_w=3, gt_h=4; 
  fin >> gt_x >> gt_y >> gt_w >> gt_h;
  // set curve img
  cv::Mat image = cv::Mat::zeros(500, 920, CV_8UC3);
	image.setTo(cv::Scalar(255, 255, 255));
  std::vector<cv::Point> points;

  // perform the tracking process
  clock_t start, end;
  double track_times = 0.0, max_iou = 0.0;
  int frame_cnt = 0, correc_cnt = 0, max_iou_loc = 0;
  printf("Start the tracking process, press ESC to quit.\n");
  for ( ;; ){
    // get frame from the video
    cap >> frame;
    if (frame.empty()){
			break;
	  }
    // stop the program if no more images
    if(frame.rows==0 || frame.cols==0)
      break;
    frame_cnt += 1;
    // update the tracking result
    start = clock();
    tracker->update(frame,roi);
    end = clock();
    track_times += end - start;

    // eval IOU
    fin >> gt_x >> gt_y >> gt_w >> gt_h;
    // cout<< gt_x << " " <<  gt_y << " " << gt_w << " " << gt_h <<endl;
    Rect2d roi_gt(gt_x, gt_y, gt_w, gt_h);
    Rect2d rectUnion = roi_gt | roi;
    Rect2d rectIntersec = roi_gt & roi;
    double IOU = rectIntersec.area() *1.0/ rectUnion.area();
	  // cout << "IOU=" << IOU << endl;
    if (IOU > 0.50)
      correc_cnt += 1;
    
    if (IOU > max_iou)
    {
      max_iou = IOU;
      max_iou_loc = frame_cnt;
    }

    // set curve point
    points.push_back(cv::Point(frame_cnt, 500 - IOU * 500));


    // draw the tracked object
    rectangle( frame, roi, Scalar( 255, 0, 0 ), 2, 1 );
    // rectangle( frame, roi_gt, Scalar( 255, 255, 0 ), 2, 1 );

    // show image with the tracked object
    if (frame_cnt % 30 == 0)
    {
      int font_face = cv::FONT_HERSHEY_COMPLEX;
      double font_scale = 0.7;
      int thickness = 2;
      string out_name = to_string(frame_cnt);
      cv::putText(frame, "IOU: " + to_string(IOU), cv::Point(roi.x, roi.y -15), font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 5, 0);
      imwrite("./res1/" + out_name + ".jpg" ,frame);
    }
    
    // writer.write(frame);
    // imshow("tracker",frame);
    //quit on ESC button
    if(waitKey(1)==27)break;
  }
  fin.close();

  // plot line chart
  cv::polylines(image, points, false, cv::Scalar(0, 0, 255), 1, 8, 0);
  // plot max IOU loc
  cv::circle(image, cv::Point(max_iou_loc, 500 - max_iou * 500), 5, cv::Scalar(0, 255, 255), 2, 8, 0);
  int font_face = cv::FONT_HERSHEY_COMPLEX;
  double font_scale = 1;
	int thickness = 2;
  cv::putText(image, to_string(max_iou), cv::Point(max_iou_loc, 500 - max_iou * 500), font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 5, 0);
  imwrite("iou_curve.jpg", image);

  cout << "Average computational time per frame: " << (track_times / CLOCKS_PER_SEC) / frame_cnt << " s" << endl;
  cout << "Accuracy rate (number of correct frames (IoU>50%) / total frame number): " <<  ((double)correc_cnt / (double)frame_cnt) * 100 << " %" << endl;
  
  return 0;
}