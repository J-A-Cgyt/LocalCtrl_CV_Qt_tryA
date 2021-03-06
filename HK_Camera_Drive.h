#pragma once

#include <stdio.h>

#include "Includes/MvCameraControl.h"
#include "opencv2/imgproc/imgproc.hpp" 
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include <string.h>

//函数声明
void WaitForKeyPress(void); //等待键盘敲击

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);  //打印设备信息

int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);  //颜色控件转换 RGB->BGR

cv::Mat Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData);  //返回Mat类数据