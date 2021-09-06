#include <stdio.h>
#include <stdlib.h>
// #include <windows.h>
#include <iostream>
#include<string.h>
#include <math.h>
#include <time.h>
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

typedef struct components
{
    int total_pixel = 0;
    int total_x = 0, total_y = 0;
    int min_x = 0, min_y = 0, max_x = 0, max_y = 0;
    bool handVertical = true; // mode handVertical the hand is vertical: true  or  horizontal: false
    int finger_cnt = 0;

}ccp;

void grayLevel(RGB &a,RGB &b)
{
    int value = a.r * 0.299 + a.g * 0.587 + a.b * 0.114;
	b.b=value;	
	b.g=value;
	b.r=value;
}

void binary(RGB &a,RGB &b)
{
    int value = a.r * 0.299 + a.g * 0.587 + a.b * 0.114;
    if (value < 220)
        value = 0;
    else
        value = 255;
    b.b=value;	
	b.g=value;
	b.r=value;
}
void copy_img(RGB* img, RGB* img_finger)
{
    img->b = img_finger->b;
    img->g = img_finger->g;
    img->r = img_finger->r;
}

void BFS(RGB* pixel, int idx, int row_idx, int col_idx, ccp* comp)
{   
    pixel->b = idx * 18;
    pixel->g = idx * 18;
    pixel->r = idx * 18;
    comp->total_pixel++;
    comp->total_x += col_idx;
    comp->total_y += row_idx;
    // right
    if(col_idx + 1 < 512){
        if((pixel + 1)->b == 0){
            if(col_idx + 1 > comp->max_x)
                comp->max_x = col_idx + 1;
            BFS((pixel + 1), idx, row_idx, col_idx + 1, comp);
        }
            
    }
    // down
    if(row_idx + 1 < 512){
        if((pixel + 512)->b == 0){
            if(row_idx + 1 > comp->max_y)
                comp->max_y = row_idx + 1; 
            BFS((pixel + 512), idx, row_idx + 1, col_idx, comp);
        }
            
    }
    // left
    if(col_idx - 1 >= 0){
        if((pixel - 1)->b == 0){
            if(col_idx - 1 < comp->min_x)
                comp->min_x = col_idx - 1;
            BFS((pixel - 1), idx, row_idx, col_idx - 1, comp);
        }
            
    }
    // top
    if(row_idx - 1 >= 0){
        if((pixel - 512)->b == 0){
            if(row_idx - 1 < comp->min_y)
                comp->min_y = row_idx - 1;
            BFS((pixel - 512), idx, row_idx - 1, col_idx, comp);
        }
    }
}

void plot_bbox(RGB* img, ccp* comp, int rgb[])
{
    RGB* pImg = img;
    for(int i = comp->min_x; i < comp->max_x + 1; i++){
        // top & down
        (pImg + (comp->min_y * 512) + i)->b = rgb[2];
        (pImg + (comp->min_y * 512) + i)->g = rgb[1];
        (pImg + (comp->min_y * 512) + i)->r = rgb[0];

        (pImg + (comp->max_y * 512) + i)->b = rgb[2];
        (pImg + (comp->max_y * 512) + i)->g = rgb[1];
        (pImg + (comp->max_y * 512) + i)->r = rgb[0];
    }

    for(int i = comp->min_y; i < comp->max_y + 1; i++){
        // left & right
        (pImg + (i * 512) + comp->min_x)->b = rgb[2];
        (pImg + (i * 512) + comp->min_x)->g = rgb[1];
        (pImg + (i * 512) + comp->min_x)->r = rgb[0];

        (pImg + (i * 512) + comp->max_x)->b = rgb[2];
        (pImg + (i * 512) + comp->max_x)->g = rgb[1];
        (pImg + (i * 512) + comp->max_x)->r = rgb[0];
    }

}

void dilation(int row_idx, int col_idx, int idx, RGB* img_finger, bool isVertical){
    int i, j;
    int mask_edge_w = 27; // e.g. 3, 5 or 7
    int mask_edge_h = 3;
    if(!isVertical){
        // if not vertical rotate mask
        int tmp = mask_edge_h;
        mask_edge_h = mask_edge_w;
        mask_edge_w = tmp;
    }
    // define mask height & Width
    // e.g. w = 5, h = 3
    // *****
    // *****
    // *****
    for(i = -(mask_edge_h / 2); i < (mask_edge_h / 2 + 1); i++){
        for(j = -(mask_edge_w / 2); j < (mask_edge_w / 2 + 1); j++){
            if(row_idx + i < 0 || row_idx + i >= 512) continue;
            if(col_idx + j < 0 || col_idx + j >= 512) continue;
            (img_finger + i * 512 + j)->b = idx * 18;
            (img_finger + i * 512 + j)->g = idx * 18;
            (img_finger + i * 512 + j)->r = idx * 18;
        }
    }
}

bool erosion(RGB* img, int row_idx, int col_idx, int idx, bool isVertical){
    int i, j;
    int mask_edge_w = 27; // e.g. 3, 5 or 7
    int mask_edge_h = 3;
    if(!isVertical){
        // if not vertical rotate mask
        int tmp = mask_edge_h;
        mask_edge_h = mask_edge_w;
        mask_edge_w = tmp;
    }
    // define mask height & Width
    // e.g. w = 5, h = 3
    // *****
    // *****
    // *****
    
    int total_cnt = mask_edge_w * mask_edge_h;
    int cnt = 0;
    for(i = -(mask_edge_h / 2); i < (mask_edge_h / 2 + 1); i++){
        for(j = -(mask_edge_w / 2); j < (mask_edge_w / 2 + 1); j++){
            if(row_idx + i < 0 || row_idx + i >= 512) continue;
            if(col_idx + j < 0 || col_idx + j >= 512) continue;
            if((img + (i * 512) + j)->b == (idx * 18))
                cnt++;
        }
    }
    if(cnt < total_cnt)
    {
        // if cnt not sufficient your assign filter or mask then return true
        return true;
    }
    return false;
}

void find_fingers(RGB* img, ccp* comp, RGB* img_finger,int idx){
    int i, j;
    // creat a tmp original img to retain fingers
    RGB* tmpOriImg = (RGB*)malloc(sizeof(RGB) * 512 * 512);
    memcpy(tmpOriImg, img, sizeof(RGB) * 512 * 512);

    // do erode
    for(i = comp->min_y; i < comp->max_y + 1; i++){
        for(j = comp->min_x; j < comp->max_x + 1; j++){
            if(erosion((img + i * 512 + j), i, j, idx, comp->handVertical)){
                (img_finger + i * 512 + j)->b = 255;
                (img_finger + i * 512 + j)->g = 255;
                (img_finger + i * 512 + j)->r = 255;
            }
            else{
                (img_finger + i * 512 + j)->b = idx * 18;
                (img_finger + i * 512 + j)->g = idx * 18;
                (img_finger + i * 512 + j)->r = idx * 18;
            }
        }
    }
    // copy
    memcpy(img, img_finger, sizeof(RGB)*512*512);

    // do dilate
    for(i = comp->min_y; i < comp->max_y + 1; i++){
        for(j = comp->min_x; j < comp->max_x + 1; j++){
            if ((img + i * 512 + j)->b == idx * 18){
                dilation(i, j, idx, (img_finger + i * 512 + j), comp->handVertical);
            }
        }
    }


    // tmp original Img(with finger) dicrease no finger hand
    for(i = comp->min_y; i < comp->max_y + 1; i++){
        for(j = comp->min_x; j < comp->max_x + 1; j++)
		{
                if(((tmpOriImg + i * 512 + j)->b == idx * 18) && ((img_finger + i * 512 + j)->b == idx * 18)){
                    (tmpOriImg + i * 512 + j)->b = 255;
                    (tmpOriImg + i * 512 + j)->g = 255;
                    (tmpOriImg + i * 512 + j)->r = 255;
                }
        }
    }
    // copy only finger in to img
    memcpy(img, tmpOriImg, sizeof(RGB)*512*512);
    free(tmpOriImg);
}

void put_number_onPic(RGB* img, ccp* comp){
    // start from centorid plot purple line to reprensent the number of fingers
    // e.g. 2 line : this hand have 2 finger be find out
    int i, j;
    for(i = 0; i < comp->finger_cnt; i++){
        for(j = 0; j < 15; j++){
            (img + ((comp->total_y / comp->total_pixel) + i * 4) * 512 + (comp->total_x / comp->total_pixel) + j)->b = 255;
            (img + ((comp->total_y / comp->total_pixel) + i * 4) * 512 + (comp->total_x / comp->total_pixel) + j)->g = 0;
            (img + ((comp->total_y / comp->total_pixel) + i * 4) * 512 + (comp->total_x / comp->total_pixel) + j)->r = 255;
        }
    }
}

int main()
{   
    struct BmpFileHeader fileHeader;
    struct BmpInfoHeader infoHeader;

    char filein[60], fileout[20];
    int mode;
    printf("Input image filename(*.bmp): ");
    cin >> filein;
    // printf("Output image filename(*.bmp): ");
    // cin >> fileout;

    FILE* pfin =fopen(filein,"rb");
    FILE* pfout = fopen("out.bmp", "wb");

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
        int size = height * weight;

        RGB* img = (RGB*)malloc(sizeof(RGB) * size);
        RGB* img_binary = (RGB*)malloc(sizeof(RGB) * size);
        RGB* img_finger = (RGB*)malloc(sizeof(RGB) * size);
        fread( img , sizeof(RGB) , size , pfin );

        // gray level -> binary
        int i, j;
        for (i=0; i<height; i++){
            for (j=0 ; j < weight ; j++ )
			{
                // binary
				binary(img[i * weight + j], img_binary[i * weight + j]);
                binary(img[i * weight + j], img_finger[i * weight + j]);
            }
        }
        memcpy(img, img_binary, sizeof(RGB) * 512 * 512);

        // timer
        clock_t begin, end, begin_m, end_m, begin_p, end_p;
        double cost, cost_morphology = 0.0, cost_plot = 0.0;
        begin = clock();

        // creat hand cc list for bouns
        ccp* hand_list = (ccp*)malloc(sizeof(ccp) * 5);
        cout << " ********************* hand *********************** " << endl;
        // find connected components
        int components_counts = 0;
        for (i=0; i<height; i++)
        {
            for (j=0 ; j < weight ; j++ )
			{
                if(img_binary[i * weight + j].b == 0){
                    components_counts++;
                    ccp comp;
                    comp.min_x = j;
                    comp.min_y = i;
                    comp.max_x = j;
                    comp.max_y = i;
                    BFS(&img_binary[i * weight + j], components_counts, i, j, &comp);
                    
                    // accroding min max xy to difine handVertical
                    comp.handVertical = (comp.max_y - comp.min_y) > (comp.max_x - comp.min_x) ? true : false;

                    // plot bounding box and print out results
                    int plot_color[3] = {255, 0, 0}; //red
                    begin_p = clock();
                    plot_bbox(img, &comp, plot_color);
                    end_p = clock();
                    cost_plot += (double)(end_p - begin_p)/CLOCKS_PER_SEC;

                    *(hand_list + components_counts - 1) = comp;

                    printf("area : %d, centroid point(%d, %d)\n", comp.total_pixel, comp.total_x / comp.total_pixel, 511 - comp.total_y / comp.total_pixel);
                    printf("the length of longest axis : %d, orientation : %s \n", comp.max_x - comp.min_x > comp.max_y - comp.min_y ? comp.max_x - comp.min_x : comp.max_y - comp.min_y, \
                                                                                   comp.max_x - comp.min_x > comp.max_y - comp.min_y ? "horizontal" : "vertical");

                    // do morphology to find finger components
                    begin_m = clock();
                    find_fingers(img_binary, &comp, img_finger, components_counts);
                    end_m = clock();
                    cost_morphology += (double)(end_m - begin_m)/CLOCKS_PER_SEC;
                }
            }
        }

        // timer
        end = clock();
        cost = (double)(end - begin)/CLOCKS_PER_SEC;
        cout << "connected components times of hand: " << cost - cost_morphology - cost_plot << " secs" << endl;
        cout << "morphology times: " << cost_morphology << " secs" << endl;
        cout << " ************************************************* " << endl;

        // at here find out the only finger img (gray level) in img_binary
        // then do gray level to binary in img_binary
        for (i=0; i<height; i++){
            for (j=0 ; j < weight ; j++ )
			{
                // binary
				binary(img_binary[i * weight + j], img_binary[i * weight + j]);
            }
        }

        cout << " ******************** finger ********************** " << endl;
        //timer
        begin = clock();
        // find connected components (fingers)
        int finger_components_counts = 0, print_cnt = 0;
        for (i=0; i<height; i++)
        {
            for (j=0 ; j < weight ; j++ )
			{
                if(img_binary[i * weight + j].b == 0){
                    finger_components_counts++;
                    ccp comp;
                    comp.min_x = j;
                    comp.min_y = i;
                    comp.max_x = j;
                    comp.max_y = i;
                    BFS(&img_binary[i * weight + j], finger_components_counts, i, j, &comp);
                    
                    // accroding min max xy to difine handVertical
                    comp.handVertical = (comp.max_y - comp.min_y) > (comp.max_x - comp.min_x);
                    
                    // define area bigger than 400 is a finger 
                    if(comp.total_pixel > 400)
                    {  
                        print_cnt++;
                        int plot_color[3] = {0, 0, 255}; //red
                        begin_p = clock();
                        plot_bbox(img, &comp, plot_color);
                        end_p = clock();
                        cost_plot += (double)(end_p - begin_p)/CLOCKS_PER_SEC;

                        // find out figner belong which hand
                        for(int h = 0; h < 5; h++){
                            if(comp.total_x / comp.total_pixel < (hand_list + h)->max_x && \
                               comp.total_x / comp.total_pixel > (hand_list + h)->min_x && \
                               comp.total_y / comp.total_pixel < (hand_list + h)->max_y && \
                               comp.total_y / comp.total_pixel > (hand_list + h)->min_y)
                            {
                                (hand_list + h)->finger_cnt++;
                                break;
                            }
                        }
                        printf("area : %d, centroid point(%d, %d)\n", comp.total_pixel, comp.total_x / comp.total_pixel, 511 - comp.total_y / comp.total_pixel);
                        printf("the length of longest axis : %d, orientation : %s \n", comp.max_x - comp.min_x > comp.max_y - comp.min_y ? comp.max_x - comp.min_x : comp.max_y - comp.min_y, \
                                                                                   comp.max_x - comp.min_x > comp.max_y - comp.min_y ? "horizontal" : "vertical");                        
                        cout << endl;
                    }
                }
            }
        }
        end = clock();
        cost = (double)(end - begin)/CLOCKS_PER_SEC;
        cout << "connected components times of finger: " << cost << " secs" << endl;
        cout << "plot all bbox times: " << cost_plot << " secs" << endl;
        cout << " ******************************************** " << endl;

        // put number of fingers on pic
        for(int i = 0; i < 5; i++){
            put_number_onPic(img, (hand_list + i));
        }
        
        //將修改後的圖片保存到文件 
        fwrite( &fileHeader , 14 , 1 , pfout);
        fwrite( &infoHeader , sizeof(struct BmpInfoHeader) , 1 , pfout);
        fwrite(img , sizeof(RGB) , size , pfout);
    }
    fclose(pfin);
    fclose(pfout);
    system("pause");

    return 0;
}