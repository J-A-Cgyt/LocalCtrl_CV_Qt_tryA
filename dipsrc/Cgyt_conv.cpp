#include "Func_Proj_2nd.h"

//���־���˵Ķ�����ʹ�ã�filter2d�ļ���ʹ���������Ǵ�������������ɲ��Ǻܸߣ��ڴ�ʵ�ַ��������װ

Mat Conv_Cgyt(Mat Src,int K_mod) //sobelģ����������ʵ�� sobel���� 20200317��Ҳ��ͼ����������Ҷȵ�һ�׵�
{
	Mat res1, res2;
	//sobel
	Mat Sobel_Zong = (Mat_<char>(3, 3) << -1, -2, -1,
										   0,  0,  0,
										   1,  2,  1);
	Mat Sobel_Heng = (Mat_<char>(3, 3) << -1, 0, 1,
										  -2, 0, 2,
										  -1, 0, 1);
	Mat Sobel_P45 = (Mat_<char>(3, 3) << -2, -1, 0,
										 -1, 0, -1,
										 0, -1, -2);
	Mat Sobel_N45 = (Mat_<char>(3, 3) << 0, -1, -2,
										-1, 0, -1,
										-2, -1, 0);
	//һ���ݶ�
	Mat Gard_Zong = (Mat_<char>(3, 3) << -1, -1, -1,
										  0,  0,  0,
										  1,  1,  1);
	Mat Gard_Heng = (Mat_<char>(3, 3) << -1, 0, 1,
										 -1, 0, 1,
										 -1, 0, 1);
	
	//Laplace����
	Mat Laplace8 = (Mat_<char>(3, 3) << -1, -1, -1,
									   -1, 8, -1,
									   -1, -1, -1);
	Mat Laplace4 = (Mat_<char>(3, 3) << 0, -1, 0,
									   -1, 4, -1,
										0, -1, 0);

	Mat end;
	vector<Mat> Chanel_Queue;
	Mat Gard_angle;
	Mat Sobel_Grad;

	//��ͬ����˵����ѡ��
	switch (K_mod)
	{
		//Sobel
		case(0):
			filter2D(Src, res1, Src.depth(), Sobel_Zong);
			filter2D(Src, res2, Src.depth(), Sobel_Heng);

			//ת��������
			cout << Src.depth() << endl;
			cout << res1.depth() << endl;
			cout << res2.depth() << endl; //�����ֵ��enum

			res1.convertTo(res1, CV_32FC1);
			res2.convertTo(res2, CV_32FC1);

			Gard_angle = Mat(Src.size(), CV_32FC1);//һ�׵����򣨻Ҷ��ݶȷ���
			for (int i = 0; i < Src.cols; i++)
			{
				for (int j = 0; j < Src.rows; j++)
				{
					Gard_angle.at<float>(j, i) = atan(res1.at<float>(j, i) / res2.at<float>(j, i));
				}
			}

			Chanel_Queue.push_back(res1);
			Chanel_Queue.push_back(res2);
			Chanel_Queue.push_back(Gard_angle);

			Sobel_Grad = Mat(Src.size(), CV_32FC3);
			merge(Chanel_Queue, Sobel_Grad);
			end = Sobel_Grad.clone();
			break;

		//laplece8
		case(1):	
			filter2D(Src, res1, Src.depth(), Laplace8);
			end = res1.clone();
			break;
		//laplace4
		case(2):
			filter2D(Src, res1, Src.depth(), Laplace4);
			end = res1.clone();
			break;
		
		//�Ҷ�һ�׵���һ���ݶȣ�
		case(3):
			filter2D(Src, res1, Src.depth(), Gard_Zong);
			filter2D(Src, res2, Src.depth(), Gard_Heng);

			//ת��������
			cout << Src.depth() << endl;
			cout << res1.depth() << endl;
			cout << res2.depth() << endl; //�����ֵ��enum

			res1.convertTo(res1, CV_32FC1);
			res2.convertTo(res2, CV_32FC1);

			Gard_angle = Mat(Src.size(), CV_32FC1);//һ�׵����򣨻Ҷ��ݶȷ���
			for (int i = 0; i < Src.cols; i++)
			{
				for (int j = 0; j < Src.rows; j++)
				{
					Gard_angle.at<float>(j, i) = atan(res1.at<float>(j, i) / res2.at<float>(j, i));
				}
			}

			Chanel_Queue.push_back(res1);
			Chanel_Queue.push_back(res2);
			Chanel_Queue.push_back(Gard_angle);

			Sobel_Grad = Mat(Src.size(), CV_32FC3);
			merge(Chanel_Queue, Sobel_Grad);
			end = Sobel_Grad.clone();		
			break;
		//б��45��Sobel
		case 4:
			filter2D(Src, res1, Src.depth(), Sobel_P45);
			filter2D(Src, res2, Src.depth(), Sobel_N45);

			//ת��������
			cout << Src.depth() << endl;
			cout << res1.depth() << endl;
			cout << res2.depth() << endl; //�����ֵ��enum

			res1.convertTo(res1, CV_32FC1);
			res2.convertTo(res2, CV_32FC1);

			Gard_angle = Mat(Src.size(), CV_32FC1);//һ�׵����򣨻Ҷ��ݶȷ���
			for (int i = 0; i < Src.cols; i++)
			{
				for (int j = 0; j < Src.rows; j++)
				{
					Gard_angle.at<float>(j, i) = atan(res1.at<float>(j, i) / res2.at<float>(j, i));
				}
			}

			Chanel_Queue.push_back(res1);
			Chanel_Queue.push_back(res2);
			Chanel_Queue.push_back(Gard_angle);

			Sobel_Grad = Mat(Src.size(), CV_32FC3);
			merge(Chanel_Queue, Sobel_Grad);
			end = Sobel_Grad.clone();
			break;
	}

	return end;
}