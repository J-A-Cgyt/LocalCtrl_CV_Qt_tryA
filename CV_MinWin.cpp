#include "CV_MinWin.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "HK_Camera_Drive.h"
#include "qdebug.h"

//#include <thread> //Ϲ����������

CV_MinWin::CV_MinWin(QWidget *parent)
    : QMainWindow(parent) //ע���ʼ���̵߳ĸ���ϵ
{
    ui.setupUi(this);
	//���캯������Щcombo�����Լ���ص��趨���ǵüӰ� ���ڴ��ͳ������ XMLҲ���Ǹ����õ�ѡ��

	//���������Ҫ�õ��ĳ�Ա��ֵ��ʼ��
	isViewing = false;
	isConnected = false;
	handle = nullptr;
	g_nPayLoadSize = 0;

	//Ԥ���̳߳�Ա��ʼ��
	thread_view = new preViewHK_thread(handle,this);
	//�ۺ�������
	connect(thread_view, SIGNAL(oneFrameFinished()), this, SLOT(updatePic()));
}

//�ǲ۳�Ա���������ߺ������Ķ��� ��CV_MinWin.h row 19 start
 //Ϊ����ʾ��������Ҫ����һ����cv::Mat��QPixMapת���ĺ��� 20201209����ͨ��
void CV_MinWin::showMat(const Mat &src) {
	//�˴�������ж�һ���Ǻڰ׻��ǲ�ɫ
	QImage tempImg;
	Mat color;
	if (src.type() == CV_8UC1) {
		tempImg = QImage((const unsigned char*)(src.data), src.cols, src.rows, QImage::Format_Grayscale8);
	}
	else if (src.type() == CV_8UC3) {		
		cvtColor(src, color, COLOR_BGR2RGB); //����ǲ�ɫ��Ҫת��һ��ɫ�ռ� Qt��rgb ���Ҵ���ϰ����BGR
		tempImg = QImage((const unsigned char*)(color.data), color.cols, color.rows, 
			color.cols * color.channels(),   //ע1
			QImage::Format_RGB888);
	}
	else {
		throw runtime_error("src.type() ����Ϊ CV_8UC1 �� CV_8UC3");  //�����ʽ������ ��Ҫ����
	}
	displayImg = QPixmap::fromImage(tempImg.scaled(ui.picBox->size(), Qt::KeepAspectRatio));
	//ui.picBox->setPixmap(displayImg);
	ui.picBox->setPic(displayImg);
}
/*
���ͼ���Ȳ���4��������,��ͼ��ᷢ����б
ԭ�����:
QImageת��ΪFormat_RGB888�ȸ�ʽʱ,ÿ�лᰴ4�ֽ�(32λ)����,�������Զ�����
����rgb.data�е�������û���Զ������,���Իᵼ��ͼ����ΪQImage��ʾʱ,�ᷢ����б
������ϴ���Ӧ
*/

//�ۺ������� 20201208 ���� ����һЩ�����
void  CV_MinWin::openPic() {
	QString filePathq;
	filePathq = QFileDialog::getOpenFileName(this,"openPic");
	std::string filePath = filePathq.toStdString();  //��ȡ���ļ�·���� / �ָתΪstd::string�����ֱ�ӱ�imread��ȡ
	if (ui.colorRadio->isChecked()) {
		Src = imread(filePath, IMREAD_COLOR);
	}
	else if (ui.grayRadio->isChecked())
	{
		Src = imread(filePath, IMREAD_GRAYSCALE);
	}
	else {
		throw runtime_error("�����ںڰ׺Ͳ�ɫ�м�ѡһ��");
	}
	showMat(Src);
}

void CV_MinWin::gaussBlur() {
	//��Ҫ��һ���Ի������������ֵ
	int sidelen = ui.ksizeSpin->value();	
	//�ж��������� û���ݵĻ��͵����� �𱨴���
	if (!Src.data) {
		QMessageBox::information(this, QString::fromLocal8Bit("ע��"), QString::fromLocal8Bit("��������ͼƬ"));
		return;
	}
	else if (sidelen % 2 == 0) { //�ж�ģ��߳��Ƿ�Ƿ�
		QMessageBox::information(this, QString::fromLocal8Bit("ע��"),QString::fromLocal8Bit("ģ��߳�ӦΪ����"));
		ui.ksizeSpin->setValue(sidelen - 1);
		return;
	}

	if (!Res.data) { 
		GaussianBlur(Src, Res, Size(sidelen, sidelen), 1);
		temp = Src.clone(); 
	}
	else {
		temp = Res.clone();
		GaussianBlur(temp, Res, Size(sidelen, sidelen), 1);
	}
	showMat(Res);
}

//��������
void CV_MinWin::undo() { 
	//temp = Res.clone();  
	//ֻ�ܳ���һ��Ŷ ���������action�۵ı༭���� ֱ��������������������Ӻ��ڲ۱༭������ѡ�񼴿�
	Res = temp.clone();
	showMat(Res);
}

void CV_MinWin::connectCamera() {

	char strTemp[256] = {0};
	QString errorInfo; //������Ϣ����ֲ�����

	int nRet = MV_OK;
	handle = nullptr;

	//step1:ö���豸
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));  //���ƺ��Ǹ�STL���� ���ڳ�ʼ���ڴ�ռ�Ϊ�趨ֵ
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList);
	if (MV_OK != nRet) {
		//printf("ö���豸ʧ�� nRet [0x%x]\n", nRet); //%x������16������ʾ�Ĵ������
		errorInfo = QString::fromLocal8Bit("ö���豸ʧ��,nRet:[");
		errorInfo.append(QString::number(nRet,16));
		errorInfo.append("]");
		ui.missionInfoEdit->setText(errorInfo);
		return;
	}

	if (stDeviceList.nDeviceNum > 0) {
		for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
			//printf("[Device %d]:\n", i);
			MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; //�����б����Ҳֻ���豸��Ϣ��ָ��ʵ�ʵĶ���δֱ�ӱ�¶����ָ��Ӧ����Ȩ������
			if (NULL == pDeviceInfo) {
				break;
			}
			//PrintDeviceInfo(pDeviceInfo); //�˾����ڿ���̨��ӡ�豸��Ϣ ��ʽ���ܺ���ʾ�Ĳ�̫һ�� �ʴ���ע�͵�
			sprintf(strTemp, "%s", pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
			ui.cameraLabel2->setText(strTemp);
		}
	}
	else {
		errorInfo = QString::fromLocal8Bit("û�ҵ��豸");
		ui.missionInfoEdit->setText(errorInfo);
		return;
	}

	//ѡ���豸���������
	nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[0]);
	if (MV_OK != nRet) {
		errorInfo = QString::fromLocal8Bit("�������ʧ��,nRet:[");
		errorInfo.append(QString::number(nRet, 16));
		errorInfo.append("]");
		ui.missionInfoEdit->setText(errorInfo);
		return;
	}

	//���豸
	nRet = MV_CC_OpenDevice(handle);
	if (MV_OK != nRet) {
		errorInfo = QString::fromLocal8Bit("���豸ʧ��,nRet:[");
		errorInfo.append(QString::number(nRet, 16));
		errorInfo.append("]");
		ui.missionInfoEdit->setText(errorInfo);	
		//��Ҫ�ͷ�ռ��
		releaseCamera(handle,nRet,strTemp);
		// Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			sprintf(strTemp, "Destroy Handle fail! nRet [0x%x]\n", nRet);
			ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
		}
		return;
	}

	// Detection network optimal package size(It only works for the GigE camera)
//�ƺ������ݰ��趨���
	if (stDeviceList.pDeviceInfo[0]->nTLayerType == MV_GIGE_DEVICE) {
		int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
		if (nPacketSize > 0) {
			nRet = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
			if (nRet != MV_OK) {
				sprintf(strTemp,"Warning: Set Packet Size fail nRet [0x%x]!", nRet);
				errorInfo = QString::fromLocal8Bit(strTemp);
				ui.missionInfoEdit->setText(errorInfo);
			}
		}
		else {
			sprintf(strTemp,"Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
			ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
		}
	}

	// Set trigger mode as off  �رմ���ģʽ �˴�ӦҲ���趨ͼ���ʽ
	nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
	if (MV_OK != nRet) {
		sprintf(strTemp, "Set Trigger Mode fail! nRet [0x%x]\n", nRet);
		ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
		//��Ҫ�ͷ�ռ��
		releaseCamera(handle, nRet, strTemp);
		// Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			sprintf(strTemp, "Destroy Handle fail! nRet [0x%x]\n", nRet);
			ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
		}
		return;
	}

	//��ȡ��Ч���ݴ�С ��PayloadSize��
	MVCC_INTVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
	if (MV_OK != nRet) {
		sprintf(strTemp, "��ȡ��Ч���ݴ�Сʧ�� nRet [0x%x]\n", nRet);
		ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
		//��Ҫ�ͷ�ռ��
		releaseCamera(handle, nRet, strTemp);
		// Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			sprintf(strTemp, "Destroy Handle fail! nRet [0x%x]\n", nRet);
			ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
		}
		return;
	}
	g_nPayLoadSize = stParam.nCurValue;

	ui.missionInfoEdit->setText(QString::fromLocal8Bit("Connect Camera Successfully"));

	//���ӳɹ��������Ҫ�Ŀ��Ʋ�������Ԥ��ִ���߳�
	thread_view->set_param(handle, g_nPayLoadSize);
}

void CV_MinWin::startView() {  //�˺�����Ҫ���߳� �����������߳� Ȼ����handle�Ĳ���

	//����ť����Ϊ������
	ui.startViewBtn->setEnabled(false);
	ui.endViewBtn->setEnabled(true);

	//�����߳�
	thread_view->flagT();
	thread_view->start();
}

void CV_MinWin::endView() {
	isViewing = false;	
	char strTemp[256] = { 0 };
	int nRet = MV_OK;

	//�˳��߳�
	thread_view->flagF();
	thread_view->quit();

	ui.startViewBtn->setEnabled(true);
	ui.endViewBtn->setEnabled(false);

	releaseCamera(handle, nRet, strTemp);
	// Destroy handle
	nRet = MV_CC_DestroyHandle(handle);
	if (MV_OK != nRet)
	{
		sprintf(strTemp, "Destroy Handle fail! nRet [0x%x]\n", nRet);
		ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
	}
	else {
		sprintf(strTemp, "Disconnect Camera Successfully!");
		ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
	}

	//�Ͽ����Ӻ������̺߳���UI�Ŀ��Ʋ���
	handle = nullptr;
	g_nPayLoadSize = 0;	
	thread_view->set_param(handle, 0);
}

void CV_MinWin::updatePic()
{
	displayImg = thread_view->res.scaled(ui.picBox->size(), Qt::KeepAspectRatio);
	ui.picBox->setPic(displayImg);
	//ui.missionInfoEdit->setText("x");
	qDebug() << "slot hitted " << QThread::currentThreadId();
}
