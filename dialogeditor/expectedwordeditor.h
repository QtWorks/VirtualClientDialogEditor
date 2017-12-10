#ifndef EXPECTEDWORDEDITOR_H
#define EXPECTEDWORDEDITOR_H

#include <QWidget>

namespace Ui {
class ExpectedWordEditor;
}

class ExpectedWordEditor
	: public QWidget
{
	Q_OBJECT

public:
	explicit ExpectedWordEditor(const QString& text, QWidget* parent = 0);
	~ExpectedWordEditor();

	QString text() const;
	void setFocus();

signals:
	void changed(QString text);
	void removed();

private:
	Ui::ExpectedWordEditor* m_ui;
};

#endif // EXPECTEDWORDEDITOR_H
