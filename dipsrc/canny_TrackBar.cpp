#include "Func_Proj_2nd.h"

	//ȫ�ֱ�������
Mat src, src_gray;//src_grayδʹ��
Mat dst, detected_edges;
int edgeThresh = 1;
int lowThreshold;	
int const max_lowThreshold = 100;
int Ratio;
int const Max_Ratio = 20;
int kernel_size = 3;
const char* Window_name = "Edge Map";

	//trackbar�ص������������贫�뺯���ʴ˲�д������
static void CannyThreshold(int, void*)
{
	blur(src, detected_edges, Size(kernel_size, kernel_size));//��3*3�ľ���Ϊ�˽���
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold * Ratio, kernel_size);//canny��Ե���
	dst = Scalar::all(0);
	src.copyTo(dst, detected_edges);//��������õĽ����Ϊ����ʹԴͼ�񿽱����������
	imshow(Window_name, dst);//��ʾͼ��
}

int TrackBar_cgyt_canny(int, char** argv)
{
	src = imread("G:\\Pictures\\Test For Programming\\ɨ����_����.jpg", IMREAD_COLOR);
	if (!src.data)
	{
		return -1;
	}
	resize(src, src, cv::Size(800, 600));//��С

	cvtColor(src, src, COLOR_BGR2GRAY); //ɫ��ת��
	equalizeHist(src, src);//ֱ��ͼ����
	dst.create(src.size(), src.type());

		namedWindow(Window_name, WINDOW_NORMAL);

	createTrackbar("Min Threshold:", Window_name, &lowThreshold, max_lowThreshold, CannyThreshold);
	createTrackbar("Ratio", Window_name, &Ratio, Max_Ratio, CannyThreshold);

	CannyThreshold(0, 0);

	waitKey(0);
	return 0;
}

