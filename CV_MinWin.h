#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CV_MinWin.h"
//����ͼ�������ش��� Mat������˵����������Ҳûɶ����
#include "dipsrc/Func_Proj_2nd.h"
//����������߳�
//#include <thread>
class preViewHK_thread;

class CV_MinWin : public QMainWindow  //�����ڴ������������˸��� ���������������Ĳۺ��� Ӧ��Ҳ���ܱ����õ� �Ͼ�������������һ��ui��ʵ�����ɶҲû��
{
    Q_OBJECT

public:
    CV_MinWin(QWidget *parent = Q_NULLPTR);

private slots:  //����źźͲ������Զ��� Ҳ��һ��Ҫ�ý������������� ĳ�����ڲ���Ҳ��
	void openPic();    //��ͼ��
	void gaussBlur();  //��һ���򵥵ĸ�˹�˲�����һ�¹��� ����Ҫ����һ���Ի���
	void undo();

	//���Թ�������������������� ������Ҫ�ۺ����ͷǲۺ��������
	void connectCamera();
	void startView();
	void endView();

	//ʹ�ö��̵߳ģ����Զ����źŴ�������ʾͼ��ۺ���
	void updatePic();

private: //������һЩ���ǲ� ���Ǳ��۵��õĹ��ߺ�������
	void showMat(const Mat &src);  //Ϊ����ʾ��������Ҫ����һ����cv::Mat��QPixMapת���ĺ���
	//void viewRefresh();          //ˢ��ͼ��Ķ��̺߳˺���

private:
    Ui::CV_MinWinClass ui;  //����������Ĵ���ʵ��
	//���ݳ�Ա
	Mat Src;		//ԭʼͼ�� ��Ա
	Mat Res;		//���ͼ�� ��Ա
	Mat temp;		//�м��� ��Ա
	//��ʾ��Ա
	QPixmap displayImg; //������ʾ�ĳ�Ա

private:
	unsigned int g_nPayLoadSize;  //�������������payloadsize��¼��Ա
	void* handle;                 //����������Ƶľ��
	bool isViewing;               //�Ƿ�����Ԥ��
	bool isConnected;             //�Ƿ�������

	//����ʵʱԤ���ĳ�Ա�߳�
	preViewHK_thread* thread_view;
};

//���Կ�����ͼ�������