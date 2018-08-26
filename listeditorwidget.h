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

signals:
	void itemEditRequested(QString item);
	void itemCreateRequested();

protected:
	void showProgressDialog(const QString& title, const QString& label);
	void hideProgressDialog();

private slots:
	void onAddButtonClicked();
	void onEditButtonClicked();
	void onRemoveButtonClicked();

	void onSelectionChanged();

private:
	virtual QStringList items() const = 0;
	virtual void removeItems(const QStringList& items) = 0;

private:
	Ui::ListEditorWidget* m_ui;

	std::unique_ptr<QProgressDialog> m_progressDialog;
};
