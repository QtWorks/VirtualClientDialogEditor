#include "zoomablegraphicsview.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimeLine>

ZoomableGraphicsView::ZoomableGraphicsView(QSplitter* /*splitter*/)
	: m_minRatio(100.0)
	, m_maxRatio(100.0)
	, m_currentScale(100.0)
	, m_originX(0)
	, m_originY(0)
	, m_scheduledScalingsNumber(0)
{
}

void ZoomableGraphicsView::setMinRatio(qreal minRatio)
{
	Q_ASSERT(minRatio <= m_maxRatio);
	m_minRatio = minRatio;
}

void ZoomableGraphicsView::setMaxRatio(qreal maxRatio)
{
	Q_ASSERT(maxRatio >= m_minRatio);
	m_maxRatio = maxRatio;
}

void ZoomableGraphicsView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_originX = event->x();
		m_originY = event->y();
	}

	QGraphicsView::mousePressEvent(event);
}

void ZoomableGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		QPointF oldp = mapToScene(m_originX, m_originY);
		QPointF newp = mapToScene(event->pos());
		QPointF translation = newp - oldp;

		translate(translation.x(), translation.y());

		m_originX = event->x();
		m_originY = event->y();
	}

	QGraphicsView::mouseMoveEvent(event);
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent* event)
{
	int numDegrees = event->delta() / 8;
	int numSteps = numDegrees / 15;
	m_scheduledScalingsNumber += numSteps;
	if (m_scheduledScalingsNumber * numSteps < 0)
	{
		m_scheduledScalingsNumber = numSteps;
	}

	const int animationDuration = 350;
	QTimeLine* animation = new QTimeLine(animationDuration, this);
	animation->setUpdateInterval(animationDuration / 10);

	connect(animation, &QTimeLine::valueChanged, this, &ZoomableGraphicsView::scalingTime);
	connect(animation, &QTimeLine::finished, this, &ZoomableGraphicsView::animationFinished);

	animation->start();
}

void ZoomableGraphicsView::scalingTime()
{
	qreal factor = 1.0 + qreal(m_scheduledScalingsNumber) / 300.0;

	if (m_currentScale * factor <= m_maxRatio && m_currentScale * factor >= m_minRatio) {
		m_currentScale *= factor;
		scale(factor, factor);
	}
}

void ZoomableGraphicsView::animationFinished()
{
	m_scheduledScalingsNumber += m_scheduledScalingsNumber > 0 ? -1 : 1;

	sender()->~QObject();
}
