#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>

typedef struct 
{
    BYTE b;
    BYTE g;
    BYTE r;
}RGB;

void imgOpr(RGB &a,RGB &b)
{
	b.b=a.b;	
	b.g=a.g;
	b.r=a.r;
}

void imgOprChange(RGB &a,RGB &b)
{
    b.b=a.g;	
	b.g=a.r;
	b.r=a.b;
}

int main()
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    char filein[60], fileout[20];
    int mode;
    printf("Input image filename(*.bmp): ");
    cin >> filein;
    printf("Output image filename(*.bmp): ");
    cin >> fileout;
    printf("0: read & write img \n1: rotate img \n2: rotate & change channel \nEnter mode: ");
    cin >> mode;

    FILE* pfin =fopen(filein,"rb");
    FILE* pfout = fopen(fileout, "wb");
    if(NULL==pfin){
        printf("\7\n Cannot Open file: %s \n",filein);
        exit(1);
        }
    if(NULL==pfout){
        printf("\7\n Cannot Open file: %s \n", fileout);
        exit(1);
    }
    //Read the Bitmap file header;
    fread(&fileHeader,sizeof(BITMAPFILEHEADER),1,pfin);
    //Read the Bitmap info header;
    fread(&infoHeader,sizeof(BITMAPINFOHEADER),1,pfin);
    //爲簡化代碼，只處理24位彩色
	int height,weight;
	height=0;
	weight=0;
	height=infoHeader.biHeight;
	weight=infoHeader.biWidth;
    if( infoHeader.biBitCount >= 1)
    {
        cout << "height: " << height << endl;
        cout << "weight: " << weight << endl;
        int size = height * weight;
        // RGB img[height][weight];
        // RGB img2[height][weight];
        RGB* img = (RGB*)malloc(sizeof(RGB) * size);
        RGB* img2 = (RGB*)malloc(sizeof(RGB) * size);
        fread( img , sizeof(RGB) , size , pfin );

        // 根據 mode 對圖片做處理
        int i, j;
        for (i=0; i<height; i++) 
			for (j=0 ; j < weight ; j++ )
			{
				//將數組 img 賦值給 img2 
                if(mode == 0)
				    // imgOpr(img[i][j], img2[i][j]);
				    imgOpr(img[i * weight + j], img2[i * weight + j]);

                else if(mode == 1)
                    // imgOpr(img[i][j], img2[(height - 1) - i][(weight - 1) - j]);
                    imgOpr(img[i * weight + j], img2[((height - 1) - i) * weight + ((weight - 1) - j)]);

                else
                    // imgOprChange(img[i][j], img2[(height - 1) - i][(weight - 1) - j]);
                    imgOprChange(img[i * weight + j], img2[((height - 1) - i) * weight + ((weight - 1) - j)]);

            }
        
        //將修改後的圖片保存到文件 
        fwrite( &fileHeader , sizeof(fileHeader) , 1 , pfout);
        fwrite( &infoHeader , sizeof(infoHeader) , 1 , pfout);
        fwrite( img2 , sizeof(RGB) , size , pfout );
    }
    fclose(pfin);
    fclose(pfout);
    printf("Save Successful ... \n");
    system("pause");

    return 0;
}