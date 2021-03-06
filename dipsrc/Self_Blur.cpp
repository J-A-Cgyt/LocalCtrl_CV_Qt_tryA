#include "Func_Proj_2nd.h"
string window_name_f3 = "Demo_Result"; //结果显示窗

//均值滤波器族需自行实现种类：几何均值、谐波均值、逆谐波均值 数字图像处理page203、204 
//以下两函数均为进行多线程优化，考虑到主控设备性能未知，此类滤波器主要还是作为算法验证，若投入使用，尝试进行多线程改造或GPU加速
Mat GeoMeanFilter(Mat Src, int size)  //几何均值,暂定还是CV_8UC1类型吧 注意，此函数使用float会导致越界，则此类问题只能使用double,谐波均值应也有类似的问题 果然消不掉胡椒噪声（黑点）
{
	Mat dst = Src.clone();
	double X = 0;
	double Y = 0;
	for (int i = 0; i < Src.rows; i++) {
		for (int j = 0; j < Src.cols; j++) {
			X = Src.at<uchar>(i, j);
			for (int k = i - size; k <= i + size; k++)  //连乘循环
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

Mat HarmonicMeanFilter(Mat Src, int size,double n,int Flag) //正逆谐波均值滤波器,Flag是正逆选择器,正谐波滤波器参数n无影响
{
	Mat dst = Mat(Src.rows, Src.cols, Src.type()); 
	double X = 0;
	double Y = 0;
	double Z = 0;
	if (Flag > 0) { //谐波均值，如何避免为0的问题呢
		for (int i = 0; i < Src.rows; i++) {
			for (int j = 0; j < Src.cols; j++) {
				for (int k = i - size; k <= i + size; k++)  //求和循环
				{
					for (int l = j - size; l <= j + size; l++)
					{
						if ((k < 0) || (l < 0) || (k >= Src.rows) || (l >= Src.cols)){
							continue;}
						X = X + (1 / (Src.at<uchar>(k, l)+0.0000001));  //增加数值是为了避免出现0为分母的情况
					}
				}
				Y = pow(2 * size + 1, 2) / X;
				dst.at<uchar>(i, j) = (unsigned char)Y;
				X = 0; Y = 0;
			}
		}
	}
	else //逆谐波均值
	{
		for (int i = 0; i < Src.rows; i++) {
			for (int j = 0; j < Src.cols; j++) {
				for (int k = i - size; k <= i + size; k++)  //求和循环
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

//统计排序滤波器族需自行实现的种类：最大值、最小值滤波器，中点滤波，修正的alpha均值滤波器 数字图像处理page205-207

//自适应滤波器族：自适应局部降低噪声滤波器，自适应中值滤波器 数字图像处理page208-211

Mat Gaosi_双边(Mat Src) //高斯双边滤波与锐化，保持边缘的滤波 2019/11/14,从测试与积累项目移植
{
	int d = 0;
	double Sigma_Color = 100;
	double Sigma_Space = 5;   //sigma=5为虹膜检测参数 20200530
	Mat Dst;
	bilateralFilter(Src, Dst, d, Sigma_Color, Sigma_Space);

	//锐化滤镜
	Mat Filter = (Mat_<char>(3, 3) << 0, -1, 0,
									 -1, 5, -1,
									  0, -1, 0);  //这应该就是未标定的laplace滤镜锐化

	filter2D(Dst, Dst, Dst.depth(), Filter);
	//对于噪声较多的还是考虑用强度大一点的双边滤波吧
	return Dst;
}

//非局部均值滤波 20200310 在头文件photo.hpp中 对不起，已经有现成API的就不自己写了，知道原理就行
Mat NLM_CGYT(Mat Src)
{
	Mat temp = Src.clone();
	float h = 10;
	int templateWindowSize = 7; //区块（区域分块）的大小
	int searchWindowSize = 21;  //搜索相似区块的大小
	fastNlMeansDenoising(Src, temp, h,
						 templateWindowSize, 
						 searchWindowSize);
	/*
	h : 决定过滤器强度。h 值高可以很好的去除噪声,但也会把图像的细节抹去。(取 10 的效果不错)
	hForColorComponents : 与 h 相同,但使用与彩色图像。(与 h 相同,10)
	templateWindowSize : 奇数。(推荐值为 7)
	searchWindowSize : 奇数。(推荐值为 21)
	相关原理见NoteExpress文献笔记 “基于Zernike矩亚像素的高反光金属工件缺陷检测”
	这个方法居然比自己写的还要慢，好歹是有多线程优化的吧，效果真的好，实时性真的差
	*/
	imshow(window_name_f3, temp);
	waitKey(0);
	return temp;
}

/*
各向异性扩撒滤波 启动时间20200421 ,此函数已作为多线程的实验功能通过测试，对单纯循环的遍历操作，多线程的效果可谓立竿见影，
日后若多使用多线程的方式，则逐渐考虑将各种方法都以类的形式进行封装
	其是双边滤波的简化，速度较快，功能稍弱
	对函数参数的说明：
	K：模拟热传导系数
	Lamda：求和权重
	Iter：迭代循环次数
*/
Mat Anisotropic_Cgyt(Mat Src, double K, double Lamda, int Iter) //效果存疑 似乎有种整体滤波保持边缘却锐化细节的感觉，怪怪的
{
	double Ni, Si, Wi, Ei; //四向梯度
	double Nc, Sc, Wc, Ec; //四向导热系数
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
		cout << "迭代次数K必须>0" << endl;
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
		//数值测试
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

//矩的实际研究始于 20200225，代码尝试开始 20200226
//同样是借用一个地方，用以实现ROI的Hu矩计算 在应用以前需先对ROI进行提取，或者先提取边缘 形式1，图像类型输入
int Hu_Ju_CGYT(Mat Src,double* Hu_Ju)
{
	Mat Temp;
	Temp = Src.clone();
	Moments Basic_Ju_Cgyt;
	//double Hu_Ju[7];

	Basic_Ju_Cgyt = moments(Temp);
	HuMoments(Basic_Ju_Cgyt, Hu_Ju);

	return 0; //返回指针是否会被销毁未知，此处可能有bug 果然有bug

}

//同样是借用一个地方，用以实现ROI的Hu矩计算 在应用以前需先对ROI进行提取，或者先提取边缘 形式2，边缘，点集类型输入
int Hu_Ju_CGYT(vector<Point> Contour,double* Hu_Ju)
{

	Moments Basic_Ju_Cgyt;
	//double Hu_Ju[7];

	Basic_Ju_Cgyt = moments(Contour);
	HuMoments(Basic_Ju_Cgyt, Hu_Ju);

	return 0; //返回指针是否会被销毁未知，此处可能有bug
}

/*
	Zernike矩的实现尝试，先把它算出来，然后再考虑如何应用与计算边缘的亚像素级别位置 可能需要一个单独的头文件和源文件方能完整地实现Zernike矩的计算
	
	20200227 记录
	对图像进行亚像素边缘检测可能不需要前置Canny、Sobel等算子进行像素级的边缘提取

	明确一下亚像素边缘提取的数据类型为 vector<Point2d> ~contour
	vector<vector<Point2d>> ~contours 亚像素级的结果无法在图像上体现？ 
	似乎也可，但是单元内保存的结果非像素值而是坐标的相对偏移量，相对的参考系在像素中心，向右向下为正，看看别人如何实现的吧。若是矩阵类型，则类型为 CV_64FC2

	20200229 记录
	实现目标更改为使用矩的亚像素轮廓定位实现,先实现几何矩(简单的矩)，再尝试Zernike矩

	20200302 记录
	对于矩的灰度加权模板卷积计算方法的结果仍然存疑，若要计算针对不同坐标轴的p、q阶矩，模板又该如何应用？
	是否能应用OpenCV自身携带的矩计算方法？或者应使用模板将圆外灰度置0后使用自带方法进行计算（此方法应注意坐标原点的设定与偏置）？
	必要的情况下把之前写的代码都扔进rubbish，重新尝试，对两篇论文的计算结果至少进行一个比对 
	
	20200303记录
	简单迭代解方程通过，若方法太多，则考虑建立命名空间与亚像素边缘检测的相关类了
*/
vector<Point2d> SubPixel_Contours_Cgyt(Mat Src, vector<Point> contour)
{
	Mat temp1, temp2;
	Mat Mu_Cal = Mat(temp1.size(), CV_64FC1);
	temp1 = Src.clone();
	cv::imshow(window_name_f3,temp1);
	cv::waitKey(0);
	temp1.setTo(0);

	//为了使用drawContours在外面再套一层壳
	vector<vector<Point>> Contours;	
	Contours.push_back(contour);

	//在此处记录一个普通几何矩进行亚像素检测的模板，尺寸7*7,最好整明白原理完了再算
	/*
	20200228
	猜测只要是基于矩的亚像素检测，应该都有统一的形式，计算几个不同阶数的某像素点一定范围邻域内的矩值，
	再按照一定规则将其转换为实际亚像素精度的坐标值（在计算前按理说是要归一化的，具体的方法得继续学习）
	根据论文[1]　柯洁．基于数学形态学和灰度矩的高温亚像素边缘检测[J]．工具技术．2019，53(03)：132-136．
	*/
//矩卷积计算模板设定
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
	//填充边缘轮廓并展示
	cv::drawContours(temp1,Contours,-1,Scalar::all(255),FILLED);
	cv::imshow(window_name_f3, temp1); //黑白图
	waitKey(0);

	//重新复制原图，此部分代码20200302添加，以下计算使用原图。以下计算应使用二值图还是原始灰度图存疑
	temp1 = Src.clone();
	temp1.convertTo(temp1,CV_64FC1);


	//filter2D(temp, Mu_Cal, Mu_Cal.depth(), Mask1_JHJ);
	//cout << Mu_Cal << endl;
	size_t c_s = contour.size();
	vector<vector<double>> JHJ_value; //各阶矩存储
	vector<Moments> Basic_Mu;
	vector<double> Mu_1; //1阶
	vector<double> Mu_2; //2阶
	vector<double> Mu_3; //3阶 此处计算可多线程

	Mat RoI, RoI_2, RoI_3;    //原图局部，以检测所得的边缘点为中心
	//double Temp_Mu_Value;
	
//矩计算循环 矩的计算可能需要在原始灰度图上进行（仅预处理）
	for (int i = 0; i < c_s ; i++)
	{
		//1阶矩
		RoI = temp1(Rect(contour[i].x - 3, contour[i].y - 3, 7, 7));
		Mu_1.push_back(RoI.dot(Mask1_JHJ));
		//2阶矩,灰度平方
		RoI_2 = RoI.mul(RoI);
		Mu_2.push_back(RoI_2.dot(Mask1_JHJ));
		//3阶矩，灰度立方
		RoI_3 = RoI_2.mul(RoI);
		Mu_3.push_back(RoI_3.dot(Mask1_JHJ));

		//基本的几何矩
		temp2 = Src(Rect(contour[i].x - 3, contour[i].y - 3, 7, 7));
		Basic_Mu.push_back(moments(temp2));
	}
	//再压一层
	JHJ_value.push_back(Mu_1);
	JHJ_value.push_back(Mu_2);
	JHJ_value.push_back(Mu_3);

//边缘参数计算流程
	//计算参数声明 h2,h1为灰度，sigma，s为中间参数，P_1，P_2为灰度值h1，h2所占面积的比例有关系P_1+P_2=1
	int h2;double sigma, S_Cgyt;
	int h1; //double Rho, Theta;
	double alpha;//中间参量

	vector<double> P_1, P_2,P; 
	vector<double> Theta;
	vector<double> Rho;	 //边缘最终参数，Rho为距离中心长度，Theta为偏转角度，逆时针为正

	Point2d Center_Coordinate; //重心坐标
	vector<Point2d> Contour_subPix; //结果坐标

	//边缘参数计算循环#1
	for (int i = 0; i < c_s; i++)
	{
		sigma = sqrt(Mu_2[i] - Mu_1[i] * Mu_1[i]);
		S_Cgyt = (Mu_3[i] + 2 * Mu_1[i] * Mu_1[i] * Mu_1[i] - 3 * Mu_1[i] * Mu_2[i]) / (sigma*sigma*sigma);
		
		P_1.push_back((1 + S_Cgyt * sqrt(1 / (4 + S_Cgyt * S_Cgyt))) / 2);
		P_2.push_back((1 - S_Cgyt * sqrt(1 / (4 + S_Cgyt * S_Cgyt))) / 2);
		P.push_back(MIN(P_1[i], P_2[i]));
		
		Center_Coordinate.x = (Basic_Mu[i].m10 / Basic_Mu[i].m00) - 3.5;
		Center_Coordinate.y = (Basic_Mu[i].m01 / Basic_Mu[i].m00) - 3.5; //计算重心完成后偏置至图像的重心坐标系以便计算角度theta
		//Weight_Center.push_back(Point2d(Center_Coordinate.x, Center_Coordinate.y)); 重心坐标序列压入，非必要
		
		//角度压入，为弧度制，范围[-Pi,+Pi]
		Theta.push_back(atan2(Center_Coordinate.x, Center_Coordinate.y));
	}

	//边缘参数计算循环#2
	for (int i = 0; i < c_s; i++)
	{
		alpha = Solve_Cgyt(P[i]);
		Rho.push_back(cos(alpha));
		Contour_subPix.push_back(Point2d(3.5 * Rho[i] * cos(Theta[i]) + contour[i].x, 
										 3.5 * Rho[i] * sin(Theta[i]) + contour[i].y));
	}

	return Contour_subPix;  
	/*第一个亚像素边缘检测程序（灰度矩） 20200302运行通过，因缺少另外原理的高精度测量手段，
	其精度无法验证，但是应该不会差太多，毕竟偏移量也就是0~3个像素单位。 
	若要确定其提高精度，则需要具体的手段验证，且需要布光与前置检测算法配合*/
}

double Solve_Cgyt(double P) //非线性方程组求解函数 方程为 x - 0.5*sin(2 * x) = Pi * P,使用简单迭代方法求解 20200302
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
		if (i > 1000000) //注意 P=0.5左右时误差收敛较慢估计那会整到导数为0了
		{
			cout << "迭代次数超限" << endl;
			cout << "最终误差" << Err << endl;
			return y;
		}
	}
	return y;
}

//在此处写第二个，基于正常的普通矩的亚像素边缘检测方法。使用几何矩，比较一下OpenCV自带的矩计算方法与模板计算有何区别，使用重载函数，增加一个int型的输入参数用于区分
vector<Point2d> SubPixel_Contours_Cgyt(Mat Src_2, vector<Point> Contours_2, int second) //20200408
{
	vector<Point2d> Res;
	Mat Temp = Src_2.clone();
	Temp.convertTo(Temp, CV_64FC1);
//5*5圆形矩计算模板 归一化空间矩，模板大小5*5 计算所得的L绝对值应不会超过1，但其为1时的像素长度为2.5，在下面的计算反算为像素坐标时注意倍率
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

//最外层循环
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

	//矩计算循环
	for (int i = 0; i < Contours_2.size(); i++)
	{
		RoI = Temp(Rect(Contours_2[i].x - 2, Contours_2[i].y - 2, 5, 5));
	//未经旋转前的前三借矩，由蒙版与ROI点乘直接算得
		MomentsArray[i].M_00 = RoI.dot(Moment_00);
		MomentsArray[i].M_01 = RoI.dot(Moment_01);
		MomentsArray[i].M_10 = RoI.dot(Moment_10);
		MomentsArray[i].M_11 = RoI.dot(Moment_11);
		MomentsArray[i].M_20 = RoI.dot(Moment_20);
		MomentsArray[i].M_02 = RoI.dot(Moment_02);
	//经过旋转的部分前三阶矩，根据与原始前三阶矩的关系算得
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

	//亚像素参数计算循环，主要是边缘到圆心距离L和L与x轴正方向所成角，注意此处角度应以逆时针为正，但图像坐标y轴向下为正，注意换算关系
	for (int i = 0; i < Contours_2.size(); i++)
	{
		L_Array[i] = (4 * MomentsArray[i].MR_20 - MomentsArray[i].M_00) / (3 * MomentsArray[i].MR_10);
		phi_Array[i] = atan(MomentsArray[i].M_01 / MomentsArray[i].M_10);
	}

	Res.resize(Contours_2.size());
	for (int i = 0; i < Contours_2.size(); i++)
	{
		//像素坐标x计算
		Res[i].x = double(Contours_2[i].x) + 
						  2.5 * L_Array[i] * cos(phi_Array[i]) + 
						  0.5;
		//像素坐标y计算
		Res[i].y = double(Contours_2[i].y) - 
						  2.5 * L_Array[i] * sin(phi_Array[i]) + 
						  0.5;
	}

	return Res;
}
