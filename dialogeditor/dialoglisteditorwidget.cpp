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
	connect(this, &ListEditorWidget::itemsRemoveRequested, this, &DialogListEditorWidget::onItemsRemoveRequested);
	connect(this, &ListEditorWidget::itemCreateRequested, this, &DialogListEditorWidget::onItemCreateRequested);

	connect(&m_model, &IListDataModel::diffRecordsCountChanged, this, &DialogListEditorWidget::onDiffRecordsCountChanged);
	onDiffRecordsCountChanged(0);

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

bool DialogListEditorWidget::itemHasChanges(const QString& dialogName) const
{
	const DialogListDataModel::Index index = m_model.findIndex([&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
	return m_model.hasChanges(index);
}

void DialogListEditorWidget::saveChanges()
{
	showProgressDialog("Сохранение данных", "Идет сохранение данных. Пожалуйста, подождите.");

	m_updating = true;
	m_backendConnection->updateDialogs({ m_model.updated(), m_model.deleted(), m_model.added() });
}

void DialogListEditorWidget::revertChanges(const QString& dialogName)
{
	const DialogListDataModel::Index index = m_model.findIndex([&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
	m_model.revert(index);
}

void DialogListEditorWidget::revertAllChanges()
{
	m_model.revertAll();
}

void DialogListEditorWidget::onItemEditRequested(const QString& dialogName)
{
	const DialogListDataModel::Index index = m_model.findIndex([&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
	Q_ASSERT(index != -1);

	DialogEditorWindow* window = new DialogEditorWindow(m_model.get(index));
	connect(window, &DialogEditorWindow::dialogChanged, [this, index, dialogName](Core::Dialog dialog)
	{
		m_model.update(index, dialog);

		//Core::DialogJsonWriter writer;
		//LOG << writer.write(dialog);

		updateItem(dialogName, dialog.printableName());
	});

	window->show();
}

void DialogListEditorWidget::onItemsRemoveRequested(const QStringList& dialogs)
{
	QMessageBox messageBox(QMessageBox::Question,
		"Удаление диалогов",
		"Вы действительно хотите удалить " + QString(dialogs.size() > 1 ? "выбранные диалоги" : "выбранный диалог") + "?",
		QMessageBox::Yes | QMessageBox::No,
		this);
	messageBox.setButtonText(QMessageBox::Yes, tr("Да"));
	messageBox.setButtonText(QMessageBox::No, tr("Нет"));

	const int answer = messageBox.exec();
	if (answer != QMessageBox::Yes)
	{
		return;
	}

	for (const QString& dialogName : dialogs)
	{
		const DialogListDataModel::Index index = m_model.findIndex([&dialogName](const Core::Dialog& dialog){ return dialog.printableName() == dialogName; });
		Q_ASSERT(index != -1);
		m_model.remove(index);

		removeItem(dialogName);
	}
}

void DialogListEditorWidget::onItemCreateRequested()
{
	const Core::Dialog dialog = { "", Core::Dialog::Difficulty::Easy, { } };
	DialogEditorWindow* window = new DialogEditorWindow(dialog);

	connect(window, &DialogEditorWindow::dialogChanged, [this](Core::Dialog dialog)
	{
		m_model.append(dialog);

		Core::DialogJsonWriter writer;
		LOG << writer.write(dialog);

		addItem(dialog.printableName());
	});

	window->show();
}

void DialogListEditorWidget::onDiffRecordsCountChanged(int count)
{
	m_saveButton->setEnabled(count > 0);
	m_revertAllButton->setEnabled(count > 0);
}

void DialogListEditorWidget::onDialogsLoaded(const QList<Core::Dialog>& dialogs)
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

void DialogListEditorWidget::onDialogsLoadFailed(const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Загрузка данных", "Загрузка данных завершилась ошибкой: " + toLowerCase(error) + ".");
}

void DialogListEditorWidget::onDialogsUpdated()
{
	hideProgressDialog();

	loadData();
}

void DialogListEditorWidget::onDialogsUpdateFailed(const QString& error)
{
	hideProgressDialog();

	QMessageBox::warning(this, "Сохранение данных", "Сохранение данных завершилось ошибкой: " + toLowerCase(error) + ".");
}
