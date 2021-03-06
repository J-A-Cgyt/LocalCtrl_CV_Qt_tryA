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

//��������
void WaitForKeyPress(void); //�ȴ������û�

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);  //��ӡ�豸��Ϣ

int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);  //��ɫ�ؼ�ת�� RGB->BGR

cv::Mat Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData);  //����Mat������