#include "Func_Proj_2nd.h"

#include <opencv2/objdetect/objdetect.hpp>  //对象检测的头文件

string window_name_f1 = "Demo_Result"; //结果显示窗

Mat FAST_dect_Cgyt(Mat Src1, Mat Src2)
{

	std::vector<Mat> Temp_Array_f1;
	Mat Temp_Buffer_f1;
	Mat temp_src1, temp_src2;
	temp_src1 = Src1.clone();
	temp_src2 = Src2.clone();
	
	Temp_Array_f1.push_back(temp_src1);
	Temp_Array_f1.push_back(temp_src2);

//FAST角点检测测试代码+匹配
	//角点检测	2020-1-17测试通过
	std::vector<cv::KeyPoint> 角点A;
	std::vector<cv::KeyPoint> 角点B;

	Ptr<FastFeatureDetector> Fast_Test_A = FastFeatureDetector::create(230);
	Fast_Test_A->detect(Temp_Array_f1[0], 角点A);
	Ptr<FastFeatureDetector> Fast_Test_B = FastFeatureDetector::create(230);
	Fast_Test_B->detect(Temp_Array_f1[1], 角点B);

//角点检测结果展示	2020-1-17测试通过
	int numbers = 角点A.size();
	cout << numbers << endl;
	/*
	for (int i = 0; i < numbers; i++)
	{
		cout << 角点A[i].pt.x << "," << 角点A[i].pt.y << endl;
	}
	*/
	Temp_Buffer_f1 = Temp_Array_f1[0].clone();
	cvtColor(Temp_Buffer_f1, Temp_Buffer_f1, COLOR_GRAY2BGR);
	const Scalar &color = cv::Scalar_<double>::all(-1);
	drawKeypoints(Temp_Array_f1[0], 角点A, Temp_Buffer_f1, color, DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	imshow(window_name_f1, Temp_Buffer_f1);
	waitKey(0);

	numbers = 角点B.size();
	cout << numbers << endl;
	/*
	for (int i = 0; i < numbers; i++)
	{
		cout << 角点B[i].pt.x << "," << 角点B[i].pt.y << endl;
	}
	*/
	Temp_Buffer_f1 = Temp_Array_f1[1].clone();
	cvtColor(Temp_Buffer_f1, Temp_Buffer_f1, COLOR_GRAY2BGR);
	drawKeypoints(Temp_Array_f1[1], 角点B, Temp_Buffer_f1, color, DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	imshow(window_name_f1, Temp_Buffer_f1);
	waitKey(0);

//角点描述子计算，KeyPoint Descriptor Brief方法描述
	Mat 描述子_1, 描述子_2;
	Ptr<ORB> Brief_Descriptor_compute = ORB::create();
	Brief_Descriptor_compute->compute(Src1, 角点A, 描述子_1);
	Brief_Descriptor_compute->compute(Src2, 角点B, 描述子_2);

	//cout << 描述子_1 << endl;

	描述子_1.convertTo(描述子_1,CV_32F);
	描述子_2.convertTo(描述子_2,CV_32F); //数据类型转换用以匹配

	//cout << 描述子_1.type() << endl;
	//imwrite("G:\\Pictures\\Test For Programming\\keypoints.jpg",Temp_Buffer);
	//Ptr<FeatureDetector> Feature_Test;

//KNN方法匹配计算匹配关系
	FlannBasedMatcher Matcher_FAST_cgyt;
	vector<vector<DMatch>> 匹配关系;
	vector<DMatch> Select_Match;
	//DMatch Match_buffer;

	int k = 2; //number of nearest neighbors

	Matcher_FAST_cgyt.knnMatch(描述子_1, 描述子_2, 匹配关系, k, Mat()); 
	/*
		问题：描述子类型与函数不匹配 描述子类型需为 CV_32F 即单通道float型。
		参数 int k 表示针对同一组图像，对参考图（原图）的同一个点返回 k 个可能的配对点（位于匹配图），K-NN的字面意思
	*/
	//cout << 匹配关系.size() << endl;

	for (int i = 1; i < 匹配关系.size(); i++)
	{
		Select_Match.push_back(匹配关系[i][0]);
	}
	std::nth_element(Select_Match.begin(), Select_Match.begin() + 49, Select_Match.end()); //自动排序
	Select_Match.erase(Select_Match.begin() + 49, Select_Match.end());

	Mat result_match;
	drawMatches(temp_src1,角点A,temp_src2,角点B,匹配关系,result_match);
	imshow(window_name_f1, result_match);
	waitKey(0);		
	
	drawMatches(temp_src1, 角点A, temp_src2, 角点B, Select_Match, result_match);
	imshow(window_name_f1, result_match);
	waitKey(0);

	return Temp_Buffer_f1;
}

Mat JiaoDian_SURF_CGYT(Mat Src1, Mat Src2) //surf角点检测代码
{
	Mat temp1, temp2;
	temp1 = Src1.clone();
	temp2 = Src2.clone();

	Ptr<xfeatures2d::SurfFeatureDetector> Dect1_Surf1 = xfeatures2d::SURF::create(30000); //特征检测器
	vector<KeyPoint> KeyPoints_1_Q, KeyPoints_2_T; //特征点容器

	Mat descriptor1_Q, descriptor2_T; //描述子

	Dect1_Surf1->detectAndCompute(Src1, Mat(), KeyPoints_1_Q, descriptor1_Q);
	Dect1_Surf1->detectAndCompute(Src2, Mat(), KeyPoints_2_T, descriptor2_T); //特征点检测与对应描述子计算

	drawKeypoints(temp1, KeyPoints_1_Q, temp1);
	drawKeypoints(temp2, KeyPoints_2_T, temp2);  //特征点绘制

	imshow(window_name_f1, temp1);
	waitKey(0);
	imshow(window_name_f1, temp2);
	waitKey(0);

	//cout << descriptor1_Q.type();// debug fast 20200225
	//暴力匹配
	BFMatcher match_cgyt;
	vector<DMatch> matchs_cgyt; 
	//配对关系类，包含相关性与两图像对应关键点的序号，其分主次，源为quary，次为train目标是将train图变换至quary图
	match_cgyt.match(descriptor1_Q, descriptor2_T, matchs_cgyt);
	Mat Result;

	/* 画出所有的匹配点
	drawMatches(temp1, KeyPoints_1, temp2, KeyPoints_2, matchs_cgyt,Result);
	imshow(window_name_f1, Result);
	waitKey(0);
	*/

	//排序前60个匹配点（值小的，依据为Dmatch.distance,从小到大排）
	std::nth_element(matchs_cgyt.begin(), matchs_cgyt.begin() + 59, matchs_cgyt.end());
	matchs_cgyt.erase(matchs_cgyt.begin() + 59, matchs_cgyt.end());
	drawMatches(temp1, KeyPoints_1_Q, temp2, KeyPoints_2_T, matchs_cgyt, Result);

	imshow(window_name_f1, Result); //筛选匹配点后的匹配结果
	waitKey(0);

	vector<Point2f> Point_CPs_Q, Point_CPs_T; //对应关键点坐标（浮点型）
	for (int i = 0; i < matchs_cgyt.size(); i++)
	{
		Point_CPs_Q.push_back(KeyPoints_1_Q[matchs_cgyt[i].queryIdx].pt);
		Point_CPs_T.push_back(KeyPoints_2_T[matchs_cgyt[i].trainIdx].pt);
	}
	//变换矩阵计算，单应(homography)
	Mat Homo_Trans_mat;//变换矩阵3*3
	Homo_Trans_mat = cv::findHomography(Point_CPs_T, Point_CPs_Q, FM_RANSAC);
	cout << Homo_Trans_mat << endl;

	//透视变换，应用单应矩阵
	Mat img_trans1;
		
	Size Res_size; 
	Res_size.height = temp2.rows; 
	Res_size.width = 1.3 * temp2.cols;
	warpPerspective(Src2,img_trans1,Homo_Trans_mat,Res_size);
	imshow(window_name_f1, img_trans1);
	waitKey(0);

	//图像拷贝(融合)
	Rect Rang1, Rang2;
	Rang1 = Rect(0, 0, Src1.cols, Src1.rows);
	Rang2 = Rect(0, 0, img_trans1.cols, img_trans1.rows);

	//最终图像输出位置，最终图像大小设定，像素初始值设定
	Mat img_trans2(Src1.rows,img_trans1.cols,CV_8UC1);
	img_trans2.setTo(0);

	/*
	imshow(window_name_f1, img_trans2);
	waitKey(0);
*/
	
	//两图相关对应位置像素拷贝
	img_trans1.copyTo(img_trans2(Rang2));	
	Src1.copyTo(img_trans2(Rang1));
			
	imshow(window_name_f1, img_trans2);
	waitKey(0);

//连接处优化
	enum Locate_Jiaodian :int //四个边角位置枚举
	{
		左上=0,
		左下=1,
		右上=2,
		右下=3
	};
	vector<Mat> Points_Pic;  //变焦位置齐次坐标向量组4*（3*1）
	Mat Cord_buffer1 = Mat(3, 1, CV_64FC1);
	//四个角点设定（被变换的图像）
	Cord_buffer1.at<double>(0) = 0;
	Cord_buffer1.at<double>(1) = 0;
	Cord_buffer1.at<double>(2) = 1; 	
	cout << Cord_buffer1 << endl;	
	Points_Pic.push_back(Cord_buffer1); //左上

	Mat Cord_buffer2 = Mat(3, 1, CV_64FC1);
	Cord_buffer2.at<double>(0) = 0;
	Cord_buffer2.at<double>(1) = Src2.rows;
	Cord_buffer2.at<double>(2) = 1;
	cout << Cord_buffer2 << endl;
	Points_Pic.push_back(Cord_buffer2); //左下

	Mat Cord_buffer3 = Mat(3, 1, CV_64FC1);
	Cord_buffer3.at<double>(0) = Src2.cols;
	Cord_buffer3.at<double>(1) = 0;
	Cord_buffer3.at<double>(2) = 1;
	cout << Cord_buffer3 << endl;
	Points_Pic.push_back(Cord_buffer3); //右上

	Mat Cord_buffer4 = Mat(3, 1, CV_64FC1);
	Cord_buffer4.at<double>(0) = Src2.cols;
	Cord_buffer4.at<double>(1) = Src2.rows;
	Cord_buffer4.at<double>(2) = 1;
	Points_Pic.push_back(Cord_buffer4); //右下
	
	/*
	对push_back()函数的理解，vector.push_back函数仅复制指针，不复制具体内容。
	因此若使用同一变量多次赋不同值后 经push_back函数压入vector，
	则会出现所有容器中的元素值均为最后一次赋值的结果。注意注意
	*/

	cout << Cord_buffer4 << endl << "原始坐标设定完毕" << endl;

	//计算变换后的边界点坐标（在原图像坐标系下）
	for (int i = 0; i < 4; i++)
	{		
		cout << Points_Pic[i] << endl;
		
		Points_Pic[i] = Homo_Trans_mat * Points_Pic[i];
		cout << Points_Pic[i] << endl;
		double v0 = Points_Pic[i].at<double>(0);
		double v1 = Points_Pic[i].at<double>(1);
		double v2 = Points_Pic[i].at<double>(2);

		Points_Pic[i].at<double>(0) = v0 / v2;
		Points_Pic[i].at<double>(1) = v1 / v2;
		Points_Pic[i].at<double>(2) = 1;
		cout << Points_Pic[i] << endl << endl;
 	}
	//ss
	cout << "图像变换后边角坐标计算完毕" << endl;

	//重叠部分像素灰度重置，源：Src1 img_trans1，目标：img_trans2

	int start = MIN(Points_Pic[左上].at<double>(0), 
		            Points_Pic[左下].at<double>(0)); //重叠部分起始(横)坐标即列序
	//cout << start << endl;
	int end = Src1.cols;
	int Band_Width = end - start;
	double Rate = 1;
	for (int i = 0; i < img_trans2.rows; i++)
	{
		for (int j = start; j < end; j++)
		{
			if (img_trans1.at<uchar>(i, j) == 0)
			{
				Rate = 1;
			}
			else
			{
				Rate = (Band_Width - (j - start)) / Band_Width;
			}
			
			img_trans2.at<uchar>(i, j) = uchar(Rate * Src1.at<uchar>(i, j) + (1 - Rate) * img_trans1.at<uchar>(i, j));
			
			if (img_trans2.at<uchar>(i, j) == 0) //对灰度异常为0的探测代码 20200329
			{
				cout << "(" << i << "," << j << ")" << ";";
				uchar scale1 = Src1.at<uchar>(i, j);
				uchar scale2 = img_trans1.at<uchar>(i, j);
				cout << int(scale1) << "," << int(scale2) << ";";
				cout << Rate << endl;
			}
		}	
	
	}

	imshow(window_name_f1, img_trans2);
	waitKey(0);

	return img_trans2;

}

vector<Point2f> subPix_pt(Mat Src) //启动时间20200305 仅是对角点的检测，无描述子等相关判定，结合描述子可用于高进度的尺寸检测吧
{
	Mat temp;                     // #1 是输入图像（8位或32位单通道图）。
	temp = Src.clone(); 

	vector<Point2f> Corners;      // #2 是检测到的所有角点，类型为vector或数组，由实际给定的参数类型而定。
								  //如果是vector，那么它应该是一个包含cv::Point2f的vector对象；如果类型是cv::Mat,那么它的每一行对应一个角点，点的x、y位置分别是两列。	
	int Max_num = 50;            // #3 用于限定检测到的点数的最大值。
	double Quality_level = 0.09;  // #4 表示检测到的角点的质量水平（通常是0.10到0.01之间的数值，不能大于1.0）。
	double minDistance = 20;      // #5 用于区分相邻两个角点的最小距离（小于这个距离得点将进行合并）。
	
	goodFeaturesToTrack(temp,Corners,Max_num,Quality_level,minDistance);	
	/*
	第六个参数是mask，如果指定，它的维度必须和输入图像一致，且在mask值为0处不进行角点检测。
	第七个参数是blockSize，表示在计算角点时参与运算的区域大小，常用值为3，但是如果图像的分辨率较高则可以考虑使用较大一点的值。
	第八个参数用于指定角点检测的方法，如果是true则使用Harris角点检测，false则使用Shi Tomasi算法。
	第九个参数是在使用Harris算法时使用，最好使用默认值0.04。
	*/

	cv::Size Window_size = Size(5, 5);
	cv::Size Zone_0 = Size(-1, -1);

	cv::TermCriteria Criteria_cgyt = cv::TermCriteria(TermCriteria::EPS, 20, 0.04);
	cornerSubPix(temp,Corners,Window_size,Zone_0,Criteria_cgyt);
	/*
	第一个参数是输入图像，和cv::goodFeaturesToTrack()中的输入图像是同一个图像。
	第二个参数是检测到的角点，即是输入也是输出。
	第三个参数是计算亚像素角点时考虑的区域的大小，大小为NXN; N=(winSize*2+1)。
	第四个参数作用类似于winSize，但是总是具有较小的范围，通常忽略（即Size(-1, -1)）。
	第五个参数用于表示计算亚像素时停止迭代的标准，可选的值有cv::TermCriteria::MAX_ITER 、cv::TermCriteria::EPS（可以是两者其一，或两者均选），
	前者表示迭代次数达到了最大次数时停止，后者表示角点位置变化的最小值已经达到最小时停止迭代。
	二者均使用cv::TermCriteria()构造函数进行指定。
	*/
	
	//结果显示
	//temp.convertTo(temp, CV_8UC3); //转通道不等于转颜色表示
	cvtColor(temp, temp, COLOR_GRAY2BGR);
	for (int i = 0; i < Corners.size(); i++)
	{
		circle(temp,Corners[i],10,Scalar(0,0,255),3);
	}
	imshow(window_name_f1, temp);
	waitKey(0);

	return Corners;
}

int FaceC_cgyt(Mat Src)
{
	CascadeClassifier FaceDect;
	FaceDect.load("E:\\DIP\\OpencvPlus\\install\\etc\\haarcascades\\haarcascade_frontalface_alt2.xml");
	Mat temp;
	vector< Rect> Dected;
	FaceDect.detectMultiScale(Src, Dected, 1.06, 3, 0 | CASCADE_SCALE_IMAGE,Size(200,200));

	/*
1. const Mat& image：输入图像
2. vector& objects：输出的矩形向量组
3. double scaleFactor=1.1：这个是每次缩小图像的比例，默认是1.1
4. minNeighbors=3：匹配成功所需要的周围矩形框的数目（由于调整滑动窗口的大小和很多误报），每一个特征匹配到的区域都是一个矩形框，只有多个矩形框同时存在的时候，才认为是匹配成功，比如人脸，这个默认值是3。
5. flags=0：可以取如下这些值：
	CASCADE_DO_CANNY_PRUNING=1, 利用canny边缘检测来排除一些边缘很少或者很多的图像区域
	CASCADE_SCALE_IMAGE=2, 正常比例检测
	CASCADE_FIND_BIGGEST_OBJECT=4, 只检测最大的物体
	CASCADE_DO_ROUGH_SEARCH=8 初略的检测
6. minObjectSize maxObjectSize：匹配物体的大小范围 大小范围应该是最有效的限制
	*/

	if (Dected.empty())
	{
		printf("没找见脸\n");
		return -2;
	}
	cvtColor(Src, temp, COLOR_GRAY2BGR);
	for (int i = 0; i < Dected.size(); i++)
	{
		rectangle(temp, Dected[i], Scalar(0, 0, 255), 10);
	}
	imshow(window_name_f1, temp);
	waitKey(0);
	return 0;
}

int Components_Connected_cgyt(Mat Src)
{
	Mat Labels; //标签

	int Num_Components = connectedComponents(Src, Labels);  //默认使用八连通域，Label数据类型CV_32S
	if(Num_Components<2)
	{
		printf("未检测到对象\n");
		return -1;
	}
	else
	{
		printf("共检测到%d个对象\n", Num_Components - 1);
	}
	//创建随机颜色的输出图像
	Mat outPut = Mat::zeros(Src.rows, Src.cols, CV_8UC3);
	RNG seed(time(0)); //随机颜色种子
	for (int i = 1; i < Num_Components; i++) //i=0时的对象是背景
	{
		Mat mask = Labels == i;
		/*首先优先级 ==高于=，所以要先执行labels == i
		labels是一个Mat（矩阵），i为int，其目的提取矩阵中与i值相同的点，然后放在mask中，这样的操作就会使mask中区域值为1，非区域值为0。
		mask应是个二值矩阵*/
		int B = seed.uniform(0, 255);
		int G = seed.uniform(0, 255);
		int R = seed.uniform(0, 255);
		outPut.setTo(Scalar(B,G,R), mask);
	}
	imshow(window_name_f1, outPut);
	waitKey(0);
	return 0;
}
