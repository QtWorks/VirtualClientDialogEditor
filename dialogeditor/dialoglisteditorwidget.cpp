#include "dialoglisteditorwidget.h"
#include "dialogeditorwindow.h"
#include "core/dialogjsonwriter.h"
#include "core/ibackendconnection.h"
#include "applicationsettings.h"

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
{
	connect(this, &ListEditorWidget::itemEditRequested, this, &DialogListEditorWidget::onItemEditRequested);
	connect(this, &ListEditorWidget::itemCreateRequested, this, &DialogListEditorWidget::onItemCreateRequested);

	connect(m_backendConnection.get(), &Core::IBackendConnection::clientsLoaded, this, &DialogListEditorWidget::onClientsLoaded);
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

void DialogListEditorWidget::setCurrentClient(const Core::Client& client)
{
	if (!m_model.contains(client.databaseName))
	{
		return;
	}

	m_currentClient = client.databaseName;

	updateData();
}

void DialogListEditorWidget::setSettings(ApplicationSettings* settings)
{
	m_settings = settings;
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

// TODO: clientId is databaseName - rename or pass whole Client structure
void DialogListEditorWidget::addDialog(const QString& clientId, const Core::Dialog& dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo)
{
	Core::DialogJsonWriter writer;
	LOG << writer.write(dialog);

	showProgressDialog("Добавление данных", "Идет добавление данных. Пожалуйста, подождите.");

	m_dialogGraphicsInfoStorage->insert({ clientId, dialog.name, dialog.difficulty }, phasesGraphicsInfo);

	m_backendConnection->updateDialogs(clientId, { {}, {}, { dialog } });
}

void DialogListEditorWidget::onItemEditRequested(const QString& dialogName)
{
	const auto& dialogs = m_model[m_currentClient];

	const auto dialogIt = std::find_if(dialogs.begin(), dialogs.end(),
		[&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
	Q_ASSERT(dialogIt != dialogs.end());
	const int index = std::distance(dialogs.begin(), dialogIt);

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

	auto currentClientIt = std::find_if(m_clients.begin(), m_clients.end(),
		[this](const Core::Client& client) { return client.databaseName == m_currentClient; });
	Q_ASSERT(currentClientIt != m_clients.end());

	auto dialogGraphicsInfo = m_dialogGraphicsInfoStorage->read({ m_currentClient, dialogIt->name, dialogIt->difficulty });
	DialogEditorWindow* editorWindow = new DialogEditorWindow(*currentClientIt, *dialogIt, dialogGraphicsInfo, validator);

	DialogEditorWindow::NameValidatorEx nameValidator = [&](const Core::Client& client, const QString& name, Core::Dialog::Difficulty difficulty) -> bool
	{
		const auto dialogsIt = m_model.find(client.databaseName);
		if (dialogsIt == m_model.end())
		{
			return true;
		}

		const auto& dialogs = *dialogsIt;

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

	const auto clientIt = std::find_if(m_clients.begin(), m_clients.end(),
		[this](const Core::Client& client) { return client.databaseName == m_currentClient; });
	Q_ASSERT(clientIt != m_clients.end());

	editorWindow->enableSaveAs(m_clients, *clientIt, nameValidator);

	connect(editorWindow, &DialogEditorWindow::dialogModified,
		[this, index](Core::Dialog dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo) { updateDialog(index, dialog, phasesGraphicsInfo); });
	connect(editorWindow, &DialogEditorWindow::dialogCreated,
		[this](Core::Client client, Core::Dialog dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo) { addDialog(client.databaseName, dialog, phasesGraphicsInfo); });

	editorWindow->show();
}

void DialogListEditorWidget::onItemCreateRequested()
{
	Core::Dialog dialog = { "", Core::Dialog::Difficulty::Easy, "", { }, {}, 0.0, {} };
	dialog.phaseRepeatReplica = m_settings->phaseRepeatReplica();
	dialog.errorReplica.errorReplica = m_settings->phaseErrorReplica();
	dialog.errorReplica.errorPenalty = m_settings->phaseErrorPenalty();
	dialog.errorReplica.finishingExpectedWords = { m_settings->phaseFinishingExpectedWords() };
	dialog.errorReplica.finishingReplica = m_settings->phaseFinishingReplica();

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

	auto currentClientIt = std::find_if(m_clients.begin(), m_clients.end(),
		[this](const Core::Client& client) { return client.databaseName == m_currentClient; });
	Q_ASSERT(currentClientIt != m_clients.end());

	auto dialogGraphicsInfo = m_dialogGraphicsInfoStorage->read({ m_currentClient, dialog.name, dialog.difficulty });

	DialogEditorWindow* editorWindow = new DialogEditorWindow(*currentClientIt, dialog, dialogGraphicsInfo, validator);

	connect(editorWindow, &DialogEditorWindow::dialogModified,
		[this](Core::Dialog dialog, QList<PhaseGraphicsInfo> phasesGraphicsInfo) { addDialog(m_currentClient, dialog, phasesGraphicsInfo); });

	editorWindow->show();
}

void DialogListEditorWidget::onClientsLoaded(Core::IBackendConnection::QueryId /*queryId*/, const QList<Core::Client>& clients)
{
	m_clients = clients;
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
