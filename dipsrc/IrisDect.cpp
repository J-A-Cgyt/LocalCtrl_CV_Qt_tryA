#include "Func_Proj_2nd.h"
#include <opencv2/cudaimgproc.hpp>
#include <thread>

Point3f IrisDect(Mat Src,int method)
{
//����任�ļ�ⷽ��
	if (method == 0)
	{
		int Hough_Method = HOUGH_GRADIENT;
		double dp = 1;
		double mindist = 30;
		double Param1 = 40;
		double Param2 = 45;
		int minRadius = 100;
		int maxRadius = 150;	 //��Ĥ������ 20200528
		//��������������

		vector<Vec3f> Circles;
		Mat tempA;

		//����Բ���
		tempA = Src.clone();

		HoughCircles(tempA, Circles, Hough_Method, dp, mindist, Param1, Param2, minRadius, maxRadius);
		
		Point3f CenterCord;
		//����Բ�ȿ���
		cvtColor(Src, tempA, COLOR_GRAY2BGR);
		for (int i = 0; i < Circles.size(); i++)
		{
			CenterCord.x = Circles[i].val[0];
			CenterCord.y = Circles[i].val[1];
			CenterCord.z = Circles[i].val[2];

			circle(tempA, Point(CenterCord.x, CenterCord.y),
				int(Circles[i].val[2]), Scalar(0, 0, 255), 3);
		}
		imshow("Demo_Result", tempA);
		waitKey(0);
		return CenterCord;
	}
//�������ķ���
	else if (method == 1)
	{
		Mat tempB1 = Src.clone();		
		Mat tempB2;
		//resize(tempB1,tempB1,Size(980, 740));
		//cout << tempB1.type();

		//tempB2 = AnisotropicMT_Cgyt(tempB1, 20, 0.75, 10); 
		tempB2 = Gaosi_˫��(tempB1);   //��һ���˲��ķ����ƺ�Ҳ�У�˫���˲�+�� 20200530
		cout << "end" << endl;
		imshow("Demo_Result", tempB2);
		waitKey(0);
		equalizeHist(tempB2, tempB2);  //ֱ��ͼ����
		//Laplace�Ѿ��Թ���Ч�����ã���һ����̬ѧ��
		Mat kernel = getStructuringElement(MORPH_CROSS,Size(7,7));  //��̬ѧ�����
		morphologyEx(tempB2, tempB2, MORPH_OPEN,kernel);  //��̬ѧ����
		imshow("Demo_Result", tempB2);
		waitKey(0);
		//��ֵ+�˲�
		threshold(tempB2, tempB2, 20, 255, THRESH_BINARY);
		GaussianBlur(tempB2, tempB2, Size(15, 15), 0);		
		threshold(tempB2, tempB2, 10, 255, THRESH_BINARY_INV);
		imshow("Demo_Result", tempB2);
		waitKey(0);
		//��Ե��ȡ
		vector<vector<Point>> Contours;
		vector<Point> IrisContour;	
		Moments Mu;
		float Radius; //���Բ�İ뾶
		float Pi = 3.1415926; //Բ����
		int index;
		findContours(tempB2, Contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);		
		cvtColor(Src, tempB1, COLOR_GRAY2BGR);
		drawContours(tempB1, Contours, -1, Scalar(0, 0, 255), 3);
		imshow("Demo_Result", tempB1);
		waitKey(0);
		for (int i = 0; i < Contours.size(); i++)
		{
			Mu = moments(Contours[i]);
			//cout << Mu.m20 << "," << i << endl;
			if (Mu.nu11 < 0.0001)
			{
				if (Contours[i].size() > 600 & Contours[i].size() < 800)
				{
					index = i;
					break;
				}
			}
		}
		IrisContour = Contours[index];
		Radius = sqrt(Mu.m00 / Pi);
		Point3f CenterCord;
		CenterCord.x = Mu.m10 / Mu.m00;
		CenterCord.y = Mu.m01 / Mu.m00;
		CenterCord.z = Radius;

		return CenterCord;
	}
//�ܲ��ܸ���HSVɫ�ʿռ���м����

	cout << "�������������0��1" << endl;
	return Point3f();
}
