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
	void onAddItemClicked();
	void onSaveClicked();
	void onCancelClicked();

	void validate();

private:
	void addItemWidget(const Core::ExpectedWords& item);
	void updateControls();

	void setError(const QString& error);
	void removeError();

	Core::ExpectedWordsNode getNode() const;
	void setNode(const Core::ExpectedWordsNode& node);

private:
	Ui::ExpectedWordsEditorWindow* m_ui;
	QVector<ExpectedWordEditor*> m_itemEditors;
	Core::ExpectedWordsNode m_initialNode;
};

#endif // EXPECTEDWORDSEDITORWINDOW_H
