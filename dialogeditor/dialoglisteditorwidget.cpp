#include "dialoglisteditorwidget.h"
#include "dialogeditorwindow.h"
#include "core/dialogjsonwriter.h"
#include "core/ibackendconnection.h"

#include "logger.h"

#include <QMessageBox>

namespace
{

QString toLowerCase(const QString& str)
{
	return str.left(1).toLower() + str.mid(1);
}

}

DialogListEditorWidget::DialogListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: ListEditorWidget(parent)
	, m_backendConnection(backendConnection)
	, m_updating(false)
{
	connect(this, &ListEditorWidget::itemEditRequested, this, &DialogListEditorWidget::onItemEditRequested);
	connect(this, &ListEditorWidget::itemCreateRequested, this, &DialogListEditorWidget::onItemCreateRequested);

	connect(m_backendConnection.get(), &Core::IBackendConnection::dialogsLoaded, this, &DialogListEditorWidget::onDialogsLoaded);
	connect(m_backendConnection.get(), &Core::IBackendConnection::dialogsLoadFailed, this, &DialogListEditorWidget::onDialogsLoadFailed);
	connect(m_backendConnection.get(), &Core::IBackendConnection::dialogsUpdated, this, &DialogListEditorWidget::onDialogsUpdated);
	connect(m_backendConnection.get(), &Core::IBackendConnection::dialogsUpdateFailed, this, &DialogListEditorWidget::onDialogsUpdateFailed);
}

void DialogListEditorWidget::loadData()
{
	showProgressDialog("Загрузка данных", "Идет загрузка данных. Пожалуйста, подождите.");

	m_backendConnection->loadDialogs();
}

QStringList DialogListEditorWidget::items() const
{
	QStringList result;

	for (const DialogListDataModel::Index& index : m_model.indexes())
	{
		result << m_model.get(index).printableName();
	}

	return result;
}

void DialogListEditorWidget::removeItems(const QStringList& dialogs)
{
	showProgressDialog("Удаление данных", "Идет удаление данных. Пожалуйста, подождите.");

	m_updating = true;

	QList<Core::Dialog> removingDialogs;
	for (const QString& dialogName : dialogs)
	{
		const DialogListDataModel::Index index = m_model.findIndex(
			[&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
		removingDialogs.append(m_model.get(index));
	}

	m_backendConnection->updateDialogs({ {}, removingDialogs, {} });
}

void DialogListEditorWidget::updateDialog(int index, const Core::Dialog& dialog)
{
	const Core::Dialog& sourceDialog = m_model.get(index);
	if (dialog == sourceDialog)
	{
		return;
	}

	showProgressDialog("Изменение данных", "Идет изменение данных. Пожалуйста, подождите.");

	const QMap<Core::Dialog, Core::Dialog> updated = {
		{ sourceDialog, dialog }
	};
	m_backendConnection->updateDialogs({ updated, {}, {} });
}

void DialogListEditorWidget::addDialog(const Core::Dialog& dialog)
{
	Core::DialogJsonWriter writer;
	LOG << writer.write(dialog);

	showProgressDialog("Добавление данных", "Идет добавление данных. Пожалуйста, подождите.");
	m_backendConnection->updateDialogs({ {}, {}, { dialog } });
}

void DialogListEditorWidget::onItemEditRequested(const QString& dialogName)
{
	const DialogListDataModel::Index index = m_model.findIndex(
		[&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
	Q_ASSERT(index != -1);

	DialogEditorWindow* editorWindow = new DialogEditorWindow(m_model.get(index), true);
	connect(editorWindow, &DialogEditorWindow::dialogModified,
		[this, index](Core::Dialog dialog) { updateDialog(index, dialog); });
	connect(editorWindow, &DialogEditorWindow::dialogCreated,
		[this](Core::Dialog dialog) { addDialog(dialog); });

	editorWindow->show();
}

void DialogListEditorWidget::onItemCreateRequested()
{
	const Core::Dialog dialog = { "", Core::Dialog::Difficulty::Easy, { }, {} };

	DialogEditorWindow* editorWindow = new DialogEditorWindow(dialog, false);
	connect(editorWindow, &DialogEditorWindow::dialogModified,
		[this](Core::Dialog dialog) { addDialog(dialog); });

	editorWindow->show();
}

void DialogListEditorWidget::onDialogsLoaded(Core::IBackendConnection::QueryId /*queryId*/, const QList<Core::Dialog>& dialogs)
{
	m_model.setData(dialogs);

	updateData();

	hideProgressDialog();

	if (m_updating)
	{
		QMessageBox::information(this, "Сохранение данных", "Сохранение данных завершилось успешно.");
		m_updating = false;
	}
}

void DialogListEditorWidget::onDialogsLoadFailed(Core::IBackendConnection::QueryId /*queryId*/, const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Загрузка данных", "Загрузка данных завершилась ошибкой: " + toLowerCase(error) + ".");
}

void DialogListEditorWidget::onDialogsUpdated(Core::IBackendConnection::QueryId /*queryId*/)
{
	hideProgressDialog();

	loadData();
}

void DialogListEditorWidget::onDialogsUpdateFailed(Core::IBackendConnection::QueryId /*queryId*/, const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Сохранение данных", "Сохранение данных завершилось ошибкой: " + toLowerCase(error) + ".");
}
