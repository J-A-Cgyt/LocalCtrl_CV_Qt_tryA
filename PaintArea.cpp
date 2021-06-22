#include "PaintArea.h"
#include <QPen>
#include <QtGui>
#include <QFileDialog>
#include <QtDebug>

PaintArea::PaintArea(QWidget *parent) :QWidget(parent) { //���캯����Ҫ���ڲ���pixmap��һ����������
	setAutoFillBackground(true);
	setPalette(QPalette(Qt::white));
	setMinimumSize(600, 400);
	
	//QString filePath = QFileDialog::getOpenFileName(this, "start image select","/");
	//pic = new QPixmap(filePath);
	pic = new QPixmap();
	//QSize psize = pic->size();
	*pic = pic->scaled(600,400);
	pic->fill(Qt::white);
	backup = new QPixmap(pic->size());
	*backup = pic->copy();
	color = Qt::red;  //���캯�����趨Ϊ��ɫ
}

void PaintArea::setPic(const QPixmap &dispImg) {   //������ͼƬ�Ժ�Ҳ��ֱ��ˢ��һ��
	*pic = dispImg;
	backup = new QPixmap(pic->size());
	*backup = pic->copy();
	update();
}

void PaintArea::mousePressEvent(QMouseEvent *e) {
	startP =e->pos();
	//qDebug() << startP;  //�˴��ɼ����������������ȶ�����PaintArea���ӿؼ�������Զ��Ϊ��ʼ�ģ���û����mainwindow������ϵ ����һ������Mat���趨ROIʱ�����겻��Ҫƽ�ƣ�������Ȼ��Ҫ����
}

void PaintArea::mouseMoveEvent(QMouseEvent *e) {
	*pic = backup->copy();
	endP = e->pos();
	QPainter *painter = new QPainter;
	QPen pen;
	pen.setStyle(Qt::DashLine);  //�趨Ϊʵ��
	pen.setWidth(1);
	pen.setColor(Qt::green);
	painter->begin(pic);
	painter->setPen(pen);
	RoIRect = QRect(startP, endP);
	painter->drawRect(RoIRect);
	painter->end();
	update();
}

void PaintArea::mouseReleaseEvent(QMouseEvent *e) {  //���ܼ�¼������ֵ����������ڻ������PaintArea ��ȷ������ʾ�����ϻ��Ʒ�����û������
	endP = e->pos();
	QPainter *painter = new QPainter;
	QPen pen;
	pen.setStyle(Qt::SolidLine);  //�趨Ϊʵ��
	pen.setWidth(1);
	pen.setColor(color);
	painter->begin(pic);
	painter->setPen(pen);
	QRect rect(startP, endP);
	painter->drawRect(rect);
	painter->end();
	update();
}

void PaintArea::paintEvent(QPaintEvent *e) {
	QPainter painter(this);
	painter.drawPixmap(QPoint(0, 0), *pic);
}