#include "CV_MinWin.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

CV_MinWin::CV_MinWin(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	//���캯������Щcombo�����Լ���ص��趨���ǵüӰ� ���ڴ��ͳ������ XMLҲ���Ǹ����õ�ѡ��
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

}