#pragma once

#include <QWidget>
#include <QPushButton>
#include <QStringList>
#include <QListWidget>
#include <QStyledItemDelegate>
#include <QProgressDialog>
#include <memory>

namespace Ui {
class ListEditorWidget;
}

class CustomFontDelegate
	: public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit CustomFontDelegate(QObject* parent = 0);

	virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

class ListEditorWidget
	: public QWidget
{
	Q_OBJECT

public:
	explicit ListEditorWidget(QWidget* parent = 0);
	~ListEditorWidget();

	void updateData();

	void updateItem(int index);
	void addItem(const QString& item);
	void removeItem(const QString& item);

signals:
	void itemEditRequested(QString item);
	void itemsRemoveRequested(QStringList items);
	void itemCreateRequested();

	void revertAllChangesRequested();
	void revertChangesRequested(QString item);
	void saveAllChangesRequested();

protected:
	void showProgressDialog(const QString& title, const QString& label);
	void hideProgressDialog();

protected:
	QPushButton* m_saveButton;
	QPushButton* m_revertButton;
	QPushButton* m_revertAllButton;

private slots:
	void onAddButtonClicked();
	void onEditButtonClicked();
	void onRemoveButtonClicked();

	void onSaveButtonClicked();
	void onRevertButtonClicked();
	void onRevertAllButtonClicked();

	void onSelectionChanged();

private:
	virtual QStringList items() const = 0;
	virtual bool itemHasChanges(const QString& item) const = 0;
	virtual bool itemIsAdded(const QString& item) const = 0;
	virtual void saveChanges() = 0;
	virtual void revertChanges(const QString& item) = 0;
	virtual void revertAllChanges() = 0;

	void setRowBackground(int index, const QColor& color);

private:
	Ui::ListEditorWidget* m_ui;

	std::unique_ptr<QProgressDialog> m_progressDialog;
};
