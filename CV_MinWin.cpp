#include "CV_MinWin.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "HK_Camera_Drive.h"
#include "qdebug.h"

//#include <thread> //瞎几把再试试

CV_MinWin::CV_MinWin(QWidget *parent)
    : QMainWindow(parent) //注意初始化线程的父关系
{
    ui.setupUi(this);
	//构造函数内那些combo的项以及相关的设定还是得加啊 对于大型程序而言 XML也许是个更好的选择？

	//海康相机需要用到的成员数值初始化
	isViewing = false;
	isConnected = false;
	handle = nullptr;
	g_nPayLoadSize = 0;

	//预览线程成员初始化
	thread_view = new preViewHK_thread(handle,this);
	//槽函数连接
	connect(thread_view, SIGNAL(oneFrameFinished()), this, SLOT(updatePic()));
}

//非槽成员函数（工具函数）的定义 从CV_MinWin.h row 19 start
 //为了显示处理结果需要设置一个从cv::Mat向QPixMap转换的函数 20201209测试通过
void CV_MinWin::showMat(const Mat &src) {
	//此处最好能判定一下是黑白还是彩色
	QImage tempImg;
	Mat color;
	if (src.type() == CV_8UC1) {
		tempImg = QImage((const unsigned char*)(src.data), src.cols, src.rows, QImage::Format_Grayscale8);
	}
	else if (src.type() == CV_8UC3) {		
		cvtColor(src, color, COLOR_BGR2RGB); //如果是彩色就要转换一下色空间 Qt是rgb 但我处理习惯用BGR
		tempImg = QImage((const unsigned char*)(color.data), color.cols, color.rows, 
			color.cols * color.channels(),   //注1
			QImage::Format_RGB888);
	}
	else {
		throw runtime_error("src.type() 必须为 CV_8UC1 或 CV_8UC3");  //如果格式都不对 就要报错
	}
	displayImg = QPixmap::fromImage(tempImg.scaled(ui.picBox->size(), Qt::KeepAspectRatio));
	//ui.picBox->setPixmap(displayImg);
	ui.picBox->setPic(displayImg);
}
/*
如果图像宽度不是4的整数倍,则图像会发生倾斜
原因分析:
QImage转换为Format_RGB888等格式时,每行会按4字节(32位)对齐,不足则自动补齐
但是rgb.data中的数据是没有自动补齐的,所以会导致图像作为QImage显示时,会发生倾斜
因此以上代码应
*/

//槽函数定义 20201208 启动 先做一些试验吧
void  CV_MinWin::openPic() {
	QString filePathq;
	filePathq = QFileDialog::getOpenFileName(this,"openPic");
	std::string filePath = filePathq.toStdString();  //读取的文件路径以 / 分割，转为std::string后可以直接被imread读取
	if (ui.colorRadio->isChecked()) {
		Src = imread(filePath, IMREAD_COLOR);
	}
	else if (ui.grayRadio->isChecked())
	{
		Src = imread(filePath, IMREAD_GRAYSCALE);
	}
	else {
		throw runtime_error("必须在黑白和彩色中间选一个");
	}
	showMat(Src);
}

void CV_MinWin::gaussBlur() {
	//需要用一个对话框接收整数的值
	int sidelen = ui.ksizeSpin->value();	
	//判定有无数据 没数据的话就弹个窗 别报错了
	if (!Src.data) {
		QMessageBox::information(this, QString::fromLocal8Bit("注意"), QString::fromLocal8Bit("请先载入图片"));
		return;
	}
	else if (sidelen % 2 == 0) { //判断模板边长是否非法
		QMessageBox::information(this, QString::fromLocal8Bit("注意"),QString::fromLocal8Bit("模板边长应为奇数"));
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

//单步撤销
void CV_MinWin::undo() { 
	//temp = Res.clone();  
	//只能撤销一步哦 这个用作对action槽的编辑测试 直接在设计器的类槽里面添加后在槽编辑器里面选择即可
	Res = temp.clone();
	showMat(Res);
}

void CV_MinWin::connectCamera() {

	char strTemp[256] = {0};
	QString errorInfo; //错误信息输出局部变量

	int nRet = MV_OK;
	handle = nullptr;

	//step1:枚举设备
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));  //这似乎是个STL函数 用于初始化内存空间为设定值
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList);
	if (MV_OK != nRet) {
		//printf("枚举设备失败 nRet [0x%x]\n", nRet); //%x估计是16进制显示的错误代码
		errorInfo = QString::fromLocal8Bit("枚举设备失败,nRet:[");
		errorInfo.append(QString::number(nRet,16));
		errorInfo.append("]");
		ui.missionInfoEdit->setText(errorInfo);
		return;
	}

	if (stDeviceList.nDeviceNum > 0) {
		for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
			//printf("[Device %d]:\n", i);
			MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i]; //看来列表里的也只是设备信息的指针实际的对象并未直接暴露，该指针应当有权限设置
			if (NULL == pDeviceInfo) {
				break;
			}
			//PrintDeviceInfo(pDeviceInfo); //此句用于控制台打印设备信息 格式可能和显示的不太一样 故此先注释掉
			sprintf(strTemp, "%s", pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
			ui.cameraLabel2->setText(strTemp);
		}
	}
	else {
		errorInfo = QString::fromLocal8Bit("没找到设备");
		ui.missionInfoEdit->setText(errorInfo);
		return;
	}

	//选择设备并创建句柄
	nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[0]);
	if (MV_OK != nRet) {
		errorInfo = QString::fromLocal8Bit("创建句柄失败,nRet:[");
		errorInfo.append(QString::number(nRet, 16));
		errorInfo.append("]");
		ui.missionInfoEdit->setText(errorInfo);
		return;
	}

	//打开设备
	nRet = MV_CC_OpenDevice(handle);
	if (MV_OK != nRet) {
		errorInfo = QString::fromLocal8Bit("打开设备失败,nRet:[");
		errorInfo.append(QString::number(nRet, 16));
		errorInfo.append("]");
		ui.missionInfoEdit->setText(errorInfo);	
		//需要释放占用
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
//似乎和数据包设定相关
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

	// Set trigger mode as off  关闭触发模式 此处应也能设定图像格式
	nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
	if (MV_OK != nRet) {
		sprintf(strTemp, "Set Trigger Mode fail! nRet [0x%x]\n", nRet);
		ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
		//需要释放占用
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

	//获取有效数据大小 （PayloadSize）
	MVCC_INTVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
	if (MV_OK != nRet) {
		sprintf(strTemp, "获取有效数据大小失败 nRet [0x%x]\n", nRet);
		ui.missionInfoEdit->setText(QString::fromLocal8Bit(strTemp));
		//需要释放占用
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

	//连接成功后将相机必要的控制参数发给预览执行线程
	thread_view->set_param(handle, g_nPayLoadSize);
}

void CV_MinWin::startView() {  //此函数需要多线程 在这里启动线程 然后传入handle的参数

	//将按钮设置为不可用
	ui.startViewBtn->setEnabled(false);
	ui.endViewBtn->setEnabled(true);

	//启动线程
	thread_view->flagT();
	thread_view->start();
}

void CV_MinWin::endView() {
	isViewing = false;	
	char strTemp[256] = { 0 };
	int nRet = MV_OK;

	//退出线程
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

	//断开连接后清理线程和主UI的控制参数
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
