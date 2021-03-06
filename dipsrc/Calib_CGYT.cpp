#include "Func_Proj_2nd.h"

string Source_Path[10];
string Window_calib = "Corners_dected";

/*用于摄像机标定的函数，总体感觉误差似乎比较大，不如matlab的标定结果更接近S10超广的标称参数，
但是中心的标定结果是差不多的，只有焦距结果出了差异，从代码实现的角度来说并无问题，难道是因为算法不行？*/
int Calib_Cgyt(Mat InputOutputArray)
{
	//图像加载路径（于XPS15 9550）
	Source_Path[0] = "G:\\Pictures\\Test For Programming\\calb13\\1.jpg";
	Source_Path[1] = "G:\\Pictures\\Test For Programming\\calb13\\2.jpg";
	Source_Path[2] = "G:\\Pictures\\Test For Programming\\calb13\\3.jpg";
	Source_Path[3] = "G:\\Pictures\\Test For Programming\\calb13\\4.jpg";
	Source_Path[4] = "G:\\Pictures\\Test For Programming\\calb13\\5.jpg";
	Source_Path[5] = "G:\\Pictures\\Test For Programming\\calb13\\6.jpg";
	Source_Path[6] = "G:\\Pictures\\Test For Programming\\calb13\\7.jpg";
	Source_Path[7] = "G:\\Pictures\\Test For Programming\\calb13\\8.jpg";
	Source_Path[8] = "G:\\Pictures\\Test For Programming\\calb13\\9.jpg";
	Source_Path[9] = "G:\\Pictures\\Test For Programming\\calb13\\10.jpg";

	//窗口展示
	namedWindow(Window_calib, WINDOW_NORMAL);
	
	//图像读取
	vector<Mat> Src_img;
	for (int i = 0; i < 10; i++)
	{
		Src_img.push_back(imread(Source_Path[i], IMREAD_GRAYSCALE));
	}
	cv::Size ChessboardSize = Size(9, 6);   //棋盘格格子数
	vector<vector<Point2f>> Corners_queue;  //检测所得的图像点序列组
	vector<Point2f> Corners;  //单幅图像测得图像点暂存

	//亚像素角点参数设定
	cv::Size Window_size = Size(5, 5);
	cv::Size Zone_0 = Size(-1, -1);
	//迭代终止条件模板类
	cv::TermCriteria Criteria_cgyt = cv::TermCriteria(
		TermCriteria::MAX_ITER + 
		TermCriteria::EPS, 
		30,     //最大迭代次数
		0.01);  //最小精度
	
	//检测循环
	for (int i = 0; i < 10; i++)
	{
		bool ifFound = findChessboardCorners(Src_img[0], ChessboardSize, Corners);
		if (ifFound = false)
		{
			cout << "find corners failed" << endl;
			return -2;
		}
		cornerSubPix(Src_img[0], Corners, Window_size, Zone_0, Criteria_cgyt);
		Corners_queue.push_back(Corners);
	}
	//角点绘制展示
	Mat map;	
	cvtColor(Src_img[0], map, COLOR_GRAY2BGR);
	drawChessboardCorners(map,ChessboardSize,Corners,true);
	imshow(Window_calib,map);
	waitKey(0);

//内外参计算
	//世界坐标计算
	cv::Size GridSize = cv::Size(20, 20);
	vector<vector<Point3f>> Cord_world;

	vector<Point3f> Cord_WordQueue;
	for (int j = 0; j < ChessboardSize.height; j++)
	{
		for (int i = 0; i < ChessboardSize.width; i++)
		{
			Point3f Cord_Temp;
			Cord_Temp.x = i * GridSize.width;
			Cord_Temp.y = j * GridSize.height;
			Cord_Temp.z = 0;
			Cord_WordQueue.push_back(Cord_Temp);
		}
	}
	for (int i = 0; i < 10; i++)
	{
		Cord_world.push_back(Cord_WordQueue);
	}

	//摄像机标定结果参数组
	Mat K_CameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));  //摄像机内参矩阵K
	Mat distortion = Mat(1, 5, CV_32FC1, Scalar::all(0));  //摄像机畸变参数，共5个
	vector<Mat> RotationV; //图像旋转向量
	vector<Mat> MoveV;  //图像平移向量

	calibrateCamera(Cord_world, Corners_queue, 
		Src_img[0].size(), 
		K_CameraMatrix, 
		distortion, RotationV, MoveV, 
		0); //可能因为取消了K3，导致对此类变形较大的超广角的焦距判断失误20200406 改用0试一下 CALIB_FIX_K3

	cout << K_CameraMatrix << endl;

//畸变矫正测试
	Mat res;
	undistort(map, res, K_CameraMatrix, distortion);
	imshow(Window_calib, res);  //效果好像有点过头了
	waitKey(0);

	return 0;
}