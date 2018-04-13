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
	explicit ExpectedWordEditor(const Core::ExpectedWords& expectedWords, QWidget* parent = 0);
	~ExpectedWordEditor();

	Core::ExpectedWords expectedWords() const;
	void setFocus();

signals:
	void changed();
	void removed();

private slots:
	void onWordsChanged();
	void onScoreChanged();
	void onRemoveClicked();

private:
	Ui::ExpectedWordEditor* m_ui;
	Core::ExpectedWords* m_originalExpectedWords;
	Core::ExpectedWords m_expectedWords;
};

#endif // EXPECTEDWORDEDITOR_H
