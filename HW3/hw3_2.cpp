#include <stdio.h>
#include <stdlib.h>
// #include <windows.h>
#include <iostream>
#include<string.h>
#include <math.h>

#define M_PI 3.14159265358979323846
#define alpha 15 * M_PI / 180
#define dx 4
#define dy -10
#define dz 5
#define th0 0.025
#define gamma0 0


using namespace std;

// #include <opencv2/opencv.hpp>

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

struct BmpFileHeader{
    WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
};

struct BmpInfoHeader{
    DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
};

#define BYTE unsigned char
typedef struct 
{
    BYTE b;
    BYTE g;
    BYTE r;
}RGB;

double cot(double x)
{
    return tan(M_PI / 2 - x);
}

double worldToImg_u(int m, double x, double y) {
    return ((m - 1) / (2 * alpha)) * (atan((dz * sin(atan((x - dx) / (y - dy)))) / (x - dx)) - 0.025 + alpha);
}
double worldToImg_v(int n, double x, double y) {
    return  ((n - 1)  / (2 * alpha)) * (atan((x - dx) / (y - dy)) - 0 + alpha);
}

double imgToWorld_x(int m, int n, double u, double v) {
    return dz * cot(th0 - alpha + u * ((2 * alpha) / (m - 1))) * sin(gamma0-alpha+v*((2*alpha)/(n-1)))+dx;
}
double imgToWorld_y(int m, int n, double u, double v) {
    return dz * cot(th0 - alpha + u * ((2 * alpha) / (m - 1))) * cos(gamma0 - alpha + v * ((2 * alpha) / (n - 1)))+dy;
}


int main()
{   
    struct BmpFileHeader fileHeader;
    struct BmpInfoHeader infoHeader;
    // BITMAPFILEHEADER fileHeader;
    // BITMAPINFOHEADER infoHeader;

    char filein[60], fileout[20];
    int mode;
    printf("Input image filename(*.bmp): ");
    cin >> filein;
    // printf("Output image filename(*.bmp): ");
    // cin >> fileout;

    FILE* pfin =fopen(filein,"rb");
    FILE* pfout = fopen("hw3_2_out.bmp", "wb");
    // if(NULL==pfin){
    //     printf("\7\n Cannot Open file: %s \n",filein);
    //     exit(1);
    //     }

    //Read the Bitmap file header;
    fread(&fileHeader,14,1,pfin);
    //Read the Bitmap info header;
    fread(&infoHeader,sizeof(struct BmpInfoHeader),1,pfin);
    //爲簡化代碼，只處理24位彩色
	int height, width;
	height=0;
	width=0;
	height=infoHeader.biHeight;
	width=infoHeader.biWidth;
    cout << "TMP:" << infoHeader.biSize << endl;
    if( infoHeader.biBitCount >= 1)
    {
        // cout << "height: " << height << endl;
        // cout << "weight: " << width << endl;
        int size = height * width;
        int i, j;
        RGB* img = (RGB*)malloc(sizeof(RGB) * size);
        fread( img , sizeof(RGB) , size , pfin );

        RGB* img_reSeq = (RGB*)malloc(sizeof(RGB) * size);
        for (i = 0; i < height; i++){
            for(j = 0; j < width; j++){
                img_reSeq[i * width + j].b = img[(height - 1 - i) * width + j].b;
                img_reSeq[i * width + j].g = img[(height - 1 - i) * width + j].g;
                img_reSeq[i * width + j].r = img[(height - 1 - i) * width + j].r;
            }
        }

        int uHorizon = ceil(((height - 1) / (2 * alpha)) * (-th0 + alpha));

        int x_max = 0;
        int y_max = 0;
        int x_min = 0;
        int y_min = 0;
        for (int u = uHorizon; u < height; u++) {
            for (int v = 0; v < width; v++) {
                double tmpx = imgToWorld_x(height, width, u, v);
                double tmpy = imgToWorld_y(height, width, u, v);
                
                if (tmpx > x_max) x_max = tmpx;
                if (tmpy > y_max) y_max = tmpy;

                if (tmpx < x_min) x_min = tmpx;
                if (tmpy < y_min) y_min = tmpy;
            }
        }

        int world_Width = y_max - y_min;
        int world_Height = x_max - x_min;
        cout << "Min X : Max X  " << x_min << " : " << x_max << endl;
        cout << "Min Y : Max Y  " << y_min << " : " << y_max << endl;
        

        int world_img_width = (world_Width/30);
        int world_img_height = world_Height/30;
        cout << "width: " << world_img_width << ", height: " << world_img_height << endl;

        // init and inverse unwrping world img
        RGB* world_out = (RGB*)malloc(sizeof(RGB) * world_img_width * world_img_height);
        for (int i = 0; i < world_img_height; i++) {
            for (int j = 0; j < world_img_width; j++) {
                world_out[(i * world_img_width + j)].b = 0;
                world_out[(i * world_img_width + j)].g = 0;
                world_out[(i * world_img_width + j)].r = 0;
            }
        }

        for (int xp = 0; xp < world_img_height; xp++) {
            for (int yp = 0; yp < world_img_width; yp++) {
                int y = yp * 30 + y_min;
                int x = xp * 30 + x_min;
                int u = worldToImg_u(height,  x,  y);
                int v = worldToImg_v(width,  x,  y);

                if (v >= 0 && v <= 1023 && u >= 347 && u <= 767) {
                    world_out[(yp + world_img_width * ((world_img_height - 1) - xp))].b = img_reSeq[(v + width * u)].b;
                    world_out[(yp + world_img_width * ((world_img_height - 1) - xp))].g = img_reSeq[(v + width * u)].g;
                    world_out[(yp + world_img_width * ((world_img_height - 1) - xp))].r = img_reSeq[(v + width * u)].r;

                }
            }
        }

        //將修改後的圖片保存到文件 
        infoHeader.biHeight = world_img_height;
	    infoHeader.biWidth = world_img_width;
        fwrite( &fileHeader , 14 , 1 , pfout);
        fwrite( &infoHeader , sizeof(struct BmpInfoHeader) , 1 , pfout);
        fwrite(world_out , sizeof(RGB) , world_img_width * world_img_height , pfout);
    }
    fclose(pfin);
    fclose(pfout);
    system("pause");

    return 0;
}