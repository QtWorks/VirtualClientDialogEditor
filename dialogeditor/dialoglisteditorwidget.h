#pragma once

#include "core/ibackendconnection.h"
#include "core/dialog.h"
#include "listeditorwidget.h"
#include "listdatamodel.h"

class DialogListEditorWidget
	: public ListEditorWidget
{
public:
	DialogListEditorWidget(IBackendConnectionSharedPtr backendConnection, QWidget* parent = nullptr);

	void loadData();

private:
	virtual QStringList items() const override;
	virtual void saveChanges() override;
	virtual bool itemHasChanges(const QString& item) const override;
	virtual void revertChanges(const QString& item) override;
	virtual void revertAllChanges() override;

private slots:
	void onItemEditRequested(const QString& dialogName);
	void onItemsRemoveRequested(const QStringList& dialogs);
	void onItemCreateRequested();

	void onDiffRecordsCountChanged(int count);

	void onDialogsLoaded(const QList<Core::Dialog>& dialogs);
	void onDialogsLoadFailed(const QString& error);
	void onDialogsUpdated();
	void onDialogsUpdateFailed(const QString& error);

private:
	IBackendConnectionSharedPtr m_backendConnection;

	typedef ListDataModel<Core::Dialog> DialogListDataModel;
	DialogListDataModel m_model;

	bool m_updating;
};
