#include <stdio.h>
#include <stdlib.h>
// #include <windows.h>
#include <iostream>
#include<string.h>
#include <math.h>

#define w_coef 0.0036
using namespace std;

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


double To_circular_r(int x, int x0, int y, int y0) {
    return sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
}

double To_circular_theta(double x, double x0, double y, double y0) {
    return atan2((y - y0), (x - x0));
}

double convertX(double r, double theta) {
    return r * cos(theta);
}

double convertY(double r, double theta) {
    return r * sin(theta);
}

double rd2ru(double rd) {
    return tan(rd * w_coef) / (2 * tan(w_coef / 2));
}

double ru2rd(double ru) {
    return (1 / w_coef) * atan(2 * ru * tan(w_coef / 2));
}


int main()
{   
    struct BmpFileHeader fileHeader;
    struct BmpInfoHeader infoHeader;
    // BITMAPFILEHEADER fileHeader;
    // BITMAPINFOHEADER infoHeader;

    char filein[60], fileout[20];
    printf("Input image filename(*.bmp): ");
    cin >> filein;
    // printf("Output image filename(*.bmp): ");
    // cin >> fileout;

    FILE* pfin =fopen(filein,"rb");
    FILE* pfout = fopen("hw3_1_out.bmp", "wb");
    // if(NULL==pfin){
    //     printf("\7\n Cannot Open file: %s \n",filein);
    //     exit(1);
    //     }

    //Read the Bitmap file header;
    fread(&fileHeader,14,1,pfin);
    //Read the Bitmap info header;
    fread(&infoHeader,sizeof(struct BmpInfoHeader),1,pfin);
    //爲簡化代碼，只處理24位彩色
	int height,weight;
	height=0;
	weight=0;
	height=infoHeader.biHeight;
	weight=infoHeader.biWidth;
    if( infoHeader.biBitCount >= 1)
    {
        cout << "height: " << height << endl;
        cout << "width: " << weight << endl;
        int size = height * weight;

        RGB* img = (RGB*)malloc(sizeof(RGB) * size);
        
        fread( img , sizeof(RGB) , size , pfin );


        // main
        int o_x = weight / 2;
        int o_y = height / 2;

        double rd = To_circular_r(weight - 1, o_x, height - 1, o_y);
        double thetad = To_circular_theta(weight - 1, o_x, height - 1, o_y);

        double ru = rd2ru(rd);

        int Xu_max = (int)(ceil((convertX(ru, thetad) * 2 / 10) / 4) * 4);
        int Yu_max = (convertY(ru, thetad) * 2 / 10);


        RGB* new_img = (RGB*)malloc(sizeof(RGB) * (Xu_max * Yu_max));
        int i, j;
        for(i = 0; i < Yu_max; i++){
            for(j = 0; j < Xu_max; j++){
                new_img[i * Xu_max + j].b = 128;
                new_img[i * Xu_max + j].g = 128;
                new_img[i * Xu_max + j].r = 128;
            }
        }


        for (int y = 0; y < Yu_max; y++) {
            for (int x = 0; x < Xu_max; x++) {
                int xu = x * 10;
                int yu = y * 10;
                double ru = To_circular_r(xu, Xu_max / 2 * 10, yu, Yu_max / 2 * 10);
                double thetau = To_circular_theta(xu, Xu_max / 2 * 10, yu, Yu_max / 2 * 10);
                int rd = ru2rd(ru);

                int xd = convertX(rd, thetau) + o_x;
                int yd = convertY(rd, thetau) + o_y;
                if (yd < 0)
                    yd = 0;
                if (yd > 479)
                    yd = 479;
                if (xd < 0)
                    xd = 0;
                if (xd > 719)
                    xd = 719;

                new_img[(y * Xu_max + x)].b = img[(yd * weight + xd)].b;
                new_img[(y * Xu_max + x)].g = img[(yd * weight + xd)].g;
                new_img[(y * Xu_max + x)].r = img[(yd * weight + xd)].r;
            }
        }

        int new_w = 100, new_h = 80;
        RGB* img_res = (RGB*)malloc(sizeof(RGB) * new_w * new_h);
        for(i = 0; i < new_h; i++){
            for(j = 0; j < new_w; j++){
                img_res[i * new_w + j].b = new_img[((Yu_max / 2) - (new_h / 2) + i) * Xu_max + (Xu_max / 2 - (new_w / 2) + j)].b;
                img_res[i * new_w + j].g = new_img[((Yu_max / 2) - (new_h / 2) + i) * Xu_max + (Xu_max / 2 - (new_w / 2) + j)].g;
                img_res[i * new_w + j].r = new_img[((Yu_max / 2) - (new_h / 2) + i) * Xu_max + (Xu_max / 2 - (new_w / 2) + j)].r;
            }
        }


        infoHeader.biHeight = new_h;
	    infoHeader.biWidth = new_w;
        //將修改後的圖片保存到文件 
        fwrite( &fileHeader , 14 , 1 , pfout);
        fwrite( &infoHeader , sizeof(struct BmpInfoHeader) , 1 , pfout);
        fwrite(img_res , sizeof(RGB) , (new_w * new_h) , pfout);

    }
    fclose(pfin);
    fclose(pfout);
    // system("pause");

    return 0;
}