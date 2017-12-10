#ifndef EXPECTEDWORDSEDITORWINDOW_H
#define EXPECTEDWORDSEDITORWINDOW_H

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
	explicit ExpectedWordsEditorWindow(const QStringList& expectedWords, QWidget* parent = 0);
	~ExpectedWordsEditorWindow();

signals:
	void accepted(QStringList expectedWords);
	void rejected();

private slots:
	void onAddItemClicked();
	void onSaveClicked();
	void onCancelClicked();

private:
	void addItemWidget(const QString& item);
	void updateControls();

private:
	Ui::ExpectedWordsEditorWindow* m_ui;
	QVector<ExpectedWordEditor*> m_items;
};

#endif // EXPECTEDWORDSEDITORWINDOW_H
