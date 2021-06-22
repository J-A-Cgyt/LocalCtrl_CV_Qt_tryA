#include "HK_Camera_Drive.h"
#include <Windows.h>  //ע�� ��ͷ�ļ��ж�����cv�����ռ��ͻ ע�����
#include <qdebug.h>

// Wait for key press �ȴ������û�
void WaitForKeyPress(void)
{
	while (!_kbhit())
	{
		Sleep(10);
	}
	_getch();
}

// print the discovered devices information to user ��ӡ�豸��Ϣ
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
		int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);       //IPv4��ַ����

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

//��ɫת��
int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
	if (NULL == pRgbData)
	{
		return MV_E_PARAMETER;
	}

	for (unsigned int j = 0; j < nHeight; j++)  //�˴��費��Ҫ����CUDA�� ������CPU���߳�
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];  //B R��λ �洢��ʽΪһάѭ����ʽ RGB RGB RGB...
			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}

	return MV_OK;
}

// convert data stream in Mat format ������Ҫ�Ĺؼ����� ת��ΪMat
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

//�˴���Ҫ��һ����ʧ������½��ռ�õĺ���
void releaseCamera(void* handle, int nRet, char* errorStr) {
	//�������ƺ���

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
	//���پ��Ӧ�����Ӻ��������
}


//------------------------------------------------------������ʵ�����Ԥ�������߳����ʵ��-------------------------------------------------------------------------
void preViewHK_thread::run()  //�̵߳�ִ�к���
{
	int nRet = MV_OK;
	char infoStr[256] = { 0 };

	qDebug()<< "preViewHK_threadID:" << currentThreadId();
	//����ť����Ϊ������
	//ui.startViewBtn->setEnabled(false);
	//ui.endViewBtn->setEnabled(true);

	//��ʼץȡͼ��
	nRet = MV_CC_StartGrabbing(handle);
	if (MV_OK != nRet) {
		sprintf(infoStr, "��ʼץȡͼ��ʧ�� nRet [0x%x]\n", nRet);
		//ui.missionInfoEdit->setText(QString::fromLocal8Bit(infoStr));
		return;
	}

	MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
	memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX)); //�ڴ����û����룬��ȡ֡�ĺ��������ݴ���ָ��ָ�����׵�ַ
	unsigned char *pData = (unsigned char*)malloc(sizeof(unsigned char) * (g_nPayLoadSize));
	if (pData == NULL) {
		sprintf(infoStr, "�����ڴ�ʧ��ʧ��\n");
		//ui.missionInfoEdit->setText(QString::fromLocal8Bit(infoStr));
		return;
	}

	//��ʾѭ��
	//����ע��ѭ��
	do {
		//��ȡһ֡ͼ�� ��ʱ1000ms  �Ӵ˴���ʼѭ����
		nRet = MV_CC_GetOneFrameTimeout(handle, pData, g_nPayLoadSize, &stImageInfo, 10000);
		if (nRet == MV_OK) {
			sprintf(infoStr, "��ȡ��֡ͼ��: Width:[%d],Height[%d], nFrameNum[%d]\n",
				stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);
			//ui.missionInfoEdit->setText(QString::fromLocal8Bit(infoStr));
		}
		else {
			sprintf(infoStr, "û���� [0x%x]\n", nRet);
			//ui.missionInfoEdit->setText(QString::fromLocal8Bit(infoStr));
			free(pData);
			pData = nullptr;
			return;
		}

		//cv::Mat Img_Catched;
		middle = Convert2Mat(&stImageInfo, pData);

		mutex.lock();
		mat2pixmap(middle);  //ת��ΪQPixmap
		mutex.unlock();           //��QPixmap����Ҫ�����Ʒ��ʵ�
		//printf("ץȡͼƬ����ʾ\n");
		//if (cv::waitKey(200) == 'e') { break; }
		emit oneFrameFinished(); //ͼ��ץȡ��ת����ɺ����ź�
		/* �˴����� ������run()�����з����źŴ�����ĳ���� �򱻴����Ĳۺ���Ҳ���������߳��� */
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

//��ʶ����Ϊ�� ѭ���ı�Ҫ����
void preViewHK_thread::flagT()
{
	flag = true;
}

//��ʶ����Ϊ�� ֹͣѭ��ǰ�ı�Ҫ����
void preViewHK_thread::flagF()
{
	flag = false;
}

void preViewHK_thread::mat2pixmap(const cv::Mat & Src)  //Mat ת��Ϊ QPixmap�ĺ��� ֱ��copy��������   ע�� �����ڴ���������� Ѱ�ҵ����ĸ��������ظ������ڴ�
{
	//�˴�������ж�һ���Ǻڰ׻��ǲ�ɫ
	QImage tempImg;
	cv::Mat color;
	if (Src.type() == CV_8UC1) {
		tempImg = QImage((const unsigned char*)(Src.data), Src.cols, Src.rows, 
			Src.cols * Src.channels(),
			QImage::Format_Grayscale8);
	}
	else if (Src.type() == CV_8UC3) {
		cvtColor(Src, color, cv::COLOR_BGR2RGB); //����ǲ�ɫ��Ҫת��һ��ɫ�ռ� Qt��rgb ���Ҵ���ϰ����BGR
		tempImg = QImage((const unsigned char*)(color.data), color.cols, color.rows,
			color.cols * color.channels(),   //ע1 δ��ֹͼ����б�¼ӵĲ�����ע��ÿ���м���byte����ֹ�Զ�����
			QImage::Format_RGB888);
	}
	else {
		throw std::runtime_error("src.type() ����Ϊ CV_8UC1 �� CV_8UC3");  //�����ʽ������ ��Ҫ����
	}
	res = QPixmap::fromImage(tempImg.scaled(tempImg.size(), Qt::KeepAspectRatio));  //�˴�δ���Ĵ�С
}
