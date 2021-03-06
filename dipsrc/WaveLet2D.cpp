#include "Func_Proj_2nd.h"

//���ļ�����ͼ��С���任����ط���ʵ��,�����İ��ȣ�����㷨ʵ�ֻ��кܶԾ��������ϸ��20200502
Mat WaveLetHarrD_CGYT(Mat Src, int LevelCount)
{	
	Mat temp = Src.clone();
	int ImgCols = temp.cols;
	int ImgRows = temp.rows;
	if (Src.type() == CV_8UC3)
	{
		vector<Mat> channels;
		split(temp, channels);
		channels[0] = WaveLetHarrD_CGYT(channels[0], 1);
		channels[1] = WaveLetHarrD_CGYT(channels[1], 1);
		channels[2] = WaveLetHarrD_CGYT(channels[2], 1);
		merge(channels, temp);
		return temp;
	}
	else if (Src.type() != CV_8UC1)
	{
		cout << "���ͱ���ΪCV_8UC1��8UC3" << endl;
		return Mat();
	}

	Mat Res = Mat(Size(ImgCols,ImgRows),CV_64FC1);
	int  Cord_res = 0;//�����궨λ�ü�������,Ҳ�ɲ��ã�
	
	//�зֽ�
	for (int row = 0; row < ImgRows; row++) //��Ҫ��һ��������������ż���ɣ�����պ����+�жϺ�resize����
	{
		for (int col = 0; col < ImgCols; col = col + 2)
		{
			//����ƽ��źţ���ֵ��
			Res.at<double>(row, col / 2) =
				(double(temp.at<uchar>(row, col) +
					temp.at<uchar>(row, col + 1))) / 2;
			//����ϸ���źţ���ֵ��
			Res.at<double>(row, (col / 2) + (ImgCols / 2)) =
				double(temp.at<uchar>(row, col) -
					temp.at<uchar>(row, col + 1));//+50Ϊ�˿�����û�б仯����ֵ����Ϊ��
		}
	}
	temp.release();
	temp = Res.clone();
	//���Դ���
	Mat map;
	temp.convertTo(map, CV_8UC1);
	imshow("Demo_Result", map);
	waitKey(0);

	//�зֽ�
	for (int col = 0; col < ImgCols; col++) //��Ҫ��һ��������������ż���ɣ�����պ����+�жϺ�resize����
	{
		for ( int row = 0; row < ImgRows;row = row + 2)
		{
			//����ƽ��źţ���ֵ��
			Res.at<double>(row / 2,col ) =
				(temp.at<double>(row, col) +
				temp.at<double>(row + 1, col)) / 2;
			//����ϸ���źţ���ֵ��
			Res.at<double>((row / 2) + (ImgRows / 2), col) =
				temp.at<double>(row, col) -
				temp.at<double>(row + 1, col);//+50Ϊ�˿�����û�б仯����ֵ����Ϊ��
		}
	}
	//�������ٶȻ�ͦ��ģ��ɴ಻д���߳���
	//normalize(Res, Res, 0, 255, NORM_MINMAX);
	Res.convertTo(Res,CV_8UC1);

	return Res;
}

Mat WaveLetHarrR_CGYT(Mat Src, int LevelCount) 
/*
harrС���Ķ�ά��任 ,���ɣ�uchar<->double������ת������ʧ�˲��ֽӽ�0�����ݣ�ʹ�ûָ������ݽ�����������δת��ΪCV_8UC1��С���ֽ�����Ϊԭʼ��������
���������ͷֽ⺯���������޸ļ��ɣ�����ֵΪCV_64FC1��CV_64FC3
*/
{
	Mat temp = Src.clone();
	int ImgCols = temp.cols;
	int ImgRows = temp.rows;
	if (Src.type() == CV_8UC3)
	{
		vector<Mat> channels;
		split(temp, channels);
		channels[0] = WaveLetHarrD_CGYT(channels[0], 1);
		channels[1] = WaveLetHarrD_CGYT(channels[1], 1);
		channels[2] = WaveLetHarrD_CGYT(channels[2], 1);
		merge(channels, temp);
		return temp;
	}
	else if (Src.type() != CV_8UC1)
	{
		cout << "���ͱ���ΪCV_8UC1��8UC3" << endl;
		return Mat();
	}
	Mat Res = Mat(Size(ImgCols, ImgRows), CV_64FC1);

	//�кϲ�
	for (int col = 0; col < ImgCols; col++) //��Ҫ��һ��������������ż���ɣ�����պ����+�жϺ�resize����
	{
		for (int row = 0; row < ImgRows; row = row + 2)
		{
			Res.at<double>(row, col) = double(
				temp.at<uchar>(row / 2, col) + temp.at<uchar>((row / 2 + ImgRows / 2), col));
			Res.at<double>(row + 1, col) = double(
				temp.at<uchar>(row / 2, col) - temp.at<uchar>((row / 2 + ImgRows / 2), col));

		}
	}

	temp.release();
	temp = Res.clone();
	//���Դ���
	Mat map;
	temp.convertTo(map, CV_8UC1);
	imshow("Demo_Result", map);
	waitKey(0);

	//�кϲ�
	for (int row = 0; row < ImgRows; row++) //��Ҫ��һ��������������ż���ɣ�����պ����+�жϺ�resize����
	{
		for (int col = 0; col < ImgCols; col = col + 2)
		{
			Res.at<double>(row, col) =
				temp.at<double>(row, col / 2) + temp.at<double>(row, (col / 2 + ImgCols / 2));
			Res.at<double>(row, col + 1) =
				temp.at<double>(row, col / 2) - temp.at<double>(row, (col / 2 + ImgCols / 2));
		}
	}
	Res.convertTo(Res, CV_8UC1);

	return Res;
}