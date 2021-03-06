#pragma once
//opencv400基本库
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/photo/photo.hpp>
#include <opencv2/ml.hpp> //机器学习功能模块 SVM DTree ANN

//c++标准文件库
#include <iostream>
#include <vector>
#include <math.h>

//加强部分，特征检测
#include <opencv2/xfeatures2d/nonfree.hpp>
//这俩头文件有啥不一样？
#include <opencv2/xfeatures2d.hpp>

//#include "D:/编程/OpenCV/opencv_contrib-4.0.0/modules/xfeatures2d/include/opencv2/xfeatures2d.hpp"
//#define buffer

using namespace std;
using namespace cv;

#ifndef Self_Blur //自行建立的滤波方法

Mat GeoMeanFilter(Mat Src, int size); //几何均值滤波器 20201014

Mat HarmonicMeanFilter(Mat Src, int size, double n, int Flag); //谐波均值滤波器 20201014

Mat Gaosi_双边(Mat Src); //高斯双边滤波与锐化，保持边缘的滤波 2019/11/14,从测试与积累项目移植

Mat NLM_CGYT(Mat Src); //NLM，非局部均值滤波

Mat Anisotropic_Cgyt(Mat Src, double K, double Lamda, int Iter); //各向异性扩散滤波 20200421

int Hu_Ju_CGYT(Mat Src,double* Hu_Ju); //Hu矩计算方法，输入图像

int Hu_Ju_CGYT(vector<Point> Contour,double* Hu_Ju); //Hu矩计算方法,输入点集

vector<Point2d> SubPixel_Contours_Cgyt(Mat Src, vector<Point> contours); //Zernike矩的实现尝试，代码+论文，慢慢来不急 几何矩已实现，找时间加强成多线程的

double Solve_Cgyt(double P); //非线性方程组求解函数 方程为 x - 0.5*sin(2 * x) = Pi * P,使用简单迭代方法求解,用以计算矩方法的亚像素坐标值的中间参数

vector<Point2d> SubPixel_Contours_Cgyt(Mat Src_2, vector<Point> Contours_2, int second); //普通矩方法的亚像素边缘检测重载

#endif // !Self_Blur

#ifndef Pre_Proc

Mat Thershold_区域(Mat Src); //区域阈值

Mat FT_CGYT(Mat Src,Mat &MiddleRes); //傅里叶变换

Mat Filter_Freq(Mat Src, unsigned char FilterType); //频域滤波的尝试 滤波器的生成其实应该类似卷积，有一套

Mat Conv_Cgyt(Mat Src,int K_mod); //图像灰度梯度计算 源文件Cgyt_conv.cpp

vector<Vec4f> LSD_cgyt(Mat Src); //LSD线段检测

Mat FenShuiLing_CGYT(Mat Src);   //分水岭方法+区域随机颜色填充结果展示

int Calib_Cgyt(Mat InputOutputArray);

Mat MoHu_HuiDuBianHuan(Mat Src); //灰度变换的处理尝试-模糊方法的理解学习

Mat MoHu_HuiDuBianHuan(Mat Src, int MT); //多线程版

#endif // !Pre_Proc

#ifndef Features2nd

Mat FAST_dect_Cgyt(Mat Src1, Mat Src2); //FAST角点检测

Mat JiaoDian_SURF_CGYT(Mat Src1, Mat Src2); //SURF角点检测与计算

vector<Point2f> subPix_pt(Mat Src); //在已有像素级角点检测结果的基础上细化，使用OpenCV自带的函数

Point3f IrisDect(Mat Src,int method);  //虹膜位置检测

Point3f IrisDectH_GPU(Mat Src);

int ORBG_cgyt(Mat Src);

int FaceG_cgyt(Mat Src); //使用的是HAAR的级联分类器 设定需输入灰度图像 20200723 啥函数没实现

int FaceC_cgyt(Mat Src); //使用的是HAAR的级联分类器 CPU版本

int Components_Connected_cgyt(Mat Src);

Mat CannyG_Cgyt(Mat Src); //GPU版本的Canny

int HoughLineG_Cgyt(Mat Src_edge);

#endif // !Features2nd


#ifndef buffer  //这个区域用暂存文件内在主函数下定义的临时试验函数

int TrackBar_cgyt_canny(int, char** argv);

static void CannyThreshold(int, void*);

Mat AnisotropicMT_Cgyt(Mat Src, double K, double Lamda, int Iter); //多线程改造尝试

Mat WaveLetHarrD_CGYT(Mat Src, int LevelCount);//小波分解尝试，应是harr小波类型，别的类型小波还没找到具体的算法思路

Mat WaveLetHarrR_CGYT(Mat Src, int LevelCount);//小波分解结果合并

Mat GPU_func_study(Mat Src1, Mat Src2);

int SURFG_cgyt(Mat Src1, Mat Src2);  //SURF的GPU版本，现在这个专利保护期已过，可以随便用了

#endif // !buffer

#ifndef Calc

void HistogramCGYT(Mat Src);

#endif // !Calc 

//新写一个文件用来放纯数学的方法，已经写进去的什么迭代求解方程啊就不挪了
#define MathActivate 1  //数学方法是否启用的宏定义 1-启用 2-不启用

#include "MathCgyt.h"