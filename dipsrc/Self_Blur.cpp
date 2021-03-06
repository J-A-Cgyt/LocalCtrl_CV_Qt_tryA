#include "Func_Proj_2nd.h"
string window_name_f3 = "Demo_Result"; //�����ʾ��

//��ֵ�˲�����������ʵ�����ࣺ���ξ�ֵ��г����ֵ����г����ֵ ����ͼ����page203��204 
//������������Ϊ���ж��߳��Ż������ǵ������豸����δ֪�������˲�����Ҫ������Ϊ�㷨��֤����Ͷ��ʹ�ã����Խ��ж��̸߳����GPU����
Mat GeoMeanFilter(Mat Src, int size)  //���ξ�ֵ,�ݶ�����CV_8UC1���Ͱ� ע�⣬�˺���ʹ��float�ᵼ��Խ�磬���������ֻ��ʹ��double,г����ֵӦҲ�����Ƶ����� ��Ȼ�����������������ڵ㣩
{
	Mat dst = Src.clone();
	double X = 0;
	double Y = 0;
	for (int i = 0; i < Src.rows; i++) {
		for (int j = 0; j < Src.cols; j++) {
			X = Src.at<uchar>(i, j);
			for (int k = i - size; k <= i + size; k++)  //����ѭ��
			{
				for (int l = j - size; l <= j + size; l++)
				{
					if ((k < 0) || (l < 0) || (k >= Src.rows) || (l >= Src.cols))
					{continue;}
					else if ((k == i) && (l == j)){
						continue;}
					else
					{
						X = X * Src.at<uchar>(k, l);
						//cout << k << "," << l << endl;
					}
					
				}
			}
			Y = pow(X, 1 / pow(2 * size + 1, 2));
			//cout << Y<<" ";
			dst.at<uchar>(i, j) = (unsigned char)Y;
		}
	}
	//cout << dst;
	return dst;
}

Mat HarmonicMeanFilter(Mat Src, int size,double n,int Flag) //����г����ֵ�˲���,Flag������ѡ����,��г���˲�������n��Ӱ��
{
	Mat dst = Mat(Src.rows, Src.cols, Src.type()); 
	double X = 0;
	double Y = 0;
	double Z = 0;
	if (Flag > 0) { //г����ֵ����α���Ϊ0��������
		for (int i = 0; i < Src.rows; i++) {
			for (int j = 0; j < Src.cols; j++) {
				for (int k = i - size; k <= i + size; k++)  //���ѭ��
				{
					for (int l = j - size; l <= j + size; l++)
					{
						if ((k < 0) || (l < 0) || (k >= Src.rows) || (l >= Src.cols)){
							continue;}
						X = X + (1 / (Src.at<uchar>(k, l)+0.0000001));  //������ֵ��Ϊ�˱������0Ϊ��ĸ�����
					}
				}
				Y = pow(2 * size + 1, 2) / X;
				dst.at<uchar>(i, j) = (unsigned char)Y;
				X = 0; Y = 0;
			}
		}
	}
	else //��г����ֵ
	{
		for (int i = 0; i < Src.rows; i++) {
			for (int j = 0; j < Src.cols; j++) {
				for (int k = i - size; k <= i + size; k++)  //���ѭ��
				{
					for (int l = j - size; l <= j + size; l++)
					{
						if ((k < 0) || (l < 0) || (k >= Src.rows) || (l >= Src.cols)){
							continue;}
						X = X + pow(Src.at<uchar>(k, l), n + 1);
						Y = Y + pow(Src.at<uchar>(k, l), n);				
					}
				}		
				Z = X / Y;
				dst.at<uchar>(i, j) = (unsigned char)Z;
				X = 0; Y = 0;
			}
		}
	}
	return dst;
}

//ͳ�������˲�����������ʵ�ֵ����ࣺ���ֵ����Сֵ�˲������е��˲���������alpha��ֵ�˲��� ����ͼ����page205-207

//����Ӧ�˲����壺����Ӧ�ֲ����������˲���������Ӧ��ֵ�˲��� ����ͼ����page208-211

Mat Gaosi_˫��(Mat Src) //��˹˫���˲����񻯣����ֱ�Ե���˲� 2019/11/14,�Ӳ����������Ŀ��ֲ
{
	int d = 0;
	double Sigma_Color = 100;
	double Sigma_Space = 5;   //sigma=5Ϊ��Ĥ������ 20200530
	Mat Dst;
	bilateralFilter(Src, Dst, d, Sigma_Color, Sigma_Space);

	//���˾�
	Mat Filter = (Mat_<char>(3, 3) << 0, -1, 0,
									 -1, 5, -1,
									  0, -1, 0);  //��Ӧ�þ���δ�궨��laplace�˾���

	filter2D(Dst, Dst, Dst.depth(), Filter);
	//���������϶�Ļ��ǿ�����ǿ�ȴ�һ���˫���˲���
	return Dst;
}

//�Ǿֲ���ֵ�˲� 20200310 ��ͷ�ļ�photo.hpp�� �Բ����Ѿ����ֳ�API�ľͲ��Լ�д�ˣ�֪��ԭ�����
Mat NLM_CGYT(Mat Src)
{
	Mat temp = Src.clone();
	float h = 10;
	int templateWindowSize = 7; //���飨����ֿ飩�Ĵ�С
	int searchWindowSize = 21;  //������������Ĵ�С
	fastNlMeansDenoising(Src, temp, h,
						 templateWindowSize, 
						 searchWindowSize);
	/*
	h : ����������ǿ�ȡ�h ֵ�߿��Ժܺõ�ȥ������,��Ҳ���ͼ���ϸ��Ĩȥ��(ȡ 10 ��Ч������)
	hForColorComponents : �� h ��ͬ,��ʹ�����ɫͼ��(�� h ��ͬ,10)
	templateWindowSize : ������(�Ƽ�ֵΪ 7)
	searchWindowSize : ������(�Ƽ�ֵΪ 21)
	���ԭ���NoteExpress���ױʼ� ������Zernike�������صĸ߷����������ȱ�ݼ�⡱
	���������Ȼ���Լ�д�Ļ�Ҫ�����ô����ж��߳��Ż��İɣ�Ч����ĺã�ʵʱ����Ĳ�
	*/
	imshow(window_name_f3, temp);
	waitKey(0);
	return temp;
}

/*
�������������˲� ����ʱ��20200421 ,�˺�������Ϊ���̵߳�ʵ�鹦��ͨ�����ԣ��Ե���ѭ���ı������������̵߳�Ч����ν���ͼ�Ӱ��
�պ�����ʹ�ö��̵߳ķ�ʽ�����𽥿��ǽ����ַ������������ʽ���з�װ
	����˫���˲��ļ򻯣��ٶȽϿ죬��������
	�Ժ���������˵����
	K��ģ���ȴ���ϵ��
	Lamda�����Ȩ��
	Iter������ѭ������
*/
Mat Anisotropic_Cgyt(Mat Src, double K, double Lamda, int Iter) //Ч������ �ƺ����������˲����ֱ�Եȴ��ϸ�ڵĸо����ֵֹ�
{
	double Ni, Si, Wi, Ei; //�����ݶ�
	double Nc, Sc, Wc, Ec; //������ϵ��
	double PixValue;
	Mat temp = Src.clone();	
	if(temp.type()==CV_8UC1)
	{
		temp.convertTo(temp,CV_64FC1);
		/*
		cout << temp.at<double>(400, 400) << endl;
		cout << temp.at<double>(401, 400) << endl;
		cout << temp.at<double>(400, 401) << endl;
		cout << temp.at<double>(401, 401) << endl;
		*/
	}
	else if (temp.type() == CV_8UC3)
	{
		vector<Mat> channels;
		split(temp, channels);
		channels[0] = Anisotropic_Cgyt(channels[0], K, Lamda, Iter);
		channels[1] = Anisotropic_Cgyt(channels[1], K, Lamda, Iter);
		channels[2] = Anisotropic_Cgyt(channels[2], K, Lamda, Iter);
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
		cout << "��������K����>0" << endl;
		return Mat();
	}	
	Mat temp2 = temp.clone();
	double K2 = K * K;
	while (Iter > 0)
	{
		for (int i = 1; i < temp.cols - 1; i++)
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
		}
		/*
		//��ֵ����
		cout << temp2.at<double>(400, 400) << endl;
		cout << temp2.at<double>(401, 400) << endl;
		cout << temp2.at<double>(400, 401) << endl;
		cout << temp2.at<double>(401, 401) << endl;
		*/
		temp = temp2.clone();
		Iter =Iter - 1;
	}
	//normalize(temp, temp, 0, 255, NORM_MINMAX);
	temp.convertTo(temp, CV_8UC1);
	return temp;
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

//�ص�ʵ���о�ʼ�� 20200225�����볢�Կ�ʼ 20200226
//ͬ���ǽ���һ���ط�������ʵ��ROI��Hu�ؼ��� ��Ӧ����ǰ���ȶ�ROI������ȡ����������ȡ��Ե ��ʽ1��ͼ����������
int Hu_Ju_CGYT(Mat Src,double* Hu_Ju)
{
	Mat Temp;
	Temp = Src.clone();
	Moments Basic_Ju_Cgyt;
	//double Hu_Ju[7];

	Basic_Ju_Cgyt = moments(Temp);
	HuMoments(Basic_Ju_Cgyt, Hu_Ju);

	return 0; //����ָ���Ƿ�ᱻ����δ֪���˴�������bug ��Ȼ��bug

}

//ͬ���ǽ���һ���ط�������ʵ��ROI��Hu�ؼ��� ��Ӧ����ǰ���ȶ�ROI������ȡ����������ȡ��Ե ��ʽ2����Ե���㼯��������
int Hu_Ju_CGYT(vector<Point> Contour,double* Hu_Ju)
{

	Moments Basic_Ju_Cgyt;
	//double Hu_Ju[7];

	Basic_Ju_Cgyt = moments(Contour);
	HuMoments(Basic_Ju_Cgyt, Hu_Ju);

	return 0; //����ָ���Ƿ�ᱻ����δ֪���˴�������bug
}

/*
	Zernike�ص�ʵ�ֳ��ԣ��Ȱ����������Ȼ���ٿ������Ӧ��������Ե�������ؼ���λ�� ������Ҫһ��������ͷ�ļ���Դ�ļ�����������ʵ��Zernike�صļ���
	
	20200227 ��¼
	��ͼ����������ر�Ե�����ܲ���Ҫǰ��Canny��Sobel�����ӽ������ؼ��ı�Ե��ȡ

	��ȷһ�������ر�Ե��ȡ����������Ϊ vector<Point2d> ~contour
	vector<vector<Point2d>> ~contours �����ؼ��Ľ���޷���ͼ�������֣� 
	�ƺ�Ҳ�ɣ����ǵ�Ԫ�ڱ���Ľ��������ֵ������������ƫ��������ԵĲο�ϵ���������ģ���������Ϊ���������������ʵ�ֵİɡ����Ǿ������ͣ�������Ϊ CV_64FC2

	20200229 ��¼
	ʵ��Ŀ�����Ϊʹ�þص�������������λʵ��,��ʵ�ּ��ξ�(�򵥵ľ�)���ٳ���Zernike��

	20200302 ��¼
	���ھصĻҶȼ�Ȩģ�������㷽���Ľ����Ȼ���ɣ���Ҫ������Բ�ͬ�������p��q�׾أ�ģ���ָ����Ӧ�ã�
	�Ƿ���Ӧ��OpenCV����Я���ľؼ��㷽��������Ӧʹ��ģ�彫Բ��Ҷ���0��ʹ���Դ��������м��㣨�˷���Ӧע������ԭ����趨��ƫ�ã���
	��Ҫ������°�֮ǰд�Ĵ��붼�ӽ�rubbish�����³��ԣ�����ƪ���ĵļ��������ٽ���һ���ȶ� 
	
	20200303��¼
	�򵥵����ⷽ��ͨ����������̫�࣬���ǽ��������ռ��������ر�Ե�����������
*/
vector<Point2d> SubPixel_Contours_Cgyt(Mat Src, vector<Point> contour)
{
	Mat temp1, temp2;
	Mat Mu_Cal = Mat(temp1.size(), CV_64FC1);
	temp1 = Src.clone();
	cv::imshow(window_name_f3,temp1);
	cv::waitKey(0);
	temp1.setTo(0);

	//Ϊ��ʹ��drawContours����������һ���
	vector<vector<Point>> Contours;	
	Contours.push_back(contour);

	//�ڴ˴���¼һ����ͨ���ξؽ��������ؼ���ģ�壬�ߴ�7*7,���������ԭ����������
	/*
	20200228
	�²�ֻҪ�ǻ��ھص������ؼ�⣬Ӧ�ö���ͳһ����ʽ�����㼸����ͬ������ĳ���ص�һ����Χ�����ڵľ�ֵ��
	�ٰ���һ��������ת��Ϊʵ�������ؾ��ȵ�����ֵ���ڼ���ǰ����˵��Ҫ��һ���ģ�����ķ����ü���ѧϰ��
	��������[1]���½࣮������ѧ��̬ѧ�ͻҶȾصĸ��������ر�Ե���[J]�����߼�����2019��53(03)��132-136��
	*/
//�ؾ������ģ���趨
	double Omega[5];
	Omega[0] = 0;
	Omega[1] = 0.00913767235;
	Omega[2] = 0.021840193;
	Omega[3] = 0.025951560;
	Omega[4] = 0.025984481;
	Mat Mask1_JHJ = Mat(7,7,CV_64FC1);
	Mask1_JHJ.setTo(-1);

	cout << Mask1_JHJ << endl;

	Mask1_JHJ.at<double>(0, 0) = Omega[0];  Mask1_JHJ.at<double>(0, 3) = Omega[3];
	Mask1_JHJ.at<double>(0, 6) = Omega[0];  Mask1_JHJ.at<double>(3, 0) = Omega[3];
	Mask1_JHJ.at<double>(6, 0) = Omega[0];  Mask1_JHJ.at<double>(3, 6) = Omega[3];
	Mask1_JHJ.at<double>(6, 6) = Omega[0];  Mask1_JHJ.at<double>(6, 3) = Omega[3];
	
	Mask1_JHJ.at<double>(0, 1) = Omega[1];  Mask1_JHJ.at<double>(0, 2) = Omega[2];
	Mask1_JHJ.at<double>(0, 5) = Omega[1];  Mask1_JHJ.at<double>(0, 4) = Omega[2];
	Mask1_JHJ.at<double>(1, 0) = Omega[1];  Mask1_JHJ.at<double>(2, 0) = Omega[2];
	Mask1_JHJ.at<double>(1, 6) = Omega[1];  Mask1_JHJ.at<double>(2, 6) = Omega[2];
	Mask1_JHJ.at<double>(6, 1) = Omega[1];  Mask1_JHJ.at<double>(4, 0) = Omega[2];
	Mask1_JHJ.at<double>(6, 5) = Omega[1];  Mask1_JHJ.at<double>(4, 6) = Omega[2];
	Mask1_JHJ.at<double>(5, 0) = Omega[1];  Mask1_JHJ.at<double>(6, 2) = Omega[2];
	Mask1_JHJ.at<double>(5, 6) = Omega[1];  Mask1_JHJ.at<double>(6, 4) = Omega[2]; 
	//double sum = 0;
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			if (Mask1_JHJ.at<double>(j, i) == -1)
			{
				Mask1_JHJ.at<double>(j, i) = Omega[4];
			}
			//sum = sum + Mask1_JHJ.at<double>(j, i);
		}
	}
	std::cout << Mask1_JHJ << endl;
	//����Ե������չʾ
	cv::drawContours(temp1,Contours,-1,Scalar::all(255),FILLED);
	cv::imshow(window_name_f3, temp1); //�ڰ�ͼ
	waitKey(0);

	//���¸���ԭͼ���˲��ִ���20200302��ӣ����¼���ʹ��ԭͼ�����¼���Ӧʹ�ö�ֵͼ����ԭʼ�Ҷ�ͼ����
	temp1 = Src.clone();
	temp1.convertTo(temp1,CV_64FC1);


	//filter2D(temp, Mu_Cal, Mu_Cal.depth(), Mask1_JHJ);
	//cout << Mu_Cal << endl;
	size_t c_s = contour.size();
	vector<vector<double>> JHJ_value; //���׾ش洢
	vector<Moments> Basic_Mu;
	vector<double> Mu_1; //1��
	vector<double> Mu_2; //2��
	vector<double> Mu_3; //3�� �˴�����ɶ��߳�

	Mat RoI, RoI_2, RoI_3;    //ԭͼ�ֲ����Լ�����õı�Ե��Ϊ����
	//double Temp_Mu_Value;
	
//�ؼ���ѭ�� �صļ��������Ҫ��ԭʼ�Ҷ�ͼ�Ͻ��У���Ԥ����
	for (int i = 0; i < c_s ; i++)
	{
		//1�׾�
		RoI = temp1(Rect(contour[i].x - 3, contour[i].y - 3, 7, 7));
		Mu_1.push_back(RoI.dot(Mask1_JHJ));
		//2�׾�,�Ҷ�ƽ��
		RoI_2 = RoI.mul(RoI);
		Mu_2.push_back(RoI_2.dot(Mask1_JHJ));
		//3�׾أ��Ҷ�����
		RoI_3 = RoI_2.mul(RoI);
		Mu_3.push_back(RoI_3.dot(Mask1_JHJ));

		//�����ļ��ξ�
		temp2 = Src(Rect(contour[i].x - 3, contour[i].y - 3, 7, 7));
		Basic_Mu.push_back(moments(temp2));
	}
	//��ѹһ��
	JHJ_value.push_back(Mu_1);
	JHJ_value.push_back(Mu_2);
	JHJ_value.push_back(Mu_3);

//��Ե������������
	//����������� h2,h1Ϊ�Ҷȣ�sigma��sΪ�м������P_1��P_2Ϊ�Ҷ�ֵh1��h2��ռ����ı����й�ϵP_1+P_2=1
	int h2;double sigma, S_Cgyt;
	int h1; //double Rho, Theta;
	double alpha;//�м����

	vector<double> P_1, P_2,P; 
	vector<double> Theta;
	vector<double> Rho;	 //��Ե���ղ�����RhoΪ�������ĳ��ȣ�ThetaΪƫת�Ƕȣ���ʱ��Ϊ��

	Point2d Center_Coordinate; //��������
	vector<Point2d> Contour_subPix; //�������

	//��Ե��������ѭ��#1
	for (int i = 0; i < c_s; i++)
	{
		sigma = sqrt(Mu_2[i] - Mu_1[i] * Mu_1[i]);
		S_Cgyt = (Mu_3[i] + 2 * Mu_1[i] * Mu_1[i] * Mu_1[i] - 3 * Mu_1[i] * Mu_2[i]) / (sigma*sigma*sigma);
		
		P_1.push_back((1 + S_Cgyt * sqrt(1 / (4 + S_Cgyt * S_Cgyt))) / 2);
		P_2.push_back((1 - S_Cgyt * sqrt(1 / (4 + S_Cgyt * S_Cgyt))) / 2);
		P.push_back(MIN(P_1[i], P_2[i]));
		
		Center_Coordinate.x = (Basic_Mu[i].m10 / Basic_Mu[i].m00) - 3.5;
		Center_Coordinate.y = (Basic_Mu[i].m01 / Basic_Mu[i].m00) - 3.5; //����������ɺ�ƫ����ͼ�����������ϵ�Ա����Ƕ�theta
		//Weight_Center.push_back(Point2d(Center_Coordinate.x, Center_Coordinate.y)); ������������ѹ�룬�Ǳ�Ҫ
		
		//�Ƕ�ѹ�룬Ϊ�����ƣ���Χ[-Pi,+Pi]
		Theta.push_back(atan2(Center_Coordinate.x, Center_Coordinate.y));
	}

	//��Ե��������ѭ��#2
	for (int i = 0; i < c_s; i++)
	{
		alpha = Solve_Cgyt(P[i]);
		Rho.push_back(cos(alpha));
		Contour_subPix.push_back(Point2d(3.5 * Rho[i] * cos(Theta[i]) + contour[i].x, 
										 3.5 * Rho[i] * sin(Theta[i]) + contour[i].y));
	}

	return Contour_subPix;  
	/*��һ�������ر�Ե�����򣨻ҶȾأ� 20200302����ͨ������ȱ������ԭ��ĸ߾��Ȳ����ֶΣ�
	�侫���޷���֤������Ӧ�ò����̫�࣬�Ͼ�ƫ����Ҳ����0~3�����ص�λ�� 
	��Ҫȷ������߾��ȣ�����Ҫ������ֶ���֤������Ҫ������ǰ�ü���㷨���*/
}

double Solve_Cgyt(double P) //�����Է�������⺯�� ����Ϊ x - 0.5*sin(2 * x) = Pi * P,ʹ�ü򵥵���������� 20200302
{
	double Pi = 3.1415926535897935384626;
	double Err = 100;
	double x, y;
	x = 0;
	int i = 0;
	double limit; limit = pow(10, -3);
	while (Err > limit)
	{
		y = 0.5*sin(2 * x) + Pi * P;
		Err = abs(y - x);
		x = y;
		i = i + 1;
		if (i > 1000000) //ע�� P=0.5����ʱ����������������ǻ���������Ϊ0��
		{
			cout << "������������" << endl;
			cout << "�������" << Err << endl;
			return y;
		}
	}
	return y;
}

//�ڴ˴�д�ڶ�����������������ͨ�ص������ر�Ե��ⷽ����ʹ�ü��ξأ��Ƚ�һ��OpenCV�Դ��ľؼ��㷽����ģ������к�����ʹ�����غ���������һ��int�͵����������������
vector<Point2d> SubPixel_Contours_Cgyt(Mat Src_2, vector<Point> Contours_2, int second) //20200408
{
	vector<Point2d> Res;
	Mat Temp = Src_2.clone();
	Temp.convertTo(Temp, CV_64FC1);
//5*5Բ�ξؼ���ģ�� ��һ���ռ�أ�ģ���С5*5 �������õ�L����ֵӦ���ᳬ��1������Ϊ1ʱ�����س���Ϊ2.5��������ļ��㷴��Ϊ��������ʱע�ⱶ��
	Mat Moment_00 = (Mat_<double>(5, 5) <<
		0.0219, 0.1231, 0.1573, 0.1231, 0.0219,
		0.1231, 0.1600, 0.1600, 0.1600, 0.1231,
		0.1573, 0.1600, 0.1600, 0.1600, 0.1573,
		0.1231, 0.1600, 0.1600, 0.1600, 0.1231,
		0.0219, 0.1231, 0.1573, 0.1231, 0.0219);

	Mat Moment_01 = (Mat_<double>(5, 5) <<
		-0.0147, -0.0469, 0.0000, 0.0469, 0.0147,
		-0.0933, -0.0640, 0.0000, 0.0640, 0.0933,
		-0.1253, -0.0640, 0.0000, 0.0640, 0.1253,
		-0.0933, -0.0640, 0.0000, 0.0640, 0.0933,
		-0.0147, -0.0469, 0.0000, 0.0469, 0.0147);

	Mat Moment_10 = (Mat_<double>(5, 5) <<
	 	 0.0147,  0.0933,  0.1253,  0.0933,  0.0147,
		 0.0469,  0.0640,  0.0640,  0.0640,  0.0469,
		 0.0000,  0.0000,  0.0000,  0.0000,  0.0000,
		-0.0469, -0.0640, -0.0640, -0.0640, -0.0469,
		-0.0147, -0.0933, -0.1253, -0.0933, -0.0147);

	Mat Moment_11 = (Mat_<double>(5, 5) <<
		-0.0098, -0.0352, 0.0000,  0.0352,  0.0098,
		-0.0352, -0.0256, 0.0000,  0.0256,  0.0352,
	 	 0.0000,  0.0000, 0.0000,  0.0000,  0.0000,
		 0.0352,  0.0256, 0.0000, -0.0256, -0.0352,
		 0.0098,  0.0352, 0.0000, -0.0352, -0.0098);

	Mat Moment_20 = (Mat_<double>(5, 5) <<
		0.0099, 0.0194, 0.0021, 0.0194, 0.0099,
		0.0719, 0.0277, 0.0021, 0.0277, 0.0719,
		0.1019, 0.0277, 0.0021, 0.0277, 0.1019,
		0.0719, 0.0277, 0.0021, 0.0277, 0.0719,
		0.0099, 0.0194, 0.0021, 0.0194, 0.0099);

	Mat Moment_02 = (Mat_<double>(5, 5) << 
		0.0099, 0.0719, 0.1019, 0.0719, 0.0099,
		0.0194, 0.0277, 0.0277, 0.0277, 0.0194,
		0.0021, 0.0021, 0.0021, 0.0021, 0.0021,
		0.0194, 0.0277, 0.0277, 0.0277, 0.0194,
		0.0099, 0.0719, 0.1019, 0.0719, 0.0099);

//�����ѭ��
	Mat RoI;
	struct Moments_cgyt
	{
		double M_00;
		double M_01;
		double M_10;
		double M_11;
		double M_20;
		double M_02;

	   #define MR_00 M_00;
		double MR_10;
		double MR_20;
		double MR_02;
	};

	vector<Moments_cgyt> MomentsArray;
	vector<double> L_Array;
	vector<double> phi_Array;
	vector<vector<double>> SubPix_Params;

	MomentsArray.resize(Contours_2.size());
	L_Array.resize(Contours_2.size());
	phi_Array.resize(Contours_2.size());

	SubPix_Params.push_back(L_Array);
	SubPix_Params.push_back(phi_Array);

	//�ؼ���ѭ��
	for (int i = 0; i < Contours_2.size(); i++)
	{
		RoI = Temp(Rect(Contours_2[i].x - 2, Contours_2[i].y - 2, 5, 5));
	//δ����תǰ��ǰ����أ����ɰ���ROI���ֱ�����
		MomentsArray[i].M_00 = RoI.dot(Moment_00);
		MomentsArray[i].M_01 = RoI.dot(Moment_01);
		MomentsArray[i].M_10 = RoI.dot(Moment_10);
		MomentsArray[i].M_11 = RoI.dot(Moment_11);
		MomentsArray[i].M_20 = RoI.dot(Moment_20);
		MomentsArray[i].M_02 = RoI.dot(Moment_02);
	//������ת�Ĳ���ǰ���׾أ�������ԭʼǰ���׾صĹ�ϵ���
		MomentsArray[i].MR_10 = sqrt(
			MomentsArray[i].M_01*MomentsArray[i].M_01 +
			MomentsArray[i].M_10*MomentsArray[i].M_10);
		
		MomentsArray[i].MR_20 = (
			(MomentsArray[i].M_10*MomentsArray[i].M_10*MomentsArray[i].M_20 +
		 2 * MomentsArray[i].M_01*MomentsArray[i].M_11*MomentsArray[i].M_10 +
			 MomentsArray[i].M_01*MomentsArray[i].M_01*MomentsArray[i].M_02)
			/
			(MomentsArray[i].M_01*MomentsArray[i].M_01 +
			 MomentsArray[i].M_10*MomentsArray[i].M_10)
			);

		MomentsArray[i].MR_02 = (
			(MomentsArray[i].M_10*MomentsArray[i].M_10*MomentsArray[i].M_20 -
		 2 * MomentsArray[i].M_01*MomentsArray[i].M_11*MomentsArray[i].M_10 +
			 MomentsArray[i].M_01*MomentsArray[i].M_01*MomentsArray[i].M_02)
			/
			(MomentsArray[i].M_01*MomentsArray[i].M_01 +
			 MomentsArray[i].M_10*MomentsArray[i].M_10)
			);
	}

	//�����ز�������ѭ������Ҫ�Ǳ�Ե��Բ�ľ���L��L��x�����������ɽǣ�ע��˴��Ƕ�Ӧ����ʱ��Ϊ������ͼ������y������Ϊ����ע�⻻���ϵ
	for (int i = 0; i < Contours_2.size(); i++)
	{
		L_Array[i] = (4 * MomentsArray[i].MR_20 - MomentsArray[i].M_00) / (3 * MomentsArray[i].MR_10);
		phi_Array[i] = atan(MomentsArray[i].M_01 / MomentsArray[i].M_10);
	}

	Res.resize(Contours_2.size());
	for (int i = 0; i < Contours_2.size(); i++)
	{
		//��������x����
		Res[i].x = double(Contours_2[i].x) + 
						  2.5 * L_Array[i] * cos(phi_Array[i]) + 
						  0.5;
		//��������y����
		Res[i].y = double(Contours_2[i].y) - 
						  2.5 * L_Array[i] * sin(phi_Array[i]) + 
						  0.5;
	}

	return Res;
}
