#include "CV_MinWin.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

CV_MinWin::CV_MinWin(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	//构造函数内那些combo的项以及相关的设定还是得加啊 对于大型程序而言 XML也许是个更好的选择？
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

}