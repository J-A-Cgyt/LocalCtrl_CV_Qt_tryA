#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CV_MinWin.h"
//数字图像处理的相关代码 Mat都能用说明环境配置也没啥问题
#include "dipsrc/Func_Proj_2nd.h"

class CV_MinWin : public QMainWindow  //这是在窗体外面又套了个壳 但是在这个壳里面的槽函数 应该也是能被调用的 毕竟这个类里面除了一个ui的实例别的啥也没有
{
    Q_OBJECT

public:
    CV_MinWin(QWidget *parent = Q_NULLPTR);

private slots:  //如果信号和槽无需自定义 也不一定要让接收者是主窗口 某个窗口部件也行
	void openPic();    //打开图像
	void gaussBlur();  //用一个简单的高斯滤波测试一下功能 槽中要弹出一个对话框
	void undo();

	//尝试构建海康相机的驱动代码 可能需要槽函数和非槽函数的配合
	void connectCamera();

private: //这里是一些不是槽 但是被槽调用的工具函数声明
	void showMat(const Mat &src);  //为了显示处理结果需要设置一个从cv::Mat向QPixMap转换的函数
	

private:
    Ui::CV_MinWinClass ui;  //这才是真正的窗体实例
	//数据成员
	Mat Src;		//原始图像 成员
	Mat Res;		//结果图像 成员
	Mat temp;		//中间结果 成员
	//显示成员
	QPixmap displayImg; //用于显示的成员
};

//试试看构建图像处理程序