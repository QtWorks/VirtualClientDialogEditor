#pragma once

#include <QWidget>
#include <QTimer>
#include <QColor>

class WaitingSpinnerWidget
	: public QWidget
{
	Q_OBJECT

public:
	WaitingSpinnerWidget(QWidget* parent = 0, bool centerOnParent = true, bool disableParentWhenSpinning = true);

	/*! Constructor - use this constructor to automatically create a modal
	* ("blocking") spinner on top of the calling widget/window.  If a valid
	* parent widget is provided, "centreOnParent" will ensure that
	* QtWaitingSpinner automatically centres itself on it, if not,
	* "centreOnParent" is ignored. */
	WaitingSpinnerWidget(Qt::WindowModality modality, QWidget* parent = 0, bool centerOnParent = true, bool disableParentWhenSpinning = true);

public slots:
	void start();
	void stop();

public:
	void setColor(QColor color);
	void setRoundness(qreal roundness);
	void setMinimumTrailOpacity(qreal minimumTrailOpacity);
	void setTrailFadePercentage(qreal trail);
	void setRevolutionsPerSecond(qreal revolutionsPerSecond);
	void setNumberOfLines(int lines);
	void setLineLength(int length);
	void setLineWidth(int width);
	void setInnerRadius(int radius);
	void setText(QString text);

	QColor color();
	qreal roundness();
	qreal minimumTrailOpacity();
	qreal trailFadePercentage();
	qreal revolutionsPersSecond();
	int numberOfLines();
	int lineLength();
	int lineWidth();
	int innerRadius();

	bool isSpinning() const;

private slots:
	void rotate();

protected:
	void paintEvent(QPaintEvent* paintEvent);

private:
	static int lineCountDistanceFromPrimary(int current, int primary, int totalNrOfLines);
	static QColor currentLineColor(int distance, int totalNrOfLines, qreal trailFadePerc, qreal minOpacity, QColor color);

	void initialize();
	void updateSize();
	void updateTimer();
	void updatePosition();

private:
	QColor m_color;
	qreal m_roundness; // 0..100
	qreal m_minimumTrailOpacity;
	qreal m_trailFadePercentage;
	qreal m_revolutionsPerSecond;
	int m_numberOfLines;
	int m_lineLength;
	int m_lineWidth;
	int m_innerRadius;

private:
	WaitingSpinnerWidget(const WaitingSpinnerWidget&);
	WaitingSpinnerWidget& operator=(const WaitingSpinnerWidget&);

	QTimer* m_timer;
	bool m_centerOnParent;
	bool m_disableParentWhenSpinning;
	int m_currentCounter;
	bool m_isSpinning;
};
