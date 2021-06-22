#include "PaintArea.h"
#include <QPen>
#include <QtGui>
#include <QFileDialog>
#include <QtDebug>

PaintArea::PaintArea(QWidget *parent) :QWidget(parent) { //构造函数需要对内部的pixmap做一个基本设置
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
	color = Qt::red;  //构造函数中设定为红色
}

void PaintArea::setPic(const QPixmap &dispImg) {   //设置完图片以后也得直接刷新一下
	*pic = dispImg;
	backup = new QPixmap(pic->size());
	*backup = pic->copy();
	update();
}

void PaintArea::mousePressEvent(QMouseEvent *e) {
	startP =e->pos();
	//qDebug() << startP;  //此处可见此坐标与绘制坐标等都是以PaintArea即子控件的坐标远点为起始的，即没有用mainwindow的坐标系 这样一来，在Mat中设定ROI时该坐标不需要平移，但是仍然需要缩放
}

void PaintArea::mouseMoveEvent(QMouseEvent *e) {
	*pic = backup->copy();
	endP = e->pos();
	QPainter *painter = new QPainter;
	QPen pen;
	pen.setStyle(Qt::DashLine);  //设定为实线
	pen.setWidth(1);
	pen.setColor(Qt::green);
	painter->begin(pic);
	painter->setPen(pen);
	RoIRect = QRect(startP, endP);
	painter->drawRect(RoIRect);
	painter->end();
	update();
}

void PaintArea::mouseReleaseEvent(QMouseEvent *e) {  //不管记录的坐标值是相对主窗口还是相对PaintArea 正确的在显示界面上绘制方框是没有问题
	endP = e->pos();
	QPainter *painter = new QPainter;
	QPen pen;
	pen.setStyle(Qt::SolidLine);  //设定为实线
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