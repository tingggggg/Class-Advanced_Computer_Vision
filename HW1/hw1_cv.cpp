#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(){

    char filein[60], fileout[20];
    int mode;
    printf("Input image filename(*.bmp): ");
    cin >> filein;
    printf("Output image filename(*.bmp): ");
    cin >> fileout;
    printf("0: read & write img \n1: rotate img \n2: rotate & change channel \nEnter mode: ");
    cin >> mode;

    cv::Mat img = cv::imread(filein);

    if (img.empty()){
        cout << "empty" << endl;
        system("pause");
        return -1;
    }

    if(mode == 0){
        cv::imwrite(fileout, img);
        return 0;
    }
    else if(mode == 1){
        cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
        cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
        cv::imwrite(fileout, img);
        return 0;

    }
    else if(mode == 2){
        vector<Mat> channels;
        cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
        cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
        split(img, channels);
        // B: 0 | G: 1 | R: 2
        // B -> G
        // G -> R
        // R -> B
        Mat rechannel;
        Mat resrc[3] = {channels[1], channels[2], channels[0]};
        merge(resrc, 3, rechannel);
        cv::imwrite(fileout, rechannel);
        return 0;
    }

    return 0;
}