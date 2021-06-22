#include "HK_Camera_Drive.h"
#include <Windows.h>  //注意 该头文件中定义会和cv命名空间冲突 注意包含
#include <qdebug.h>

// Wait for key press 等待键盘敲击
void WaitForKeyPress(void)
{
	while (!_kbhit())
	{
		Sleep(10);
	}
	_getch();
}

// print the discovered devices information to user 打印设备信息
bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
	if (NULL == pstMVDevInfo)
	{
		printf("The Pointer of pstMVDevInfo is NULL!\n");
		return false;
	}
	if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
	{
		int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
		int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
		int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
		int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);       //IPv4地址构建

		// print current ip and user defined name
		printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
		printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
		printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
	}
	else
	{
		printf("Not support.\n");
	}

	return true;
}

//颜色转换
int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
	if (NULL == pRgbData)
	{
		return MV_E_PARAMETER;
	}

	for (unsigned int j = 0; j < nHeight; j++)  //此处需不需要引入CUDA？ 还是用CPU多线程
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];  //B R换位 存储方式为一维循环形式 RGB RGB RGB...
			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}

	return MV_OK;
}

// convert data stream in Mat format 这是需要的关键函数 转换为Mat
cv::Mat Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData)
{
	cv::Mat srcImage;
	if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
	{
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
	}
	else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
	{
		RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
	}
	else
	{
		printf("unsupported pixel format\n");
		return srcImage;
	}

	if (NULL == srcImage.data)
	{
		return srcImage;
	}

	//save converted image in a local file
	try {
#if defined (VC9_COMPILE)
		cvSaveImage("MatImage.bmp", &(IplImage(srcImage)));
#else
		cv::imwrite("MatImage.bmp", srcImage);
#endif
	}
	catch (cv::Exception& ex) {
		fprintf(stderr, "Exception saving image to bmp format: %s\n", ex.what());
	}

	//srcImage.release();

	return srcImage;
}

//此处需要有一个在失败情况下解除占用的函数
void releaseCamera(void* handle, int nRet, char* errorStr) {
	//以下是善后工作

	// Stop grab image
	nRet = MV_CC_StopGrabbing(handle);
	if (MV_OK != nRet)
	{
		sprintf(errorStr, "Stop Grabbing fail! nRet [0x%x]\n", nRet);
		return;
	}

	// Close device
	nRet = MV_CC_CloseDevice(handle);
	if (MV_OK != nRet)
	{
		sprintf(errorStr, "ClosDevice fail! nRet [0x%x]\n", nRet);
		return;
	}
	//销毁句柄应在连接函数中完成
}


//------------------------------------------------------在这里实现相机预览独立线程类的实现-------------------------------------------------------------------------
void preViewHK_thread::run()  //线程的执行函数
{
	int nRet = MV_OK;
	char infoStr[256] = { 0 };

	qDebug()<< "preViewHK_threadID:" << currentThreadId();
	//将按钮设置为不可用
	//ui.startViewBtn->setEnabled(false);
	//ui.endViewBtn->setEnabled(true);

	//开始抓取图像
	nRet = MV_CC_StartGrabbing(handle);
	if (MV_OK != nRet) {
		sprintf(infoStr, "开始抓取图像失败 nRet [0x%x]\n", nRet);
		//ui.missionInfoEdit->setText(QString::fromLocal8Bit(infoStr));
		return;
	}

	MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
	memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX)); //内存由用户申请，获取帧的函数将数据存入指针指定的首地址
	unsigned char *pData = (unsigned char*)malloc(sizeof(unsigned char) * (g_nPayLoadSize));
	if (pData == NULL) {
		sprintf(infoStr, "申请内存失败失败\n");
		//ui.missionInfoEdit->setText(QString::fromLocal8Bit(infoStr));
		return;
	}

	//显示循环
	//暂且注释循环
	do {
		//获取一帧图像 超时1000ms  从此处开始循环？
		nRet = MV_CC_GetOneFrameTimeout(handle, pData, g_nPayLoadSize, &stImageInfo, 10000);
		if (nRet == MV_OK) {
			sprintf(infoStr, "获取单帧图像: Width:[%d],Height[%d], nFrameNum[%d]\n",
				stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);
			//ui.missionInfoEdit->setText(QString::fromLocal8Bit(infoStr));
		}
		else {
			sprintf(infoStr, "没数据 [0x%x]\n", nRet);
			//ui.missionInfoEdit->setText(QString::fromLocal8Bit(infoStr));
			free(pData);
			pData = nullptr;
			return;
		}

		//cv::Mat Img_Catched;
		middle = Convert2Mat(&stImageInfo, pData);

		mutex.lock();
		mat2pixmap(middle);  //转换为QPixmap
		mutex.unlock();           //仅QPixmap是需要被限制访问的
		//printf("抓取图片已显示\n");
		//if (cv::waitKey(200) == 'e') { break; }
		emit oneFrameFinished(); //图像抓取并转换完成后发射信号
		/* 此处发现 若是在run()函数中发射信号触发了某个槽 则被触发的槽函数也运行在子线程中 */
		msleep(400);
	} while (flag);

	nRet = MV_CC_StopGrabbing(handle);
	if (MV_OK != nRet)
	{
		sprintf(infoStr, "Stop Grabbing fail! nRet [0x%x]\n", nRet);
		return;
	}

	// Close device
	nRet = MV_CC_CloseDevice(handle);
	if (MV_OK != nRet)
	{
		sprintf(infoStr, "ClosDevice fail! nRet [0x%x]\n", nRet);
		return;
	}
}

void preViewHK_thread::set_param(void * c_handle, unsigned int c_g_nPayLoadSize)
{
	handle = c_handle;
	g_nPayLoadSize = c_g_nPayLoadSize;
}

//标识符置为真 循环的必要条件
void preViewHK_thread::flagT()
{
	flag = true;
}

//标识符置为假 停止循环前的必要条件
void preViewHK_thread::flagF()
{
	flag = false;
}

void preViewHK_thread::mat2pixmap(const cv::Mat & Src)  //Mat 转换为 QPixmap的函数 直接copy过来就行   注意 发生内存持续性增长 寻找到底哪个函数在重复申请内存
{
	//此处最好能判定一下是黑白还是彩色
	QImage tempImg;
	cv::Mat color;
	if (Src.type() == CV_8UC1) {
		tempImg = QImage((const unsigned char*)(Src.data), Src.cols, Src.rows, 
			Src.cols * Src.channels(),
			QImage::Format_Grayscale8);
	}
	else if (Src.type() == CV_8UC3) {
		cvtColor(Src, color, cv::COLOR_BGR2RGB); //如果是彩色就要转换一下色空间 Qt是rgb 但我处理习惯用BGR
		tempImg = QImage((const unsigned char*)(color.data), color.cols, color.rows,
			color.cols * color.channels(),   //注1 未防止图像倾斜新加的参数，注明每列有几个byte，防止自动补齐
			QImage::Format_RGB888);
	}
	else {
		throw std::runtime_error("src.type() 必须为 CV_8UC1 或 CV_8UC3");  //如果格式都不对 就要报错
	}
	res = QPixmap::fromImage(tempImg.scaled(tempImg.size(), Qt::KeepAspectRatio));  //此处未更改大小
}
