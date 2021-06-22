//多线程编程的尝试，挑一个函数进行多线程改造
#include "Func_Proj_2nd.h"
#include <thread> //尝试一下多线程改造？ 20200424

void Scan_MT(Mat Src, Mat Res, int Start, int End,
	unsigned char Iter2,
	double K2, double Lamda);

Mat AnisotropicMT_Cgyt(Mat Src, double K, double Lamda, int Iter) //效果存疑 似乎有种整体滤波保持边缘却锐化细节的感觉，怪怪的
{

	Mat temp = Src.clone();
	if (temp.type() == CV_8UC1)
	{
		temp.convertTo(temp, CV_64FC1);
	}
	else if (temp.type() == CV_8UC3)
	{
		vector<Mat> channels;
		split(temp, channels);
		channels[0] = AnisotropicMT_Cgyt(channels[0], K, Lamda, Iter);
		channels[1] = AnisotropicMT_Cgyt(channels[1], K, Lamda, Iter);
		channels[2] = AnisotropicMT_Cgyt(channels[2], K, Lamda, Iter);
		merge(channels, temp);
		return temp;
	}
	else
	{
		cout << "Type Error: Type of matrix must be CV_8UC3 or CV_8UC1" << endl;
		return Mat();
	}
	if (Iter < 1)
	{
		cout << "迭代次数K必须>0" << endl;
		return Mat();
	}
	double K2 = K * K;
	Mat temp2 = temp.clone();

	int bound = temp.cols / 4;
	thread t1 = thread(Scan_MT, temp, temp2, 1, bound, Iter, K2, Lamda);
	thread t2 = thread(Scan_MT, temp, temp2, bound + 1, 2 * bound, Iter, K2, Lamda);
	thread t3 = thread(Scan_MT, temp, temp2, 2 * bound + 1, 3 * bound, Iter, K2, Lamda);
	thread t4 = thread(Scan_MT, temp, temp2, 3 * bound + 1, temp.cols - 1, Iter, K2, Lamda);
	//等待线程同步完成
	t1.join();
	t2.join();
	t3.join();
	t4.join();

	//normalize(temp, temp, 0, 255, NORM_MINMAX);
	temp.convertTo(temp, CV_8UC1);
	return temp;
}

//多线程的核函数
void Scan_MT(Mat Src, Mat Res, int Start, int End, 
	unsigned char Iter2,
	double K2,double Lamda) //此处输入的的Src,Res类型必须是CV_64FC1 否则报错
{
	double Ni, Si, Wi, Ei; //四向梯度
	double Nc, Sc, Wc, Ec; //四向导热系数
	double PixValue;

	Mat temp = Src.clone();	
	Mat temp2 = Res.clone();

	while (Iter2 > 0)
	{
		for (int i = Start; i < End; i++)
		{
			for (int j = 1; j < temp.rows - 1; j++)
			{
				Wi = temp.at<double>(j, i - 1) - temp.at<double>(j, i);
				Ni = temp.at<double>(j - 1, i) - temp.at<double>(j, i);
				Ei = temp.at<double>(j, i + 1) - temp.at<double>(j, i);
				Si = temp.at<double>(j + 1, i) - temp.at<double>(j, i);

				Nc = exp(-Ni * Ni / K2);
				Sc = exp(-Si * Si / K2);
				Wc = exp(-Wi * Wi / K2);
				Ec = exp(-Ei * Ei / K2);

				PixValue = temp.at<double>(j, i) + Lamda * (Ni * Nc + Si * Sc + Wi * Wc + Ei * Ec);
				if (PixValue > 255)
				{
					temp2.at<double>(j, i) = 255;
				}
				else if (PixValue < 0)
				{
					temp2.at<double>(j, i) = 0;
				}
				else
				{
					temp2.at<double>(j, i) = PixValue;
				}
			}
			//cout << i << endl;
		}
		temp = temp2.clone();
		Iter2 = Iter2 - 1;
	}
}