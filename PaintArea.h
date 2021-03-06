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

	void PaintArea::setPic(const QPixmap &);  //Ϊ���ʵ�����ô����Ͷ����pixmap��Ա������

	inline QRect getRoI() { return RoIRect; }

protected:
	//��Щ�¼��ڿؼ�����ʾ��Χ���ⲻ�ᴥ��
	void mousePressEvent(QMouseEvent *e) override; //д��overrideҲ������ô��˵�����������麯���� �������ں�����Ҫ������򱨴�
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

