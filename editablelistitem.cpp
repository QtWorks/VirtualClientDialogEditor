#include "editablelistitem.h"
#include "ui_editablelistitem.h"

EditableListItem::EditableListItem(const QString& label, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::EditableListItem)
{
	m_ui->setupUi(this);

	setLabel(label);
	connect(m_ui->editItemButton, &QPushButton::clicked, [this]() { emit editRequested(); });
	connect(m_ui->removeItemButton, &QPushButton::clicked, [this]() { emit removeRequested(); });

	//m_ui->editItemButton->setIcon(QIcon("<imagePath>"));
	//m_ui->editItemButton->setIconSize(QSize(65,65));

	//m_ui->removeItemButton->setIcon(QIcon("<imagePath>"));
	//m_ui->removeItemButton->setIconSize(QSize(65,65));
}

EditableListItem::~EditableListItem()
{
	delete m_ui;
}

void EditableListItem::setLabel(const QString& label)
{
	m_ui->itemLabel->setText(label);
}
