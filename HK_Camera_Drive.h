#pragma once

#include <stdio.h>

#include "Includes/MvCameraControl.h"
#include "opencv2/imgproc/imgproc.hpp" 
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <process.h>
#include <conio.h>
#include <string.h>

//尝试的多线程实现
#include <qthread.h>
#include <qpixmap.h>
#include <qmutex.h>


//函数声明
void WaitForKeyPress(void); //等待键盘敲击

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);  //打印设备信息

int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);  //颜色控件转换 RGB->BGR

cv::Mat Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData);  //返回Mat类数据

void releaseCamera(void* handle, int nRet,char* errorStr); //失败时释放相机解除占用状态

//QThread子类的声明 尝试使用多线程实现预览
class preViewHK_thread : public QThread {
	Q_OBJECT
public:
	preViewHK_thread(QObject *parent = nullptr):QThread(parent),g_nPayLoadSize(0),flag(false) { }  //提供默认父对象指针的构造函数
	preViewHK_thread(void* c_handle,QObject *parent = nullptr):
		handle(c_handle),QThread(parent), g_nPayLoadSize(0), flag(false) { } //传入相机控制句柄的构造函数
	~preViewHK_thread() { } //析构函数

	void run() override;  //重写的虚函数 线程执行的具体内容

	//设置相机控制参数
	void set_param(void* c_handle, unsigned int c_g_nPayLoadSize);
	void flagT();
	void flagF();

	//私有工具函数
private:  
	void mat2pixmap(const cv::Mat& Src);

//自定义信号
signals:
	void oneFrameFinished();  //完成单帧处理的信号

public slots:  //可能需要的槽函数

private:  //私有成员变量
	void *handle;                 //相机控制句柄
	unsigned int g_nPayLoadSize;  //用于相机驱动的payloadsize记录成员
	QMutex mutex;                 //临界变量的访问锁
	bool flag;                    //用于停止或启动线程的标识符

public:  //共有成员变量
	cv::Mat middle;     //用于存储中间结果的Mat
	QPixmap res;
};