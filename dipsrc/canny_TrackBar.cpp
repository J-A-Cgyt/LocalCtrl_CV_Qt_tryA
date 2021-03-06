#include "Func_Proj_2nd.h"

	//全局变量声明
Mat src, src_gray;//src_gray未使用
Mat dst, detected_edges;
int edgeThresh = 1;
int lowThreshold;	
int const max_lowThreshold = 100;
int Ratio;
int const Max_Ratio = 20;
int kernel_size = 3;
const char* Window_name = "Edge Map";

	//trackbar回调函数，因无需传入函数故此不写变量名
static void CannyThreshold(int, void*)
{
	blur(src, detected_edges, Size(kernel_size, kernel_size));//以3*3的矩形为核降噪
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold * Ratio, kernel_size);//canny边缘检测
	dst = Scalar::all(0);
	src.copyTo(dst, detected_edges);//将检测所得的结果作为掩码使源图像拷贝至输出矩阵
	imshow(Window_name, dst);//显示图像
}

int TrackBar_cgyt_canny(int, char** argv)
{
	src = imread("G:\\Pictures\\Test For Programming\\扫描仪_曲线.jpg", IMREAD_COLOR);
	if (!src.data)
	{
		return -1;
	}
	resize(src, src, cv::Size(800, 600));//缩小

	cvtColor(src, src, COLOR_BGR2GRAY); //色域转换
	equalizeHist(src, src);//直方图均化
	dst.create(src.size(), src.type());

		namedWindow(Window_name, WINDOW_NORMAL);

	createTrackbar("Min Threshold:", Window_name, &lowThreshold, max_lowThreshold, CannyThreshold);
	createTrackbar("Ratio", Window_name, &Ratio, Max_Ratio, CannyThreshold);

	CannyThreshold(0, 0);

	waitKey(0);
	return 0;
}

