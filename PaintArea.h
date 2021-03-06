#pragma once
#include <qwidget.h>
#include <qmouseeventtransition.h>
#include <qlabel.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>

class PaintArea : public QWidget
{
	Q_OBJECT
public:
	PaintArea(QWidget* parent = nullptr);
	~PaintArea() { }

	void PaintArea::setPic(const QPixmap &);  //为外层实例设置此类型对象的pixmap成员而设置

	inline QRect getRoI() { return RoIRect; }

protected:
	//这些事件在控件的显示范围以外不会触发
	void mousePressEvent(QMouseEvent *e) override; //写了override也不会怎么样说明这货真的是虚函数？ 但是类内函数需要定义否则报错
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent *e);

private:
	QPixmap *pic;
	QPixmap *backup;
	QPoint startP, endP;
	QRect RoIRect;
	QColor color;
};

