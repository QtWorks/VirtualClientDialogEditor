#include "userstabwidget.h"
#include "usersxlsxdocument.h"

#include <QFileDialog>
#include <QMessageBox>

using namespace Core;

namespace
{

QList<QString> clientIds(const QList<Client>& clients)
{
	QList<Client> sorted = clients;
	std::sort(sorted.begin(), sorted.end(), [](const Client& left, const Client& right)
	{
		return left.name < right.name;
	});

	QList<QString> result;

	for (const auto& client : sorted)
	{
		result.append(client.id);
	}

	result.prepend("");

	return result;
}

bool isAdminsGroup(QString clientId)
{
	return clientId.isEmpty();
}

QList<Core::User> prepareToExport(const QList<Core::User>& users, const Core::Client& client)
{
	QList<Core::User> result;

	for (const Core::User& user : users)
	{
		Core::User updated = user;
		updated.groups = {};

		for (const QString& groupId : user.groups)
		{
			auto groupIt = std::find_if(client.groups.begin(), client.groups.end(),
			[&groupId](const Core::Group& group)
			{
				return group.id == groupId;
			});

			if (groupIt != client.groups.end())
			{
				updated.groups << groupIt->name;
			}
		}

		result << updated;
	}

	return result;
}

QList<Core::User> prepareToImport(const QList<Core::User>& users, const Core::Client& client)
{
	QList<Core::User> result;

	for (const Core::User& user : users)
	{
		Core::User updated = user;
		updated.clientId = client.id;
		updated.groups = {};

		for (const QString& groupName : user.groups)
		{
			auto groupIt = std::find_if(client.groups.begin(), client.groups.end(),
			[&groupName](const Core::Group& group)
			{
				return group.name.compare(groupName, Qt::CaseInsensitive);
			});

			updated.groups << groupIt->id;
		}

		result << updated;
	}

	return result;
}

}

UsersTabWidget::UsersTabWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent)
	: QWidget(parent)
	, m_listEditorWidget(backendConnection, parent)
{
	m_ui.setupUi(this);
	m_ui.verticalLayout->addWidget(&m_listEditorWidget);

	connect(m_ui.clientsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, QOverload<int>::of(&UsersTabWidget::updateUsersList));

	connect(m_ui.showAllCheckBox, &QCheckBox::clicked, this,
		QOverload<bool>::of(&UsersTabWidget::updateUsersList));

	connect(m_ui.showAllCheckBox, &QCheckBox::clicked,
		m_ui.clientsComboBox, &QComboBox::setDisabled);

	connect(m_ui.importFromXlsxButton, &QPushButton::clicked, this, &UsersTabWidget::importUsers);
	connect(m_ui.exportToXlsxButton, &QPushButton::clicked, this, &UsersTabWidget::exportUsers);

	connect(backendConnection.get(), &IBackendConnection::clientsLoaded, this, &UsersTabWidget::updateClientsList);
}

void UsersTabWidget::loadData()
{
	m_listEditorWidget.loadData();
}

void UsersTabWidget::setClients(const QList<Core::Client>& clients)
{
	m_clients = clients;
	m_listEditorWidget.setClients(clients);
}

void UsersTabWidget::updateClientsList(IBackendConnection::QueryId /*queryId*/, const QList<Client>& clients)
{
	setClients(clients);

	QList<QString> ids = clientIds(clients);

	int currentClientIndex = -1;
	if (ids.contains(m_currentClient))
	{
		currentClientIndex = m_ui.clientsComboBox->findText(m_currentClient);
	}

	m_ui.clientsComboBox->clear();
	for (const auto& clientId : ids)
	{
		m_ui.clientsComboBox->addItem(clientIdToName(clientId), clientId);
	}

	if (currentClientIndex != -1)
	{
		m_ui.clientsComboBox->setCurrentIndex(currentClientIndex);
	}
}

void UsersTabWidget::updateUsersList(int clientIndex)
{	
	QString clientId = m_ui.clientsComboBox->itemData(clientIndex).toString();
	m_currentClient = clientId;

	enableButtons(!isAdminsGroup(m_currentClient));
	m_listEditorWidget.setClientFilter(m_currentClient);
}

void UsersTabWidget::updateUsersList(bool showAll)
{
	enableButtons(!showAll && !isAdminsGroup(m_currentClient));

	if (showAll)
	{
		m_listEditorWidget.resetClientFilter();
	}
	else
	{
		m_listEditorWidget.setClientFilter(m_currentClient);
	}
}

QString UsersTabWidget::clientIdToName(QString clientId)
{
	if (isAdminsGroup(clientId))
	{
		return "< Администраторы >";
	}

	auto it = std::find_if(m_clients.begin(), m_clients.end(), [&](const Core::Client& client) { return client.id == clientId; });
	return it == m_clients.end() ? "" : it->name;
}

void UsersTabWidget::importUsers()
{
	QString filename = QFileDialog::getOpenFileName(this, "Открытие документа");

	UsersXlsxDocument document(filename);
	bool ok = false;
	QList<Core::User> users = document.read(ok);
	if (!ok)
	{
		QMessageBox::critical(this, "Импорт пользователей", "Не удалось прочитать список пользователей из файла");
		return;
	}

	Core::Client curClient = currentClient();

	QList<QString> validationErrors;
	for (int i = 0; i < users.size(); ++i)
	{
		const Core::User& user = users[i];

		if (user.name.isEmpty())
		{
			validationErrors << ("Пользователь #" + QString::number(i + 1) + ": имя не может быть пустым");
		}
		else
		{
			QList<Core::User> existingUsers = m_listEditorWidget.allUsers();
			auto it = std::find_if(existingUsers.begin(), existingUsers.end(),
				[&user](const Core::User& existingUser) { return existingUser.name.compare(user.name, Qt::CaseInsensitive) == 0; });
			if (it != existingUsers.end())
			{
				validationErrors << ("Пользователь #" + QString::number(i + 1) + ": пользователь с именем \"" + user.name + "\" уже существует в базе");
			}
			else
			{
				QList<QString> duplicates;
				for (int j = 0; j < users.size(); ++j)
				{
					if (i != j && users[j].name.compare(user.name, Qt::CaseInsensitive) == 0)
					{
						duplicates << "#" + QString::number(j + 1);
					}
				}

				if (!duplicates.isEmpty())
				{
					QString error = "Пользователь #" + QString::number(i + 1) + ":" +
						" пользователь с именем \"" + user.name + "\" указан на других строках - " + duplicates.join(", ");
					validationErrors << error;
				}
			}
		}

		if (!user.password || (*user.password).isEmpty())
		{
			validationErrors << ("Пользователь #" + QString::number(i + 1) + ": пароль не может быть пустым");
		}

		QList<QString> unknownGroups;
		for (const QString& groupName : user.groups)
		{
			auto groupIt = std::find_if(curClient.groups.begin(), curClient.groups.end(),
			[&groupName](const Core::Group& group)
			{
				return group.name.compare(groupName, Qt::CaseInsensitive) == 0;
			});
			if (groupIt == curClient.groups.end())
			{
				unknownGroups << groupName;
			}
		}

		if (!unknownGroups.isEmpty())
		{
			validationErrors << ("Пользователь #" + QString::number(i + 1) + ": указаны неизвестные группы - " + unknownGroups.join(", "));
		}
	}

	if (!validationErrors.isEmpty())
	{
		QMessageBox messageBox;
		messageBox.setWindowTitle("Импорт пользователей");
		messageBox.setIcon(QMessageBox::Critical);
		messageBox.setText("Не удалось импортировать список пользователей из файла");
		messageBox.setDetailedText(validationErrors.join("\n"));

		auto buttons = messageBox.buttons();
		auto it = std::find_if(buttons.begin(), buttons.end(),
			[&](QAbstractButton* btn){ return messageBox.buttonRole(btn) == QMessageBox::ActionRole; });
		if (it != buttons.end())
		{
			QAbstractButton* button = *it;
			button->click();
		}

		messageBox.exec();

		return;
	}

	users = prepareToImport(users, curClient);
	m_listEditorWidget.addUsers(users);
}

void UsersTabWidget::exportUsers()
{
	QString filename = QFileDialog::getSaveFileName(this, "Сохранение документа");

	UsersXlsxDocument document(filename);
	bool ok = false;
	QList<Core::User> users = prepareToExport(m_listEditorWidget.currentUsers(), currentClient());
	document.write(users, ok);

	if (!ok)
	{
		QMessageBox::critical(this, "Экспорт пользователей", "Не удалось записать список пользователей в файл");
	}
	else
	{
		QMessageBox::information(this, "Экспорт пользователей", "Экспорт пользователей завершился успешно");
	}
}

Core::Client UsersTabWidget::currentClient() const
{
	Q_ASSERT(!isAdminsGroup(m_currentClient));

	auto it = std::find_if(m_clients.begin(), m_clients.end(),
	[this](const Core::Client& client)
	{
		return client.id == m_currentClient;
	});
	Q_ASSERT(it != m_clients.end());

	return *it;
}

void UsersTabWidget::enableButtons(bool enabled)
{
	m_ui.importFromXlsxButton->setEnabled(enabled);
	m_ui.exportToXlsxButton->setEnabled(enabled);

	const QString tooltip = enabled ? "" : "Нужно выбрать клиента";

	m_ui.importFromXlsxButton->setToolTip(tooltip);
	m_ui.exportToXlsxButton->setToolTip(tooltip);
}
