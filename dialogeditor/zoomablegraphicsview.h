#ifndef ZOOMABLEGRAPHICSVIEW_H
#define ZOOMABLEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QSplitter>

class ZoomableGraphicsView
	: public QGraphicsView
{
public:
	ZoomableGraphicsView(QSplitter* splitter);

	void setMinRatio(qreal minRatio);
	void setMaxRatio(qreal maxRatio);

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private slots:
	void scalingTime();
	void animationFinished();

private:
	qreal m_minRatio;
	qreal m_maxRatio;
	double m_currentScale;

	int m_originX;
	int m_originY;
	int m_scheduledScalingsNumber;
};

#endif // ZOOMABLEGRAPHICSVIEW_H
