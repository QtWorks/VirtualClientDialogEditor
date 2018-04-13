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
	void accepted(const Core::ExpectedWordsNode& expectedWords);
	void rejected();

private slots:
	void onUseHintCheckboxChecked(bool checked);
	void onHintChanged();
	void onAddItemClicked();
	void onSaveClicked();
	void onCancelClicked();

	void validate();

private:
	void addItemWidget(const Core::ExpectedWords& item);
	void updateControls();

	void setError(const QString& error);
	void removeError();

private:
	Ui::ExpectedWordsEditorWindow* m_ui;
	Core::ExpectedWordsNode m_expectedWords;
	QVector<ExpectedWordEditor*> m_itemEditors;
};

#endif // EXPECTEDWORDSEDITORWINDOW_H
