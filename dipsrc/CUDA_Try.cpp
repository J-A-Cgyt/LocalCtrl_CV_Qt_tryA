#pragma once
#include "Func_Proj_2nd.h"
//cuda模块的图像处理应用尝试
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp> //cv::cuda::add函数在此
#include <opencv2/cudafeatures2d.hpp> //特征检测类在此
#include <opencv2/xfeatures2d/cuda.hpp>
#include <opencv2/cudaobjdetect.hpp>

#include <opencv2/objdetect/objdetect.hpp> //haar级联分类器似乎要用 基本实锤和那个编译失败的cudev库有关 似乎是因为架构不兼容 可能需要更高版本的opencv以适配cuda10

//#include <opencv2/cudalegacy/NCVHaarObjectDetection.hpp>
//#include <opencv2/cudalegacy.hpp>

string window_nameC = "Demo_Result";

//此函数是部分CUDA功能的实现看看 20200705
Mat GPU_func_study(Mat Src1,Mat Src2)
{
	cv::cuda::printCudaDeviceInfo(cv::cuda::getDevice());

	namedWindow(window_nameC, WINDOW_NORMAL);

	if (!Src1.data || !Src2.data)
	{
		printf("传入图像为空\n");
		return Mat();
	}

	cuda::GpuMat GRes, Gimg1, Gimg2;
	Mat Cres;
	//图像传至显存
	Gimg1.upload(Src1);
	Gimg2.upload(Src2);
	//两图像按像素加
	cuda::add(Gimg1, Gimg2, GRes);
	GRes.download(Cres);
	imshow(window_nameC, Cres);
	waitKey(0);
	//两图像按像素减
	cuda::subtract(Gimg1, Gimg2, GRes);
	GRes.download(Cres);
	imshow(window_nameC, Cres);
	waitKey(0);

	return Cres;
}

//Hough变换的GPU版本重写看看吧，这东西用GPU加速性能不知道能快多少 真的是不管多少圆都是秒检测太猛了
Point3f IrisDectH_GPU(Mat Src)
{
	double dp = 1;
	double mindist = 40;
	double Param1 = 55;
	double Param2 = 40;
	int minRadius = 130;
	int maxRadius = 150;
	int maxNumbers = 1;  //GPU圆检测参数组

	cuda::GpuMat SrcG, DstG;
	Mat Dst, Map;
	vector<Vec3f> circles;
	Ptr<cuda::HoughCirclesDetector> Iris_deutector;
	Iris_deutector = cuda::createHoughCirclesDetector(
		dp, mindist, Param1, Param2, minRadius, maxRadius,maxNumbers);

	SrcG.upload(Src);
	Iris_deutector->detect(SrcG, DstG);
	if (!DstG.data)
	{
		cout << "未检测到圆" << endl;
		return Point3f();
	}
	DstG.download(Dst);

	//Map = Src.clone();
	cvtColor(Src, Map, COLOR_GRAY2BGR);
	for (int i = 0; i < Dst.cols; i++) //现有参数下会检测出一堆同心圆
	{
		circles.push_back(Dst.at<Vec3f>(0, i));
		circle(Map,
			Point(circles[i][0], circles[i][1]),
			circles[i][2],
			Scalar(0, 0, 255), 2);
	}
	imshow(window_nameC, Map);
	cout << Dst.size << endl;
	waitKey(0);
	return Point3f();
}


//用GPU的角点检测也试一下，应该大图没问题了，快的一批 真的快的一批，占用率5%
//01-ORB角点检测，并无描述子，需要brief描述子配合
int ORBG_cgyt(Mat Src)
{
	Ptr<cuda::ORB> detectorOG = cuda::ORB::create(); //创建中有参数可选，注意
	vector<KeyPoint> KeyP_ORBG;
	cuda::GpuMat SrcG;
	SrcG.upload(Src);
	detectorOG->detect(SrcG, KeyP_ORBG); //似乎允许显存和内存中的变量进行交互？
	cout << KeyP_ORBG.size() << endl;
	Mat Map;
	cvtColor(Src, Map, COLOR_GRAY2BGR);
	drawKeypoints(Map, KeyP_ORBG, Map);
	imshow(window_nameC, Map);
	waitKey(0);
	return 0;
}

//surf的角点检测，带匹配但不合成 关于CUDEV模块的问题，很有可能要使用，原因是CUDA不再支持2.0架构（可能是计算能力）。等真的要用的时候，在按照网上的办法重新编译试试吧
int SURFG_cgyt(Mat Src1,Mat Src2)
{
	cuda::GpuMat Src1G, Src2G;  //显存图像
	cuda::GpuMat KPsG1, KPsG2;  //显存角点存放
	vector<KeyPoint> KPsC1, KPsC2;  //内存角点存放
	cuda::GpuMat DescriptorG1, DescriptorG2; //显存描述子
	Src1G.upload(Src1);
	Src2G.upload(Src2);

	cuda::SURF_CUDA Surf_detector(150);  //surf检测器
	Surf_detector(Src1G, cuda::GpuMat(), KPsG1, DescriptorG1);
	Surf_detector(Src2G, cuda::GpuMat(), KPsG2, DescriptorG2);

	Ptr<cuda::DescriptorMatcher> MatcherG = cuda::DescriptorMatcher::createBFMatcher(); //角点匹配
	vector<vector<DMatch>> DMatchesG;  //显存匹配关系
	MatcherG->knnMatch(DescriptorG1, DescriptorG1, DMatchesG, 3); //KNN匹配
	Surf_detector.downloadKeypoints(KPsG1, KPsC1);
	Surf_detector.downloadKeypoints(KPsG2, KPsC2);  //匹配结果下载至内存

	vector<DMatch> Selected_matchs;
	for (int i = 0; i < MIN(KPsC1.size() - 1, DMatchesG.size()); i++)
	{
		if ((DMatchesG[i][0].distance < (0.75 * DMatchesG[i][1].distance)) &&
			int(DMatchesG[i].size()) <= 2 &&
			int(DMatchesG[i].size()) > 0
			)
		{
			Selected_matchs.push_back(DMatchesG[i][0]);
		}
	}		
	cout << Selected_matchs.size() << endl;
	Mat Map;
	drawMatches(Src1,KPsC1,Src2,KPsC2,Selected_matchs,Map);
	imshow(window_nameC, Map);
	waitKey(0);
	return 0;
}


//再来加点东西,似乎有个人脸识别啥的还挺好玩 有别人早好的轮子是真的方便快捷 废，部分版本问题导致GPU版本级联分类器无法实例化
int FaceG_cgyt(Mat Src) //使用的是HAAR的级联分类器 设定需输入灰度图像
{
	cv::cuda::printCudaDeviceInfo(cv::cuda::getDevice());

	Mat Temp = Src.clone();
	
	Ptr<cuda::CascadeClassifier> Cas_cgyt = cuda::CascadeClassifier::create("E:\\DIP\\OpencvPlus\\install\\etc\\haarcascades\\haarcascade_frontalface_alt2.xml");

	cuda::GpuMat d_Buffer;
	cuda::GpuMat d_Img;       //申请设备显存
	d_Img.upload(Temp);       //拷贝图像至显存
	Cas_cgyt->detectMultiScale(d_Img, d_Buffer);   //人脸检测
	vector< Rect> Detected;
	Cas_cgyt->convert(d_Buffer, Detected);   //画个框框把人脸框出来，gpu的检测结果放回主机内存
	if (Detected.empty())
	{
		printf("没找见脸\n");
		return 0;
	}
	cvtColor(Src, Temp, COLOR_GRAY2BGR);
	for (int i = 0; i < Detected.size(); i++)
	{
		rectangle(Temp, Detected[i], Scalar(0, 0, 255), 2);
	}
	imshow(window_nameC, Temp);
	waitKey(0);
	return 0;
}

//如果要自己写图像处理程序，最简单的方法可能还是用CUDA项目构建了。图像处理核函数只能使用指针找数据，GpuMat好像不能用at,一定注意显存中的数据分布
Mat CannyG_Cgyt(Mat Src)
{
	cuda::GpuMat d_edge, d_img;
	Mat h_edge;
	d_img.upload(Src);
	Ptr<cuda::CannyEdgeDetector> Canny_edge = cuda::createCannyEdgeDetector(2.0, 100.0, 3, false); //创建GPU的Canny检测器并设定参数
	Canny_edge->detect(d_img, d_edge);  //进行检测
	d_edge.download(h_edge);
	imshow(window_nameC, h_edge);
	waitKey(0);
	return h_edge;
}

int HoughLineG_Cgyt(Mat Src_edge) //输入图像应是CANNY的检测结果
{
	Mat hc_edge;
	cvtColor(Src_edge, hc_edge, COLOR_GRAY2BGR);
	cuda::GpuMat d_edge, d_lines;
	d_edge.upload(Src_edge);
	Ptr<cuda::HoughSegmentDetector> Hough = cuda::createHoughSegmentDetector(1.0f, (float)(CV_PI / 180.0f), 50, 5);
	Hough->detect(d_edge, d_lines);

	vector< Vec4i> Lines_info;
	if (!d_lines.empty())
	{
		Lines_info.resize(d_lines.cols);
		Mat h_lines(1, d_lines.cols, CV_32SC4, &Lines_info[0]);
		d_lines.download(h_lines);   //此处为何不能直接下载，不能匹配Mat类型吗 CUDA+OPENCV PAGE141
	}
	for (size_t i = 0; i < Lines_info.size(); ++i)  //划线循环
	{
		Vec4i line_point = Lines_info[i];
		line(hc_edge, Point(line_point[0], line_point[1]), 
					   Point(line_point[2], line_point[3]), 
			Scalar(0, 0, 255), 2, LINE_AA);
	}
	imshow(window_nameC, hc_edge);
	waitKey(0);
	return 0;
}