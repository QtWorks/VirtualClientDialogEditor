#ifndef EXPECTEDWORDSEDITORWINDOW_H
#define EXPECTEDWORDSEDITORWINDOW_H

#include "core/dialog.h"
#include <QWidget>

namespace Ui {
class ExpectedWordsEditorWindow;
}

class ExpectedWordEditor;

class ExpectedWordsEditorWindow
	: public QWidget
{
	Q_OBJECT

public:
	explicit ExpectedWordsEditorWindow(const Core::ExpectedWordsNode& expectedWords, QWidget* parent = 0);
	~ExpectedWordsEditorWindow();

signals:
	void accepted(Core::ExpectedWordsNode expectedWords);
	void rejected();

private slots:
	void onAddItemClicked();
	void onSaveClicked();
	void onCancelClicked();

private:
	void addItemWidget(const Core::ExpectedWords& item);
	void updateControls();

private:
	Ui::ExpectedWordsEditorWindow* m_ui;
	QVector<ExpectedWordEditor*> m_itemEditors;
};

#endif // EXPECTEDWORDSEDITORWINDOW_H
