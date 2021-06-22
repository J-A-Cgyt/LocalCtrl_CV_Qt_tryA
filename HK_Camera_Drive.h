#pragma once

#include <stdio.h>

#include "Includes/MvCameraControl.h"
#include "opencv2/imgproc/imgproc.hpp" 
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <process.h>
#include <conio.h>
#include <string.h>

//���ԵĶ��߳�ʵ��
#include <qthread.h>
#include <qpixmap.h>
#include <qmutex.h>


//��������
void WaitForKeyPress(void); //�ȴ������û�

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);  //��ӡ�豸��Ϣ

int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);  //��ɫ�ؼ�ת�� RGB->BGR

cv::Mat Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData);  //����Mat������

void releaseCamera(void* handle, int nRet,char* errorStr); //ʧ��ʱ�ͷ�������ռ��״̬

//QThread��������� ����ʹ�ö��߳�ʵ��Ԥ��
class preViewHK_thread : public QThread {
	Q_OBJECT
public:
	preViewHK_thread(QObject *parent = nullptr):QThread(parent),g_nPayLoadSize(0),flag(false) { }  //�ṩĬ�ϸ�����ָ��Ĺ��캯��
	preViewHK_thread(void* c_handle,QObject *parent = nullptr):
		handle(c_handle),QThread(parent), g_nPayLoadSize(0), flag(false) { } //����������ƾ���Ĺ��캯��
	~preViewHK_thread() { } //��������

	void run() override;  //��д���麯�� �߳�ִ�еľ�������

	//����������Ʋ���
	void set_param(void* c_handle, unsigned int c_g_nPayLoadSize);
	void flagT();
	void flagF();

	//˽�й��ߺ���
private:  
	void mat2pixmap(const cv::Mat& Src);

//�Զ����ź�
signals:
	void oneFrameFinished();  //��ɵ�֡������ź�

public slots:  //������Ҫ�Ĳۺ���

private:  //˽�г�Ա����
	void *handle;                 //������ƾ��
	unsigned int g_nPayLoadSize;  //�������������payloadsize��¼��Ա
	QMutex mutex;                 //�ٽ�����ķ�����
	bool flag;                    //����ֹͣ�������̵߳ı�ʶ��

public:  //���г�Ա����
	cv::Mat middle;     //���ڴ洢�м�����Mat
	QPixmap res;
};