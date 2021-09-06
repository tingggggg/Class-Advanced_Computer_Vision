#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
#include <time.h>


int main(){

    char filein[60], fileout[20];

    printf("Input image filename(*.bmp): ");
    cin >> filein;

    Mat img = cv::imread(filein);
    Mat img_binary, gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    if (img.empty()){
        cout << "empty" << endl;
        system("pause");
        return -1;
    }

    // binary
    threshold(gray, img_binary, 230, 255.0, CV_THRESH_BINARY);

    Mat k = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
    morphologyEx(img_binary, img_binary, MORPH_OPEN, k);
    morphologyEx(img_binary, img_binary, MORPH_CLOSE, k);

    // timer
    clock_t begin, end, begin_m, end_m, begin_p, end_p;
    double cost, cost_morphology = 0.0, cost_plot = 0.0;

    bitwise_not(img_binary, img_binary);
    Mat labels = Mat::zeros(img.size(), CV_32S);
    Mat stats, centroids;

    begin = clock();
    int num_labels = connectedComponentsWithStats(img_binary, labels, stats, centroids, 4, 4);
    end = clock();

    cout << " ********************* hand *********************** " << endl;
    vector<Vec3b> colors(num_labels);
    colors[0] = Vec3b(0, 0, 0);
    for (int i = 1; i < num_labels; i++) {
        colors[i] = Vec3b(255, 255, 255);
    }
    // render result
    Mat dst = Mat::zeros(img.size(), img.type());
    int w = img.cols;
    int h = img.rows;
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int label = labels.at<int>(row, col);
            if (label == 0) continue;
            dst.at<Vec3b>(row, col) = colors[label];
        }
    }
    Mat final_result;
    final_result = dst.clone();
    
    begin_m = clock();
    k = getStructuringElement(MORPH_RECT, Size(25, 25), Point(-1, -1));
    morphologyEx(dst, dst, MORPH_OPEN, k);
    k = getStructuringElement(MORPH_ELLIPSE, Size(25, 25), Point(-1, -1));
    morphologyEx(dst, dst, MORPH_OPEN, k);
    end_m = clock();
    cost_morphology = (double)(end_m - begin_m)/CLOCKS_PER_SEC;
     
    // plot bbox centroid
    for (int i = 1; i < num_labels; i++) {
        Vec2d pt = centroids.at<Vec2d>(i, 0);
        int x = stats.at<int>(i, CC_STAT_LEFT);
        int y = stats.at<int>(i, CC_STAT_TOP);
        int width = stats.at<int>(i, CC_STAT_WIDTH);
        int height = stats.at<int>(i, CC_STAT_HEIGHT);
        int area = stats.at<int>(i, CC_STAT_AREA);
        printf("area : %d, centroid point(%.2f, %.2f)\n", area, pt[0], pt[1]);
        printf("the length of longest axis : %d, orientation : %s \n", width > height ? width : height, width > height ? "horizontal" : "vertical");

        begin_p = clock();
        char buff[10] = "";
        sprintf(buff, "%d", (int)pt[0]); 
        putText(final_result, buff, Point(pt[0]-40, pt[1]), FONT_HERSHEY_COMPLEX, 0.6, Scalar(0, 0, 255), 1, 1, 0);
        sprintf(buff, "%d", (int)pt[1]);
        putText(final_result, buff, Point(pt[0]+2, pt[1]), FONT_HERSHEY_COMPLEX, 0.6, Scalar(0, 0, 255), 1, 1, 0);
        sprintf(buff, "%d", area);
        putText(final_result, buff, Point(pt[0]-40, pt[1]+30), FONT_HERSHEY_COMPLEX, 0.6, Scalar(0, 0, 255), 1, 1, 0);
        circle(final_result, Point(pt[0], pt[1]), 2, Scalar(0, 0, 255), -1, 8, 0);
        rectangle(final_result, Rect(x, y, width, height), Scalar(0, 0, 255), 1, 8, 0);
        end_p = clock();
        cost_plot += (double)(end_p - begin_p)/CLOCKS_PER_SEC;
    }
    cost = (double)(end - begin)/CLOCKS_PER_SEC;
    cout << "connected components times of hand: " << cost << " secs" << endl;
    cout << "morphology times: " << cost_morphology << " secs" << endl;
    cout << " ************************************************** " << endl;
    // find out only fingers
    Mat fingers;
    fingers = img_binary.clone();
    Mat_<Vec3b>::iterator it = fingers.begin<Vec3b>();	
	Mat_<Vec3b>::iterator itend = fingers.end<Vec3b>();	
	Mat_<Vec3b>::iterator it1 = img_binary.begin<Vec3b>(); 
	Mat_<Vec3b>::iterator it2 = dst.begin<Vec3b>();	
	// original - only hand
	for (; it != itend; it++)
	{
		(*it)[0] = ((*it1)[0] - (*it2)[0]);
		(*it)[1] = ((*it1)[1] - (*it2)[1]);
		(*it)[2] = ((*it1)[2] - (*it2)[2]);
		it1++;
		it2++;
	}
    // do only fingers connected component
    Mat labels_f = Mat::zeros(img.size(), CV_32S);
    Mat stats_f, centroids_f;
    begin = clock();
    int num_labels_f = connectedComponentsWithStats(fingers, labels_f, stats_f, centroids_f, 4, 4);
    end = clock();
    vector<Vec3b> colors_f(num_labels_f);
    colors_f[0] = Vec3b(0, 0, 0);
    for (int i = 1; i < num_labels_f; i++) {
        colors_f[i] = Vec3b(255, 255, 255);
    }
    cout << " ******************** finger ********************** " << endl;
    // plot bbox centroid for fingers
    for (int i = 1; i < num_labels_f; i++) {
        Vec2d pt = centroids_f.at<Vec2d>(i, 0);
        int x = stats_f.at<int>(i, CC_STAT_LEFT);
        int y = stats_f.at<int>(i, CC_STAT_TOP);
        int width = stats_f.at<int>(i, CC_STAT_WIDTH);
        int height = stats_f.at<int>(i, CC_STAT_HEIGHT);
        int area = stats_f.at<int>(i, CC_STAT_AREA);
        if(area > 300){
            printf("area : %d, centroid point(%.2f, %.2f)\n", area, pt[0], pt[1]);
            printf("the length of longest axis : %d, orientation : %s \n", width > height ? width : height, width > height ? "horizontal" : "vertical");
            
            begin_p = clock();
            circle(final_result, Point(pt[0], pt[1]), 2, Scalar(255, 0, 0), -1, 8, 0);
            rectangle(final_result, Rect(x, y, width, height), Scalar(255, 0, 0), 1, 8, 0);
            end_p = clock();
            cost_plot += (double)(end_p - begin_p)/CLOCKS_PER_SEC;
            cout << endl;
        }
        
    }
    cost = (double)(end - begin)/CLOCKS_PER_SEC;
    cout << "connected components times of finger: " << cost << " secs" << endl;
    cout << "plot all bbox times: " << cost_plot << " secs" << endl;
    cout << " ************************************************** " << endl;
    // find number of fingers in each hand
    for (int i = 1; i < num_labels; i++) {
        int nfingers = 0;
        int x = stats.at<int>(i, CC_STAT_LEFT);
        int y = stats.at<int>(i, CC_STAT_TOP);
        int width = stats.at<int>(i, CC_STAT_WIDTH);
        int height = stats.at<int>(i, CC_STAT_HEIGHT);

        for (int j = 1; j < num_labels_f; j++) {
            Vec2d pt = centroids_f.at<Vec2d>(j, 0);
            int area = stats_f.at<int>(j, CC_STAT_AREA);
            if(area > 300){
                if(pt[0] < x + width && \
                   pt[0] > x && \
                   pt[1] < y + height && \
                   pt[1] > y)
                {
                    nfingers++;
                }
            }
            
        }
        char buff[10] = "";
        sprintf(buff, "%d", nfingers);
        putText(final_result, buff, Point(x, y + 15), FONT_HERSHEY_COMPLEX, 0.6, Scalar(255, 255, 0), 1, 1, 0);
    }

    cv::imwrite("out_cv.bmp", final_result);
    system("pause");
    return 0;

}