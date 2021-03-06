#include "Func_Proj_2nd.h"
#include <thread>

#define ViewFilter 1 //用于启动关闭频域滤波滤镜显示的宏开关

string window_name_f2 = "Demo_Result"; //结果显示窗

Mat Thershold_区域(Mat Src)
{
	//自带自适应阈值参数组
	double Max_Value = 255;  //最大值
	int Block_Size = 159;     //区块尺寸（像素值）
	int C = 5;              //均值计算以后阈值的偏移量 （-C）

	Mat Dst;
	adaptiveThreshold(Src, Dst, Max_Value, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, Block_Size, C);

	return Dst;
}

//再尝试一下傅里叶变换吧，没道理就这个函数用不了 启动20200323
Mat FT_CGYT(Mat Src,Mat &MiddleRes) //增加一个引用用于导出频域滤波的素材
{
	Mat temp = Src.clone();

	int row_opt = getOptimalDFTSize(Src.rows);
	int col_opt = getOptimalDFTSize(Src.cols);

	copyMakeBorder(Src, temp, 0, row_opt - Src.rows, 
							  0, col_opt - Src.cols, 
							  BORDER_CONSTANT, Scalar::all(0));

	Mat FT_res = Mat(temp.size(), CV_32FC2); //图像尺寸设定包含了扩展出去的边界，也就是获取了最合适的傅里叶变换尺寸
	//Mat FT_res = Mat(temp.rows + row_opt, temp.cols + col_opt, CV_32FC2); //空矩阵 注意，此处矩阵尺寸并非重要参数，设定双通道float类型的数据即可
	
	Mat mForFourier[] = { Mat_<float>(temp),//源矩阵的浮点数化通道1 
						  Mat::zeros(temp.size(), CV_32F)  //尺寸与源矩阵相同的的0矩阵通道2
	}; 
						 
	Mat FT_src;
	merge(mForFourier, 2, FT_src); //源矩阵通道合并

	dft(FT_src, FT_res); //FT_res的两个通道分别为变换结果的实部和虚部 因未舍弃任何数据，所以这里的结果包含了幅值与相角，并未损失数据，根据此还原的结果应该也是完全一样的
	MiddleRes = FT_res.clone(); //包含实部与虚部Mat 通道为CV_32FC2 注意，此处的图像并未重合中心原点（中心化）

	vector<Mat> channels_res;
	split(FT_res, channels_res);
	Mat FT_R = channels_res[0];  //实部
	Mat FT_I = channels_res[1];  //虚部

	//模长计算，（对应坐标的复数模长计算）
	Mat mAmplitude;
	magnitude(FT_R, FT_I, mAmplitude);  //幅值计算函数

	mAmplitude += Scalar(1); //取对数前不能有长度为0的像素灰度值，+1后保证所有对数值非负
	log(mAmplitude, mAmplitude); //取对数

	normalize(mAmplitude, mAmplitude, 0, 255, NORM_MINMAX); //归一化（0~255，即uchar的表示范围

	mAmplitude.convertTo(mAmplitude, CV_8UC1);  //矩阵类型转换
	imshow(window_name_f2, mAmplitude);
	waitKey(0);

	Rect Achor_cgyt[4];
	Achor_cgyt[0] = Rect(0, 0, 
						 col_opt / 2, row_opt / 2); //此构造函数为左上角坐标与长宽值，不是左上角坐标和右下角坐标
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

	copyMakeBorder(PartQ[0], PartQ[0], row_opt / 2, 0, col_opt / 2, 0, BORDER_CONSTANT, 0);   //局部扩展*4
	copyMakeBorder(PartQ[2], PartQ[2], 0, row_opt / 2, col_opt / 2, 0, BORDER_CONSTANT, 0);
	copyMakeBorder(PartQ[1], PartQ[1], row_opt / 2, 0, 0, col_opt / 2, BORDER_CONSTANT, 0);
	copyMakeBorder(PartQ[3], PartQ[3], 0, row_opt / 2, 0, col_opt / 2, BORDER_CONSTANT, 0);

	Mat Location_rev = Mat(Size(col_opt, row_opt), CV_8UC1);
	Location_rev.setTo(0);	
	
	Location_rev = PartQ[0] + PartQ[1] + PartQ[2] + PartQ[3];
	imshow(window_name_f2, Location_rev);
	waitKey(0);
	//mAmplitude.copyTo(Location_rev );

	return Location_rev; //因坐标轴变化需中心化后方可进行滤波操作，滤完以后再换回来（还是得看OpenCV自己的实现方法），频域滤波可以再写个类了
	//幅值图并未代表全部信息，若要进行频域滤波，则建议对实部和虚部同时滤波，按理说保证相角不变（零相移）的滤波器应是一个纯实数矩阵
}

//现在过来再把频域滤波补上，滤波器模板看看如何生成 这应该是主要的，对应点位相乘的操作到也还行，就是像素多了就考虑用GPU实现把，听说CUDA集成了FFT？
Mat Filter_Freq(Mat Src,unsigned char FilterType) //频域滤波的尝试 滤波器的生成其实应该类似卷积，有一套
{
	Mat SuCai;
	if (Src.type() != CV_32FC2) {
		Mat FreqPu = FT_CGYT(Src, SuCai);
	}
	else
	{
		SuCai = Src.clone();
	}
//还需要对图像进行中心重合的操作
	const int Re = 0; const int Im = 1;
	int width = Src.cols;  int cx = width / 2;
	int height = Src.rows; int cy = height / 2;
	Mat Channels[] = { Mat::zeros(SuCai.size(),CV_32FC1),Mat::zeros(SuCai.size(),CV_32FC1) };
	split(SuCai,Channels); //拆分通道
	//实部换位
	Mat Part1r(Channels[Re], Rect( 0,  0, cx, cy));
	Mat Part2r(Channels[Re], Rect(cx,  0, cx, cy));
	Mat Part3r(Channels[Re], Rect( 0, cy, cx, cy));
	Mat Part4r(Channels[Re], Rect(cx, cy, cx, cy));  //拷贝局部 copy的操作似乎联动了原始图像？
	Mat Temp;

	Part1r.copyTo(Temp);
	Part4r.copyTo(Part1r);
	Temp.copyTo(Part4r);  //左上右下换位
	
	Part2r.copyTo(Temp);
	Part3r.copyTo(Part2r);
	Temp.copyTo(Part3r);  //右下左上换位

	//虚部换位
	Mat Part1i(Channels[Im], Rect(0, 0, cx, cy));
	Mat Part2i(Channels[Im], Rect(cx, 0, cx, cy));
	Mat Part3i(Channels[Im], Rect(0, cy, cx, cy));
	Mat Part4i(Channels[Im], Rect(cx, cy, cx, cy));  //拷贝局部

	Part1i.copyTo(Temp);
	Part4i.copyTo(Part1i);
	Temp.copyTo(Part4i);  //左上右下换位

	Part2i.copyTo(Temp);
	Part3i.copyTo(Part2i);
	Temp.copyTo(Part3i);  //右下左上换位

	Mat Filter;//纯实数的滤镜，滤波器的构造还是比较麻烦的，按照中心为原点按说填充四分之一的画面元素再镜像2次应该就可了
	Filter = Filter.zeros(Src.rows,Src.cols,CV_32FC1);
	switch (FilterType) //低通滤波器族，高通就1-就完了
	{
	case 1://高斯低通滤波器，高斯频域图像 20201008高斯滤波器测试通过 走过了这个流程，剩下的就是滤波器构造的问题
	{
		float Sigma = 10.0; //可以认为是高斯函数的半径（凸起部分
		float D0 = 2 * Sigma * Sigma;
		for (int i = 0; i < Src.rows; i++) //算法写的越多，越不喜欢二重循环
		{
			for (int j = 0; j < Src.cols; j++)
			{
				float d = pow(i - Src.rows / 2, 2) + pow(j - Src.cols / 2, 2); //pow()为指数函数，pow(底,指)
				//Filter.at<Vec2f>(i, j) = Vec2f(expf(-d/D0), expf(-d / D0));
				Filter.at<float>(i, j) = expf(-d / D0);
			}
		}
		break;
	}
	
	case 2://理想低通滤波器
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

	case 3://理想带通滤波器-这个用于滤出噪声
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

	case 4://巴特沃斯低通滤波器
	{				
		const float D0 = 100;
		const int n = 4; //阶数
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
//滤镜显示
	normalize(Filter, Filter, 0, 255, NORM_MINMAX);
	Filter.convertTo(Filter, CV_8UC1);
	printf("%d",Filter.at<uchar>(450, 450));
	imshow(window_name_f2, Filter);
	waitKey(0);
#endif // DEBUG

	idft(ResMerged, ResMerged);                       //反变换,结果为复数
	split(ResMerged, res);                            //拆分通道
	magnitude(res[Re], res[Im], res[0]);              //求幅值
	normalize(res[Re], res[Re], 0, 255, NORM_MINMAX); //归一化
	res[Re].convertTo(res[Re], CV_8UC1);              //图像类型转换
	imshow(window_name_f2, res[Re]);
	waitKey(0);
	//要确定怎样的变换结果可以进行相乘滤波，改造傅里叶函数使其能接收引用吧
	return res[Re];

	//return Mat(); //无好的结果返回
}

//在此处借用有一下地方，先写个直线段检测的函数在此
vector<Vec4f> LSD_cgyt(Mat Src)
{
	Mat temp = Src.clone();
	vector<Vec4f> Lines_Decd; //检测所得线段容器
	
	//直线检测器对象实例化,注意参数选择
	Ptr<LineSegmentDetector> LSD_CGYT_Dec = createLineSegmentDetector(LSD_REFINE_STD);

	LSD_CGYT_Dec->detect(Src, Lines_Decd); //检测直线

	LSD_CGYT_Dec->drawSegments(temp, Lines_Decd); //绘制结果

	imshow(window_name_f2, temp);
	waitKey(0);

	return Lines_Decd;
	
}

Mat FenShuiLing_CGYT(Mat Src)
{
	//分水岭 20200222 20200223测试通过
	Mat Img_Gray;
	Mat Temp;
	Temp = Src.clone();
	cvtColor(Temp, Img_Gray, COLOR_BGR2GRAY); //灰度化
	imshow(window_name_f2, Img_Gray);
	waitKey(0);
	GaussianBlur(Img_Gray, Img_Gray, Size(7, 7), 2); //高斯滤波
	imshow(window_name_f2, Img_Gray);
	waitKey(0);
	Canny(Img_Gray, Img_Gray, 90, 150); //canny
	imshow(window_name_f2, Img_Gray);
	waitKey(0);

	//查找轮廓
	vector<vector<Point>> Contours_cgyt;
	vector<Vec4i> Hierarchy_cgyt;  //层次结构存放变量（其内部int型数据为轮廓容器vector<Point>的索引）
	//记录边缘结构层次的边缘查找
	findContours(Img_Gray, Contours_cgyt, Hierarchy_cgyt, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
	Mat Img_contours = Mat::zeros(Img_Gray.size(), CV_8UC1);
	Mat Img_mark = Mat(Img_Gray.size(), CV_32S);
	Img_mark = Scalar::all(0); //颜色设定
	int index = 0;
	int compCount = 0;
	for (; index >= 0; index = Hierarchy_cgyt[index][0])
	{

		//对marks进行标记，对不同区域的轮廓进行编号，相当于设置注水点，有多少轮廓，就有多少注水点 
		//marks与imageContours差别就是在颜色的赋值上，marks是不同轮廓赋予不同的值，imageContours是轮廓赋值白色

		compCount++;//具有层次结构的边缘计数	
		//带有层次结构的边缘绘制，颜色用不同强度灰度嘛 Mark
		drawContours(Img_mark, Contours_cgyt, index, Scalar::all(compCount + 1), 1, 8, Hierarchy_cgyt);

		//单纯的就是把边缘画出来，不按照层次分别着色
		drawContours(Img_contours, Contours_cgyt, index, Scalar(255), 1, 8, Hierarchy_cgyt);
	}
	Mat Marks_see;
	convertScaleAbs(Img_mark, Marks_see); //转换数据类型From CV_32S(32bit signed int) to CV_8U(8bit usigned int)
	imshow(window_name_f2, Marks_see);
	waitKey(0);
	imshow(window_name_f2, Img_contours);
	waitKey(0);

	watershed(Temp, Img_mark); //分水岭方法

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
				Img_Color_Filled.at<Vec3b>(j, i) = Vec3b(255, 255, 255); //边界用纯白绘制
			}
			else
			{
				int seed = index % 255;
				RNG random_color;  //随机数生成器
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

//对各种函数的的trackBar参数试验在以下进行 刚好callback函数也在这写呗,不过这并没有关联什么按键 估计是要有button什么的才有用
void ShowHistoCallbcak(int state,Mat Src)
{
	if (Src.type() != CV_8UC1 || Src.type() != CV_8UC3)
	{
		printf("输入矩阵格式非常规");
	}
	//剖分通道
	else if(Src.type() == CV_8UC3) //BGR
	{	
		vector< Mat> Channels;
		split(Src, Channels);

		int GrayLevels = 256;
		float range[] = { 0,256 };
		const float* histRange{ range };

		//统计
		Mat B_hist, G_hist, R_hist;
		calcHist(&Channels[0], 1,0, Mat(), B_hist, 1, &GrayLevels, &histRange);
		calcHist(&Channels[1], 1,0, Mat(), G_hist, 1, &GrayLevels, &histRange);
		calcHist(&Channels[2], 1,0, Mat(), R_hist, 1, &GrayLevels, &histRange);

		//画图
		int width = 512;
		int hight = 384;
		Mat HistGram(hight, width, CV_8UC3, Scalar(20, 20, 20));

		//归一化
		normalize(B_hist, B_hist, 0, hight, NORM_MINMAX);
		normalize(G_hist, G_hist, 0, hight, NORM_MINMAX);
		normalize(R_hist, R_hist, 0, hight, NORM_MINMAX);

		int binStep = cvRound((float)width / (float)GrayLevels);  //圆整
		for (int i = 0; i < GrayLevels; i++)
		{
			line(HistGram,  //蓝色直方图
				Point(binStep*(i-1),hight-cvRound(B_hist.at<float>(i-1))),
				Point(binStep*(i),hight-cvRound(B_hist.at<float>(i))),
				Scalar(255,0,0));
			line(HistGram,  //绿色直方图
				Point(binStep*(i - 1), hight - cvRound(G_hist.at<float>(i - 1))),
				Point(binStep*(i), hight - cvRound(G_hist.at<float>(i))),
				Scalar(0, 255, 0));
			line(HistGram,  //红色直方图
				Point(binStep*(i - 1), hight - cvRound(R_hist.at<float>(i - 1))),
				Point(binStep*(i), hight - cvRound(R_hist.at<float>(i))),
				Scalar(0, 0, 255));
		}
		imshow(window_name_f2, HistGram);
		waitKey(0);
	}

}
//增加一个直方图的计算函数，这东西我似乎写过又似乎没写过
void HistogramCGYT(Mat Src)
{
	//Mat Hist;
	int Histsize = 256;
	float range[] = { 0,255 };
	const float* histRange = range;
	Mat Hist_gray;
	calcHist(&Src, 1, 0, Mat(), Hist_gray, 1, &Histsize, &histRange);  //统计的结果是一个一维的序列，没办法用图显示，需要对统计数据进行归一化

	//cout << Hist_gray;
	int width_hist = 512;
	int height_hist = 400;
	int bin_width = width_hist / Histsize;
	Mat HistGram = Mat(height_hist, width_hist, CV_8UC3, Scalar(0, 0, 0));
	normalize(Hist_gray, Hist_gray, 0, height_hist, NORM_MINMAX, -1, Mat());  //归一化
	for (int i = 1; i < Histsize; i++)
	{
		line(HistGram, Point((i - 1)*bin_width, height_hist - cvRound(Hist_gray.at<float>(i - 1)))
			,Point(i*bin_width, height_hist - cvRound(Hist_gray.at<float>(i))),
			Scalar(255, 255, 255), 1, LINE_AA); //注意图像的坐标y轴向下为正，故此要减去高
		//cout << Point((i - 1)*bin_width, height_hist - cvRound(Hist_gray.at<float>(i - 1))) << ";";
		//cout << Point(i*bin_width, height_hist - cvRound(Hist_gray.at<float>(i))) << endl;
	}	
	imshow(window_name_f2, HistGram);
	waitKey(0);
}

//至今为止，使用模糊集合的灰度变换应该是效果最好的直方图均化方法，看看MATLAB的代码试试能不能实现吧，为加速性能事实上可以使用查找表
Vec3f Menbership_FuncI(uchar Input) //输入的隶属度函数uchar应该就能保证吧 范围必然在0-255范围
{
	float z0;
	z0 = ((float)(Input + 1)) / 256.0;
	float Ud, Ug, Ub;
	
	//分开写的话性能有点差，算了合并一下拿个记事本存一下吧	
	if (z0 <= 0.3) //0-0.3
	{
		Ud = 1;	 //暗的隶属度函数 membership function of dark
		Ug = 0;	 //灰的隶属度函数 membership function of gray
		Ub = 0;	 //亮的隶属度函数 membership function of bright
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

void MHHDBH_MT(Mat Src, Mat Res, int start, int end) //多线程执行函数 其实可执行的层级有高有低
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

Mat MoHu_HuiDuBianHuan(Mat Src)  //算了写什么多线程 改造也就是多个函数的事情 多线程还是有必要的写八个 过两天直接CUDA
{
	if (Src.type() == CV_8UC1)
	{
		Mat Dst = Src.clone();
		int cols = Src.cols; //列数
		int rows = Src.rows; //行数
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
		printf("必须是8bit单通道的灰度图像/n");
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

			//线程同步
			t1.join();	t2.join();	t3.join();	t4.join();
			t5.join();	t6.join();	t7.join();	t8.join();

			//imshow(window_name_f2, Dst);
			//waitKey(0);

			return Dst;
		}
		else
		{
			printf("必须是8bit单通道的灰度图像/n");
			return Mat();
		}
	}
	else
	{
		printf("启动数值错误，启动数值必须为1");
		return Mat();
	}
}