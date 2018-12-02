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

DialogListEditorWidget::DialogListEditorWidget(IBackendConnectionSharedPtr backendConnection, DialogGraphicsInfoStoragePtr dialogGraphicsInfoStorage,
	QWidget* parent)
	: ListEditorWidget(parent)
	, m_backendConnection(backendConnection)
	, m_dialogGraphicsInfoStorage(dialogGraphicsInfoStorage)
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

void DialogListEditorWidget::setCurrentClient(const QString& client)
{
	if (!m_model.contains(client))
	{
		return;
	}

	m_currentClient = client;

	updateData();
}

QStringList DialogListEditorWidget::items() const
{
	QStringList result;

	if (!m_model.contains(m_currentClient))
	{
		return result;
	}

	const auto& dialogs = m_model[m_currentClient];

	for (const auto& dialog: dialogs)
	{
		result << dialog.printableName();
	}

	return result;
}

void DialogListEditorWidget::removeItems(const QStringList& dialogs)
{
	showProgressDialog("Удаление данных", "Идет удаление данных. Пожалуйста, подождите.");

	m_updating = true;

	const auto& sourceDialogs = m_model[m_currentClient];
	QList<Core::Dialog> removingDialogs;
	QList<DialogGraphicsInfoStorage::Key> removingDialogGraphicsInfoKeys;
	for (const QString& dialogName : dialogs)
	{		
		const auto it = std::find_if(sourceDialogs.begin(), sourceDialogs.end(),
			[&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
		Q_ASSERT(it != sourceDialogs.end());
		removingDialogs.append(*it);
		removingDialogGraphicsInfoKeys.append({ m_currentClient, dialogName, it->difficulty });
	}

	for (const auto& key : removingDialogGraphicsInfoKeys)
	{
		m_dialogGraphicsInfoStorage->remove(key);
	}

	m_backendConnection->updateDialogs(m_currentClient, { {}, removingDialogs, {} });
}

void DialogListEditorWidget::updateDialog(int index, const Core::Dialog& dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo)
{
	const Core::Dialog& sourceDialog = m_model[m_currentClient][index];

	if (sourceDialog.name != dialog.name || sourceDialog.difficulty != dialog.difficulty)
	{
		m_dialogGraphicsInfoStorage->remove({ m_currentClient, sourceDialog.name, sourceDialog.difficulty });
		m_dialogGraphicsInfoStorage->insert({ m_currentClient, dialog.name, dialog.difficulty }, phasesGraphicsInfo);
	}
	else
	{
		m_dialogGraphicsInfoStorage->update({ m_currentClient, dialog.name, dialog.difficulty }, phasesGraphicsInfo);
	}

	if (dialog == sourceDialog)
	{
		return;
	}

	showProgressDialog("Изменение данных", "Идет изменение данных. Пожалуйста, подождите.");

	const QMap<Core::Dialog, Core::Dialog> updated = {
		{ sourceDialog, dialog }
	};
	m_backendConnection->updateDialogs(m_currentClient, { updated, {}, {} });
}

void DialogListEditorWidget::addDialog(const Core::Dialog& dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo)
{
	Core::DialogJsonWriter writer;
	LOG << writer.write(dialog);

	showProgressDialog("Добавление данных", "Идет добавление данных. Пожалуйста, подождите.");

	m_dialogGraphicsInfoStorage->insert({ m_currentClient, dialog.name, dialog.difficulty }, phasesGraphicsInfo);

	m_backendConnection->updateDialogs(m_currentClient, { {}, {}, { dialog } });
}

void DialogListEditorWidget::onItemEditRequested(const QString& dialogName)
{
	const auto& dialogs = m_model[m_currentClient];

	const auto it = std::find_if(dialogs.begin(), dialogs.end(),
		[&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
	Q_ASSERT(it != dialogs.end());
	const int index = std::distance(dialogs.begin(), it);

	const auto validator = [this, index, &dialogs](const QString& name, Core::Dialog::Difficulty difficulty)
	{
		const QString newName = Core::Dialog::printableName(name, difficulty);
		for (int i = 0; i < dialogs.length(); i++)
		{
			if (i == index)
			{
				continue;
			}

			if (dialogs[i].printableName().compare(newName, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	auto dialogGraphicsInfo = m_dialogGraphicsInfoStorage->read({ m_currentClient, it->name, it->difficulty });
	DialogEditorWindow* editorWindow = new DialogEditorWindow(*it, dialogGraphicsInfo, validator, true);
	connect(editorWindow, &DialogEditorWindow::dialogModified,
		[this, index](Core::Dialog dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo) { updateDialog(index, dialog, phasesGraphicsInfo); });
	connect(editorWindow, &DialogEditorWindow::dialogCreated,
		[this](Core::Dialog dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo) { addDialog(dialog, phasesGraphicsInfo); });

	editorWindow->show();
}

void DialogListEditorWidget::onItemCreateRequested()
{
	const Core::Dialog dialog = { "", Core::Dialog::Difficulty::Easy, { }, {}, 0.0 };

	const auto validator = [this](const QString& name, Core::Dialog::Difficulty difficulty)
	{
		const QString newName = Core::Dialog::printableName(name, difficulty);

		const auto& dialogs = m_model[m_currentClient];

		for (int i = 0; i < dialogs.length(); i++)
		{
			if (dialogs[i].printableName().compare(newName, Qt::CaseInsensitive) == 0)
			{
				return false;
			}
		}

		return true;
	};

	auto dialogGraphicsInfo = m_dialogGraphicsInfoStorage->read({ m_currentClient, dialog.name, dialog.difficulty });
	DialogEditorWindow* editorWindow = new DialogEditorWindow(dialog, dialogGraphicsInfo, validator, false);
	connect(editorWindow, &DialogEditorWindow::dialogModified,
		[this](Core::Dialog dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo) { addDialog(dialog, phasesGraphicsInfo); });

	editorWindow->show();
}

void DialogListEditorWidget::onDialogsLoaded(Core::IBackendConnection::QueryId /*queryId*/,
	const QMap<QString, QList<Core::Dialog>>& dialogs)
{
	m_model = dialogs;
	m_currentClient = dialogs.isEmpty() ? "" : dialogs.keys().first();

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
