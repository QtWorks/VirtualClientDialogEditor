#ifndef DIALOGEDITORWINDOW_H
#define DIALOGEDITORWINDOW_H

#include "core/idialogmodel.h"
#include "dialoggraphicsscene.h"
#include <QWidget>
#include <memory>

namespace Ui {
class DialogEditorWindow;
}

class DialogEditorWindow
	: public QWidget
{
	Q_OBJECT

public:
	explicit DialogEditorWindow(const Dialog& dialog, QWidget* parent = 0);
	~DialogEditorWindow();

signals:
	void dialogChanged(Dialog dialog);

public slots:
	void updateControls();
	void showError(QString text);
	void hideError();

private:
	Ui::DialogEditorWindow* m_ui;

	QGraphicsScene* m_dialogConstructorGraphicsScene;
	DialogGraphicsScene* m_dialogGraphicsScene;
	std::unique_ptr<IDialogModel> m_dialogModel;
};

#endif // DIALOGEDITORWINDOW_H
