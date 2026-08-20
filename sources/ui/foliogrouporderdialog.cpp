#include "foliogrouporderdialog.h"
#include "../qetproject.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

/// OLVS-version edition
FolioGroupOrderDialog::FolioGroupOrderDialog(QETProject *project, QWidget *parent) :
	QDialog(parent),
	m_project(project)
{
	setWindowTitle(tr("Folio group order"));

	m_list_widget = new QListWidget(this);
	for (auto type : project->folioGroupOrder()) {
		auto *item = new QListWidgetItem(labelForType(type));
		item->setData(Qt::UserRole, static_cast<int>(type));
		m_list_widget->addItem(item);
	}

	auto *up_button = new QPushButton(tr("Move up"), this);
	auto *down_button = new QPushButton(tr("Move down"), this);
	connect(up_button, &QPushButton::clicked, this, &FolioGroupOrderDialog::moveSelectedUp);
	connect(down_button, &QPushButton::clicked, this, &FolioGroupOrderDialog::moveSelectedDown);

	auto *button_layout = new QVBoxLayout();
	button_layout->addWidget(up_button);
	button_layout->addWidget(down_button);
	button_layout->addStretch();

	auto *list_layout = new QHBoxLayout();
	list_layout->addWidget(m_list_widget);
	list_layout->addLayout(button_layout);

	auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto *main_layout = new QVBoxLayout(this);
	main_layout->addLayout(list_layout);
	main_layout->addWidget(box);
	setLayout(main_layout);
}

FolioGroupOrderDialog::~FolioGroupOrderDialog()
{
}

QString FolioGroupOrderDialog::labelForType(Diagram::FolioType type)
{
	switch (type) {
		case Diagram::FolioType::PanelLayout:   return tr("Panel Layout");
		case Diagram::FolioType::Documentation: return tr("Documentation");
		case Diagram::FolioType::Schematic:
		default:                                return tr("Diagrams");
	}
}

void FolioGroupOrderDialog::moveSelectedUp()
{
	int row = m_list_widget->currentRow();
	if (row <= 0) return;
	QListWidgetItem *item = m_list_widget->takeItem(row);
	m_list_widget->insertItem(row - 1, item);
	m_list_widget->setCurrentRow(row - 1);
}

void FolioGroupOrderDialog::moveSelectedDown()
{
	int row = m_list_widget->currentRow();
	if (row < 0 || row >= m_list_widget->count() - 1) return;
	QListWidgetItem *item = m_list_widget->takeItem(row);
	m_list_widget->insertItem(row + 1, item);
	m_list_widget->setCurrentRow(row + 1);
}

void FolioGroupOrderDialog::editOrder(QETProject *project, QWidget *parent)
{
	if (!project) return;

	FolioGroupOrderDialog dialog(project, parent);
	if (dialog.exec() != QDialog::Accepted) return;

	QList<Diagram::FolioType> new_order;
	for (int i = 0; i < dialog.m_list_widget->count(); ++i) {
		QListWidgetItem *item = dialog.m_list_widget->item(i);
		new_order << static_cast<Diagram::FolioType>(item->data(Qt::UserRole).toInt());
	}

	project->setFolioGroupOrder(new_order);
}
