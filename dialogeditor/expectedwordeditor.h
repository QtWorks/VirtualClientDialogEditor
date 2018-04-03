#ifndef EXPECTEDWORDEDITOR_H
#define EXPECTEDWORDEDITOR_H

#include "core/dialog.h"
#include <QWidget>

namespace Ui {
class ExpectedWordEditor;
}

class ExpectedWordEditor
	: public QWidget
{
	Q_OBJECT

public:
	explicit ExpectedWordEditor(const Core::ExpectedWords& text, QWidget* parent = 0);
	~ExpectedWordEditor();

	QString text() const;
	void setFocus();

signals:
	void changed(Core::ExpectedWords expectedWords);
	void removed();

private:
	Ui::ExpectedWordEditor* m_ui;
	Core::ExpectedWords m_expectedWords;
};

#endif // EXPECTEDWORDEDITOR_H
