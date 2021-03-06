#include "Func_Proj_2nd.h"
#include <thread>

#define ViewFilter 1 //���������ر�Ƶ���˲��˾���ʾ�ĺ꿪��

string window_name_f2 = "Demo_Result"; //�����ʾ��

Mat Thershold_����(Mat Src)
{
	//�Դ�����Ӧ��ֵ������
	double Max_Value = 255;  //���ֵ
	int Block_Size = 159;     //����ߴ磨����ֵ��
	int C = 5;              //��ֵ�����Ժ���ֵ��ƫ���� ��-C��

	Mat Dst;
	adaptiveThreshold(Src, Dst, Max_Value, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, Block_Size, C);

	return Dst;
}

//�ٳ���һ�¸���Ҷ�任�ɣ�û�������������ò��� ����20200323
Mat FT_CGYT(Mat Src,Mat &MiddleRes) //����һ���������ڵ���Ƶ���˲����ز�
{
	Mat temp = Src.clone();

	int row_opt = getOptimalDFTSize(Src.rows);
	int col_opt = getOptimalDFTSize(Src.cols);

	copyMakeBorder(Src, temp, 0, row_opt - Src.rows, 
							  0, col_opt - Src.cols, 
							  BORDER_CONSTANT, Scalar::all(0));

	Mat FT_res = Mat(temp.size(), CV_32FC2); //ͼ��ߴ��趨��������չ��ȥ�ı߽磬Ҳ���ǻ�ȡ������ʵĸ���Ҷ�任�ߴ�
	//Mat FT_res = Mat(temp.rows + row_opt, temp.cols + col_opt, CV_32FC2); //�վ��� ע�⣬�˴�����ߴ粢����Ҫ�������趨˫ͨ��float���͵����ݼ���
	
	Mat mForFourier[] = { Mat_<float>(temp),//Դ����ĸ�������ͨ��1 
						  Mat::zeros(temp.size(), CV_32F)  //�ߴ���Դ������ͬ�ĵ�0����ͨ��2
	}; 
						 
	Mat FT_src;
	merge(mForFourier, 2, FT_src); //Դ����ͨ���ϲ�

	dft(FT_src, FT_res); //FT_res������ͨ���ֱ�Ϊ�任�����ʵ�����鲿 ��δ�����κ����ݣ���������Ľ�������˷�ֵ����ǣ���δ��ʧ���ݣ����ݴ˻�ԭ�Ľ��Ӧ��Ҳ����ȫһ����
	MiddleRes = FT_res.clone(); //����ʵ�����鲿Mat ͨ��ΪCV_32FC2 ע�⣬�˴���ͼ��δ�غ�����ԭ�㣨���Ļ���

	vector<Mat> channels_res;
	split(FT_res, channels_res);
	Mat FT_R = channels_res[0];  //ʵ��
	Mat FT_I = channels_res[1];  //�鲿

	//ģ�����㣬����Ӧ����ĸ���ģ�����㣩
	Mat mAmplitude;
	magnitude(FT_R, FT_I, mAmplitude);  //��ֵ���㺯��

	mAmplitude += Scalar(1); //ȡ����ǰ�����г���Ϊ0�����ػҶ�ֵ��+1��֤���ж���ֵ�Ǹ�
	log(mAmplitude, mAmplitude); //ȡ����

	normalize(mAmplitude, mAmplitude, 0, 255, NORM_MINMAX); //��һ����0~255����uchar�ı�ʾ��Χ

	mAmplitude.convertTo(mAmplitude, CV_8UC1);  //��������ת��
	imshow(window_name_f2, mAmplitude);
	waitKey(0);

	Rect Achor_cgyt[4];
	Achor_cgyt[0] = Rect(0, 0, 
						 col_opt / 2, row_opt / 2); //�˹��캯��Ϊ���Ͻ������볤��ֵ���������Ͻ���������½�����
	Achor_cgyt[1] = Rect(col_opt / 2, 0,
						 col_opt / 2, row_opt / 2);
	Achor_cgyt[2] = Rect(0, row_opt / 2,
						 col_opt / 2, row_opt / 2);
	Achor_cgyt[3] = Rect(col_opt / 2, row_opt / 2,
						 col_opt / 2, row_opt / 2);
	Mat PartQ[4];
	for (int i = 0; i < 4; i++)
	{
		PartQ[i] = mAmplitude(Achor_cgyt[i]);
		//imshow(window_name_f2, PartQ[i]);
		//waitKey(0);
	}

	copyMakeBorder(PartQ[0], PartQ[0], row_opt / 2, 0, col_opt / 2, 0, BORDER_CONSTANT, 0);   //�ֲ���չ*4
	copyMakeBorder(PartQ[2], PartQ[2], 0, row_opt / 2, col_opt / 2, 0, BORDER_CONSTANT, 0);
	copyMakeBorder(PartQ[1], PartQ[1], row_opt / 2, 0, 0, col_opt / 2, BORDER_CONSTANT, 0);
	copyMakeBorder(PartQ[3], PartQ[3], 0, row_opt / 2, 0, col_opt / 2, BORDER_CONSTANT, 0);

	Mat Location_rev = Mat(Size(col_opt, row_opt), CV_8UC1);
	Location_rev.setTo(0);	
	
	Location_rev = PartQ[0] + PartQ[1] + PartQ[2] + PartQ[3];
	imshow(window_name_f2, Location_rev);
	waitKey(0);
	//mAmplitude.copyTo(Location_rev );

	return Location_rev; //��������仯�����Ļ��󷽿ɽ����˲������������Ժ��ٻ����������ǵÿ�OpenCV�Լ���ʵ�ַ�������Ƶ���˲�������д������
	//��ֵͼ��δ����ȫ����Ϣ����Ҫ����Ƶ���˲��������ʵ�����鲿ͬʱ�˲�������˵��֤��ǲ��䣨�����ƣ����˲���Ӧ��һ����ʵ������
}

//���ڹ����ٰ�Ƶ���˲����ϣ��˲���ģ�忴��������� ��Ӧ������Ҫ�ģ���Ӧ��λ��˵Ĳ�����Ҳ���У��������ض��˾Ϳ�����GPUʵ�ְѣ���˵CUDA������FFT��
Mat Filter_Freq(Mat Src,unsigned char FilterType) //Ƶ���˲��ĳ��� �˲�����������ʵӦ�����ƾ������һ��
{
	Mat SuCai;
	if (Src.type() != CV_32FC2) {
		Mat FreqPu = FT_CGYT(Src, SuCai);
	}
	else
	{
		SuCai = Src.clone();
	}
//����Ҫ��ͼ����������غϵĲ���
	const int Re = 0; const int Im = 1;
	int width = Src.cols;  int cx = width / 2;
	int height = Src.rows; int cy = height / 2;
	Mat Channels[] = { Mat::zeros(SuCai.size(),CV_32FC1),Mat::zeros(SuCai.size(),CV_32FC1) };
	split(SuCai,Channels); //���ͨ��
	//ʵ����λ
	Mat Part1r(Channels[Re], Rect( 0,  0, cx, cy));
	Mat Part2r(Channels[Re], Rect(cx,  0, cx, cy));
	Mat Part3r(Channels[Re], Rect( 0, cy, cx, cy));
	Mat Part4r(Channels[Re], Rect(cx, cy, cx, cy));  //�����ֲ� copy�Ĳ����ƺ�������ԭʼͼ��
	Mat Temp;

	Part1r.copyTo(Temp);
	Part4r.copyTo(Part1r);
	Temp.copyTo(Part4r);  //�������»�λ
	
	Part2r.copyTo(Temp);
	Part3r.copyTo(Part2r);
	Temp.copyTo(Part3r);  //�������ϻ�λ

	//�鲿��λ
	Mat Part1i(Channels[Im], Rect(0, 0, cx, cy));
	Mat Part2i(Channels[Im], Rect(cx, 0, cx, cy));
	Mat Part3i(Channels[Im], Rect(0, cy, cx, cy));
	Mat Part4i(Channels[Im], Rect(cx, cy, cx, cy));  //�����ֲ�

	Part1i.copyTo(Temp);
	Part4i.copyTo(Part1i);
	Temp.copyTo(Part4i);  //�������»�λ

	Part2i.copyTo(Temp);
	Part3i.copyTo(Part2i);
	Temp.copyTo(Part3i);  //�������ϻ�λ

	Mat Filter;//��ʵ�����˾����˲����Ĺ��컹�ǱȽ��鷳�ģ���������Ϊԭ�㰴˵����ķ�֮һ�Ļ���Ԫ���پ���2��Ӧ�þͿ���
	Filter = Filter.zeros(Src.rows,Src.cols,CV_32FC1);
	switch (FilterType) //��ͨ�˲����壬��ͨ��1-������
	{
	case 1://��˹��ͨ�˲�������˹Ƶ��ͼ�� 20201008��˹�˲�������ͨ�� �߹���������̣�ʣ�µľ����˲������������
	{
		float Sigma = 10.0; //������Ϊ�Ǹ�˹�����İ뾶��͹�𲿷�
		float D0 = 2 * Sigma * Sigma;
		for (int i = 0; i < Src.rows; i++) //�㷨д��Խ�࣬Խ��ϲ������ѭ��
		{
			for (int j = 0; j < Src.cols; j++)
			{
				float d = pow(i - Src.rows / 2, 2) + pow(j - Src.cols / 2, 2); //pow()Ϊָ��������pow(��,ָ)
				//Filter.at<Vec2f>(i, j) = Vec2f(expf(-d/D0), expf(-d / D0));
				Filter.at<float>(i, j) = expf(-d / D0);
			}
		}
		break;
	}
	
	case 2://�����ͨ�˲���
	{	
		float Radius = 100;
		for (int i = 0; i < Filter.rows; i++)	
		{
			for (int j = 0; j < Filter.cols; j++)
			{
				float d = pow(i - Src.rows / 2, 2) + pow(j - Src.cols / 2, 2);
				float Radius2 = pow(Radius, 2);
				if (d > Radius2)
				{
					Filter.at<float>(i, j) = 0.0;
				}
				else
				{
					Filter.at<float>(i, j) = 1.0;
				}
			}	
		}
		break; 
	}

	case 3://�����ͨ�˲���-��������˳�����
	{
		const float RadiusBig = 100;
		const float RadiusSmall = 80;
		for (int i = 0; i < Filter.rows; i++)
		{
			for (int j = 0; j < Filter.cols; j++)
			{
				float d = pow(i - Src.rows / 2, 2) + pow(j - Src.cols / 2, 2);
				float RadiusB2 = pow(RadiusBig, 2);
				float RadiusS2 = pow(RadiusSmall, 2);
				if ((d > RadiusS2) && (d<RadiusB2))
				{
					Filter.at<float>(i, j) = 1.0;
				}
				else
				{
					Filter.at<float>(i, j) = 0.0;
				}
			}
		}
		break;
	}

	case 4://������˹��ͨ�˲���
	{				
		const float D0 = 100;
		const int n = 4; //����
		for (int i = 0; i < Filter.rows; i++)
		{
			for (int j = 0; j < Filter.cols; j++)
			{
				float D2 = pow(i - Filter.rows / 2, 2) + pow(j - Filter.cols / 2, 2);
				float D02 = D0 * D0;
				float K = 0;
				K = pow(D2 / D02, n);
				Filter.at<float>(i, j) = 1 / (1 + K);
			}
		}
		break;
	}
	}

	Mat res[] = { Mat::zeros(SuCai.size(),CV_32FC1),Mat::zeros(SuCai.size(),CV_32FC1) };
	multiply(Channels[Re], Filter, res[Re]);
	multiply(Channels[Im], Filter, res[Im]);
	Mat ResMerged;
	merge(res, 2, ResMerged);

#if ViewFilter == 0
//�˾���ʾ
	normalize(Filter, Filter, 0, 255, NORM_MINMAX);
	Filter.convertTo(Filter, CV_8UC1);
	printf("%d",Filter.at<uchar>(450, 450));
	imshow(window_name_f2, Filter);
	waitKey(0);
#endif // DEBUG

	idft(ResMerged, ResMerged);                       //���任,���Ϊ����
	split(ResMerged, res);                            //���ͨ��
	magnitude(res[Re], res[Im], res[0]);              //���ֵ
	normalize(res[Re], res[Re], 0, 255, NORM_MINMAX); //��һ��
	res[Re].convertTo(res[Re], CV_8UC1);              //ͼ������ת��
	imshow(window_name_f2, res[Re]);
	waitKey(0);
	//Ҫȷ�������ı任������Խ�������˲������층��Ҷ����ʹ���ܽ������ð�
	return res[Re];

	//return Mat(); //�޺õĽ������
}

//�ڴ˴�������һ�µط�����д��ֱ�߶μ��ĺ����ڴ�
vector<Vec4f> LSD_cgyt(Mat Src)
{
	Mat temp = Src.clone();
	vector<Vec4f> Lines_Decd; //��������߶�����
	
	//ֱ�߼��������ʵ����,ע�����ѡ��
	Ptr<LineSegmentDetector> LSD_CGYT_Dec = createLineSegmentDetector(LSD_REFINE_STD);

	LSD_CGYT_Dec->detect(Src, Lines_Decd); //���ֱ��

	LSD_CGYT_Dec->drawSegments(temp, Lines_Decd); //���ƽ��

	imshow(window_name_f2, temp);
	waitKey(0);

	return Lines_Decd;
	
}

Mat FenShuiLing_CGYT(Mat Src)
{
	//��ˮ�� 20200222 20200223����ͨ��
	Mat Img_Gray;
	Mat Temp;
	Temp = Src.clone();
	cvtColor(Temp, Img_Gray, COLOR_BGR2GRAY); //�ҶȻ�
	imshow(window_name_f2, Img_Gray);
	waitKey(0);
	GaussianBlur(Img_Gray, Img_Gray, Size(7, 7), 2); //��˹�˲�
	imshow(window_name_f2, Img_Gray);
	waitKey(0);
	Canny(Img_Gray, Img_Gray, 90, 150); //canny
	imshow(window_name_f2, Img_Gray);
	waitKey(0);

	//��������
	vector<vector<Point>> Contours_cgyt;
	vector<Vec4i> Hierarchy_cgyt;  //��νṹ��ű��������ڲ�int������Ϊ��������vector<Point>��������
	//��¼��Ե�ṹ��εı�Ե����
	findContours(Img_Gray, Contours_cgyt, Hierarchy_cgyt, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
	Mat Img_contours = Mat::zeros(Img_Gray.size(), CV_8UC1);
	Mat Img_mark = Mat(Img_Gray.size(), CV_32S);
	Img_mark = Scalar::all(0); //��ɫ�趨
	int index = 0;
	int compCount = 0;
	for (; index >= 0; index = Hierarchy_cgyt[index][0])
	{

		//��marks���б�ǣ��Բ�ͬ������������б�ţ��൱������עˮ�㣬�ж������������ж���עˮ�� 
		//marks��imageContours����������ɫ�ĸ�ֵ�ϣ�marks�ǲ�ͬ�������費ͬ��ֵ��imageContours��������ֵ��ɫ

		compCount++;//���в�νṹ�ı�Ե����	
		//���в�νṹ�ı�Ե���ƣ���ɫ�ò�ͬǿ�ȻҶ��� Mark
		drawContours(Img_mark, Contours_cgyt, index, Scalar::all(compCount + 1), 1, 8, Hierarchy_cgyt);

		//�����ľ��ǰѱ�Ե�������������ղ�ηֱ���ɫ
		drawContours(Img_contours, Contours_cgyt, index, Scalar(255), 1, 8, Hierarchy_cgyt);
	}
	Mat Marks_see;
	convertScaleAbs(Img_mark, Marks_see); //ת����������From CV_32S(32bit signed int) to CV_8U(8bit usigned int)
	imshow(window_name_f2, Marks_see);
	waitKey(0);
	imshow(window_name_f2, Img_contours);
	waitKey(0);

	watershed(Temp, Img_mark); //��ˮ�뷽��

	Mat Mark_after_fsl;
	convertScaleAbs(Img_mark, Mark_after_fsl);
	imshow(window_name_f2, Mark_after_fsl);
	waitKey(0);

	Mat Img_Color_Filled = Mat::zeros(Temp.size(), CV_8UC3);
	for (int i = 0; i < Img_mark.cols; i++)
	{
		for (int j = 0; j < Img_mark.rows; j++)
		{
			int index = Img_mark.at<int>(j, i);
			if (Img_mark.at<int>(j, i) == -1)
			{
				Img_Color_Filled.at<Vec3b>(j, i) = Vec3b(255, 255, 255); //�߽��ô��׻���
			}
			else
			{
				int seed = index % 255;
				RNG random_color;  //�����������
				Img_Color_Filled.at<Vec3b>(j, i)[0] = random_color.uniform(0, seed);
				Img_Color_Filled.at<Vec3b>(j, i)[1] = random_color.uniform(0, seed);
				Img_Color_Filled.at<Vec3b>(j, i)[2] = random_color.uniform(0, seed);
			}
		}
	}
	imshow(window_name_f2, Img_Color_Filled);
	waitKey(0);

	return Img_mark;
}

//�Ը��ֺ����ĵ�trackBar�������������½��� �պ�callback����Ҳ����д��,�����Ⲣû�й���ʲô���� ������Ҫ��buttonʲô�Ĳ�����
void ShowHistoCallbcak(int state,Mat Src)
{
	if (Src.type() != CV_8UC1 || Src.type() != CV_8UC3)
	{
		printf("��������ʽ�ǳ���");
	}
	//�ʷ�ͨ��
	else if(Src.type() == CV_8UC3) //BGR
	{	
		vector< Mat> Channels;
		split(Src, Channels);

		int GrayLevels = 256;
		float range[] = { 0,256 };
		const float* histRange{ range };

		//ͳ��
		Mat B_hist, G_hist, R_hist;
		calcHist(&Channels[0], 1,0, Mat(), B_hist, 1, &GrayLevels, &histRange);
		calcHist(&Channels[1], 1,0, Mat(), G_hist, 1, &GrayLevels, &histRange);
		calcHist(&Channels[2], 1,0, Mat(), R_hist, 1, &GrayLevels, &histRange);

		//��ͼ
		int width = 512;
		int hight = 384;
		Mat HistGram(hight, width, CV_8UC3, Scalar(20, 20, 20));

		//��һ��
		normalize(B_hist, B_hist, 0, hight, NORM_MINMAX);
		normalize(G_hist, G_hist, 0, hight, NORM_MINMAX);
		normalize(R_hist, R_hist, 0, hight, NORM_MINMAX);

		int binStep = cvRound((float)width / (float)GrayLevels);  //Բ��
		for (int i = 0; i < GrayLevels; i++)
		{
			line(HistGram,  //��ɫֱ��ͼ
				Point(binStep*(i-1),hight-cvRound(B_hist.at<float>(i-1))),
				Point(binStep*(i),hight-cvRound(B_hist.at<float>(i))),
				Scalar(255,0,0));
			line(HistGram,  //��ɫֱ��ͼ
				Point(binStep*(i - 1), hight - cvRound(G_hist.at<float>(i - 1))),
				Point(binStep*(i), hight - cvRound(G_hist.at<float>(i))),
				Scalar(0, 255, 0));
			line(HistGram,  //��ɫֱ��ͼ
				Point(binStep*(i - 1), hight - cvRound(R_hist.at<float>(i - 1))),
				Point(binStep*(i), hight - cvRound(R_hist.at<float>(i))),
				Scalar(0, 0, 255));
		}
		imshow(window_name_f2, HistGram);
		waitKey(0);
	}

}
//����һ��ֱ��ͼ�ļ��㺯�����ⶫ�����ƺ�д�����ƺ�ûд��
void HistogramCGYT(Mat Src)
{
	//Mat Hist;
	int Histsize = 256;
	float range[] = { 0,255 };
	const float* histRange = range;
	Mat Hist_gray;
	calcHist(&Src, 1, 0, Mat(), Hist_gray, 1, &Histsize, &histRange);  //ͳ�ƵĽ����һ��һά�����У�û�취��ͼ��ʾ����Ҫ��ͳ�����ݽ��й�һ��

	//cout << Hist_gray;
	int width_hist = 512;
	int height_hist = 400;
	int bin_width = width_hist / Histsize;
	Mat HistGram = Mat(height_hist, width_hist, CV_8UC3, Scalar(0, 0, 0));
	normalize(Hist_gray, Hist_gray, 0, height_hist, NORM_MINMAX, -1, Mat());  //��һ��
	for (int i = 1; i < Histsize; i++)
	{
		line(HistGram, Point((i - 1)*bin_width, height_hist - cvRound(Hist_gray.at<float>(i - 1)))
			,Point(i*bin_width, height_hist - cvRound(Hist_gray.at<float>(i))),
			Scalar(255, 255, 255), 1, LINE_AA); //ע��ͼ�������y������Ϊ�����ʴ�Ҫ��ȥ��
		//cout << Point((i - 1)*bin_width, height_hist - cvRound(Hist_gray.at<float>(i - 1))) << ";";
		//cout << Point(i*bin_width, height_hist - cvRound(Hist_gray.at<float>(i))) << endl;
	}	
	imshow(window_name_f2, HistGram);
	waitKey(0);
}

//����Ϊֹ��ʹ��ģ�����ϵĻҶȱ任Ӧ����Ч����õ�ֱ��ͼ��������������MATLAB�Ĵ��������ܲ���ʵ�ְɣ�Ϊ����������ʵ�Ͽ���ʹ�ò��ұ�
Vec3f Menbership_FuncI(uchar Input) //����������Ⱥ���ucharӦ�þ��ܱ�֤�� ��Χ��Ȼ��0-255��Χ
{
	float z0;
	z0 = ((float)(Input + 1)) / 256.0;
	float Ud, Ug, Ub;
	
	//�ֿ�д�Ļ������е����˺ϲ�һ���ø����±���һ�°�	
	if (z0 <= 0.3) //0-0.3
	{
		Ud = 1;	 //���������Ⱥ��� membership function of dark
		Ug = 0;	 //�ҵ������Ⱥ��� membership function of gray
		Ub = 0;	 //���������Ⱥ��� membership function of bright
	}
	else if ((z0 > 0.3) & (z0 <= 0.5))  //0.3-0.5
	{
		Ud = (-5.0)*(z0 - 0.5);
		Ug = (5.0)*(z0 - 0.3);
		Ub = 0;
	}
	else if ((z0 > 0.5) & (z0 <= 0.7))  //0.5-0.7
	{
		Ud = 0;
		Ug = (-5.0)*(z0 - 0.7);
		Ub = (5.0)*(z0 - 0.5);
	}
	else //0.7-1
	{
		Ud = 0;
		Ug = 0;
		Ub = 1;
	}
	Vec3f res;
	res[0] = Ud;
	res[1] = Ug;
	res[2] = Ub;
	
	return res;
}

void MHHDBH_MT(Mat Src, Mat Res, int start, int end) //���߳�ִ�к��� ��ʵ��ִ�еĲ㼶�и��е�
{
	int cols = Src.cols;
	Vec3f U;
	uchar Input, v0;
	float v0d;

	for (int ROW = start; ROW < end; ROW++)
	{
		for (int COL = 0; COL < cols; COL++)
		{
			Input = Src.at<uchar>(ROW, COL);
			U = Menbership_FuncI(Input);
			v0d = (U[0] * 0.0 + U[1] * 127.0 + U[2] * 255) / (U[0] + U[1] + U[2]);
			v0 = (unsigned char)v0d;
			Res.at<uchar>(ROW, COL) = v0;
		}
	}
}

Mat MoHu_HuiDuBianHuan(Mat Src)  //����дʲô���߳� ����Ҳ���Ƕ������������ ���̻߳����б�Ҫ��д�˸� ������ֱ��CUDA
{
	if (Src.type() == CV_8UC1)
	{
		Mat Dst = Src.clone();
		int cols = Src.cols; //����
		int rows = Src.rows; //����
		Vec3f U;
		uchar Input, v0;
		float v0d;

		for (int ROW = 0; ROW < rows; ROW++)
		{
			for (int COL = 0; COL < cols; COL++)
			{
				Input = Src.at<uchar>(ROW, COL);
				U = Menbership_FuncI(Input);
				v0d = (U[0] * 0.0 + U[1] * 127.0 + U[2] * 255) / (U[0] + U[1] + U[2]);
				v0 = (unsigned char)v0d;
				Dst.at<uchar>(ROW, COL) = v0;
			}
		}

		//imshow(window_name_f2, Dst);
		//waitKey(0);

		return Dst;
	}
	else
	{
		printf("������8bit��ͨ���ĻҶ�ͼ��/n");
		return Mat();
	}
}

Mat MoHu_HuiDuBianHuan(Mat Src,int MT)
{
	const int NumOfThreads = 8;



	if (MT == 1)
	{
		if (Src.type() == CV_8UC1)
		{
			Mat Dst = Src.clone();
			int bound = Src.rows / NumOfThreads;
			thread t1 = thread(MHHDBH_MT, Src, Dst,             0,     bound);
			thread t2 = thread(MHHDBH_MT, Src, Dst,     bound, 2 * bound);
			thread t3 = thread(MHHDBH_MT, Src, Dst, 2 * bound, 3 * bound);
			thread t4 = thread(MHHDBH_MT, Src, Dst, 3 * bound, 4 * bound);
			thread t5 = thread(MHHDBH_MT, Src, Dst, 4 * bound, 5 * bound);
			thread t6 = thread(MHHDBH_MT, Src, Dst, 5 * bound, 6 * bound);
			thread t7 = thread(MHHDBH_MT, Src, Dst, 6 * bound, 7 * bound);
			thread t8 = thread(MHHDBH_MT, Src, Dst, 7 * bound,  Src.rows);

			//�߳�ͬ��
			t1.join();	t2.join();	t3.join();	t4.join();
			t5.join();	t6.join();	t7.join();	t8.join();

			//imshow(window_name_f2, Dst);
			//waitKey(0);

			return Dst;
		}
		else
		{
			printf("������8bit��ͨ���ĻҶ�ͼ��/n");
			return Mat();
		}
	}
	else
	{
		printf("������ֵ����������ֵ����Ϊ1");
		return Mat();
	}
}