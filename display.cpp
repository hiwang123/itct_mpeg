#include "libs.h"

long long int last_frame_time = 0;
double delay_scale = 1;
int play_video = 1;
int trackbar_pos_default = 0;
int trackbar_pos = 0;

void reset_display(){
	last_frame_time = 0;
}

void display_image(Pixel **data, int width, int height, double delay){
	IplImage *Image1;
	CvSize ImageSize1 = cvSize(width, height);
	Image1 = cvCreateImage(ImageSize1,IPL_DEPTH_8U,3);
	// convert to cv image format
	for(int i=0;i<Image1->height;i++)
	{
		for(int j=0;j<Image1->widthStep;j=j+3)
		{
			Image1->imageData[i*Image1->widthStep+j]=data[i][j/3].RGB[0];
			Image1->imageData[i*Image1->widthStep+j+1]=data[i][j/3].RGB[1];
			Image1->imageData[i*Image1->widthStep+j+2]=data[i][j/3].RGB[2];
		}
	}
	long long int cur_frame_time = get_current_time();
	// count real delay
	delay *= delay_scale;
	long long int real_delay = delay - (cur_frame_time - last_frame_time); 
	if(last_frame_time && real_delay >0) // if delay<=0, no wait
		cvWaitKey(real_delay); //delay
	cvShowImage("Mpeg-1 player", Image1);
	last_frame_time = get_current_time();
	trackbar_pos_default++;
	trackbar_pos = trackbar_pos_default;
	cvSetTrackbarPos("Pic","Mpeg-1 controller", trackbar_pos);
	while(!play_video){ // pause
		reset_display();
		cvWaitKey(5);
	}
}

void onTrackbarRandomAccess(int target_pos){
	if(!play_video){ // can only random access when pause video
		printf("random access pic: %d\n",target_pos);
		trackbar_pos_default = target_pos;
		set_pic_pos(target_pos);
	}
}

int cwidth = 360, cheight = 80;
int c2witdh = 150;
CvFont Font1=cvFont(2, 2);
CvSize ImageSize1 = cvSize(cwidth + c2witdh,cheight); 
IplImage *Image1 = cvCreateImage(ImageSize1,IPL_DEPTH_8U,3);

void onMouse(int Event,int x,int y,int flags,void* param){
	if(Event == CV_EVENT_LBUTTONDOWN){ // mouse down event
		if(x>=cwidth/3 && x<2*cwidth/3 && y>=0 && y<=cheight){  //play or pause
			printf("play/pause %d %d\n",x,y);
			play_video = 1 - play_video;
			cvRectangle(Image1, cvPoint(cwidth/3,0), cvPoint(cwidth*2/3,cheight), CV_RGB(0, 153, 204), CV_FILLED);
			if(play_video) cvPutText(Image1, "pause", cvPoint(cwidth/3+20,cheight/2), &Font1, CV_RGB(0,0,0));
			else cvPutText(Image1, "play", cvPoint(cwidth/3+20,cheight/2), &Font1, CV_RGB(0,0,0));
			cvShowImage("Mpeg-1 controller", Image1);
		}else{ //change playing speed
			if(x>=0 && x<cwidth/3 && y>=0 && y<=cheight){ // slow down
				printf("0.5x %d %d\n",x,y);
				delay_scale *= 2;
				delay_scale = Min(delay_scale, 4);
			}else if(x>=2*cwidth/3 && x<cwidth && y>=0 && y<=cheight){ // fast up
				printf("2x %d %d\n",x,y);
				delay_scale *= 0.5;
				delay_scale = Max(delay_scale, 0.25);
			}
			char buf0[20] = "x";
			char buf[10];
			sprintf(buf, "%.2f", 1.0/delay_scale);
			cvRectangle(Image1, cvPoint(cwidth,0), cvPoint(cwidth+c2witdh,cheight), CV_RGB(255, 255, 255), CV_FILLED);
			strcat(buf0, buf);
			cvPutText(Image1, buf0, cvPoint(cwidth+20,cheight/2), &Font1, CV_RGB(0,0,0));
			cvShowImage("Mpeg-1 controller", Image1);
		}
	}
}

void set_control_pannel(int all_frame_num){
	cvNamedWindow("Mpeg-1 controller", 0);
	cvResizeWindow("Mpeg-1 controller",cwidth + c2witdh,cheight); 
	cvSetMouseCallback("Mpeg-1 controller",onMouse,NULL);
	cvRectangle(Image1, cvPoint(0,0), cvPoint(cwidth/3,cheight), CV_RGB(176, 196, 222), CV_FILLED);
	cvPutText(Image1, "0.5x", cvPoint(20,cheight/2), &Font1, CV_RGB(0,0,0));
	cvRectangle(Image1, cvPoint(cwidth/3,0), cvPoint(cwidth*2/3,cheight), CV_RGB(0, 153, 204), CV_FILLED);
	cvPutText(Image1, "pause", cvPoint(cwidth/3+20,cheight/2), &Font1, CV_RGB(0,0,0));
	cvRectangle(Image1, cvPoint(cwidth*2/3,0), cvPoint(cwidth,cheight), CV_RGB(204, 255, 255), CV_FILLED);
	cvPutText(Image1, "2x", cvPoint(cwidth*2/3+20,cheight/2), &Font1, CV_RGB(0,0,0));
	cvRectangle(Image1, cvPoint(cwidth,0), cvPoint(cwidth+c2witdh,cheight), CV_RGB(255, 255, 255), CV_FILLED);
	cvPutText(Image1, "x1.00", cvPoint(cwidth+20,cheight/2), &Font1, CV_RGB(0,0,0));
	cvShowImage("Mpeg-1 controller", Image1);
	cvCreateTrackbar("Pic", "Mpeg-1 controller", &trackbar_pos, all_frame_num-1, onTrackbarRandomAccess);
}

void init_gui(){
	cvNamedWindow("Mpeg-1 player", 0);
}
