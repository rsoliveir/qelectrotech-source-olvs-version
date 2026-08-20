/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "diagrampropertiesdialog.h"

#include "../diagram.h"
#include "../diagramcommands.h"
#include "../undocommand/changetitleblockcommand.h"
#include "borderpropertieswidget.h"
#include "conductorpropertieswidget.h"
#include "projectpropertiesdialog.h"
#include "titleblockpropertieswidget.h"

#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>

/**
	@brief DiagramPropertiesDialog::DiagramPropertiesDialog
	Default constructor
	@param diagram : diagram to edit properties
	@param parent : parent widget
*/
DiagramPropertiesDialog::DiagramPropertiesDialog(Diagram *diagram, QWidget *parent) :
	QDialog (parent),
	m_diagram (diagram)
{
	bool diagram_is_read_only = diagram -> isReadOnly();

	// Get some properties of edited diagram
	TitleBlockProperties titleblock = diagram -> border_and_titleblock.exportTitleBlock();
	BorderProperties     border     = diagram -> border_and_titleblock.exportBorder();
	ConductorProperties  conductors = diagram -> defaultConductorProperties;

	setWindowModality(Qt::WindowModal);
#ifdef Q_OS_MACOS
	setWindowFlags(Qt::Sheet);
#endif

	setWindowTitle(tr("Propriétés du folio", "window title"));

	//Border widget
	BorderPropertiesWidget *border_infos = new BorderPropertiesWidget(border, this);
	border_infos -> setReadOnly(diagram_is_read_only);

	//Title block widget
	TitleBlockPropertiesWidget  *titleblock_infos;

	if (QETProject *parent_project = diagram -> project())
		titleblock_infos = new TitleBlockPropertiesWidget(parent_project -> embeddedTitleBlockTemplatesCollection(), titleblock, false, diagram->project(), this);
	else
		titleblock_infos = new TitleBlockPropertiesWidget(titleblock, false, diagram->project(), this);

	titleblock_infos -> setReadOnly(diagram_is_read_only);
	connect(titleblock_infos, &TitleBlockPropertiesWidget::openAutoNumFolioEditor, this, &DiagramPropertiesDialog::editAutoFolioNum);
	//titleblock_infos->setMinimumSize(590,480); //Minimum Size needed for correct display

		//Conductor widget
	m_cpw = new ConductorPropertiesWidget(conductors, this);
	m_cpw -> setReadOnly(diagram_is_read_only);

	QComboBox *autonum_combobox = m_cpw->autonumComboBox();
	autonum_combobox->addItems(diagram->project()->conductorAutoNum().keys());
	autonum_combobox->setCurrentIndex(autonum_combobox->findText(diagram->conductorsAutonumName()));

	connect(m_cpw->editAutonumPushButton(), &QPushButton::clicked, this, &DiagramPropertiesDialog::editAutonum);

	// OLVS-version edition — physical page size & contractual plot scale
	QGroupBox *scale_group = new QGroupBox(tr("Physical page && plot scale"), this);
	QComboBox *page_size_cb = new QComboBox(scale_group);
	page_size_cb->addItem(tr("Custom (fit to page)"), static_cast<int>(Diagram::PageSize::Custom));
	page_size_cb->addItem(tr("A4"),  static_cast<int>(Diagram::PageSize::A4));
	page_size_cb->addItem(tr("A3"),  static_cast<int>(Diagram::PageSize::A3));
	page_size_cb->addItem(tr("Letter"), static_cast<int>(Diagram::PageSize::Letter));
	int current_page_size_index = page_size_cb->findData(static_cast<int>(diagram->pageSize()));
	page_size_cb->setCurrentIndex(current_page_size_index >= 0 ? current_page_size_index : 0);
	page_size_cb->setEnabled(!diagram_is_read_only);

	// OLVS-version edition — standard engineering scale presets
	QComboBox *scale_preset_cb = new QComboBox(scale_group);
	const QList<qreal> standard_scales = {1.0, 2.0, 5.0, 10.0, 20.0, 25.0, 50.0, 100.0};
	for (qreal s : standard_scales) {
		scale_preset_cb->addItem(tr("1 : %1").arg(s), s);
	}
	scale_preset_cb->addItem(tr("Custom"), -1.0);

	QDoubleSpinBox *scale_sb = new QDoubleSpinBox(scale_group);
	scale_sb->setRange(1.0, 1000.0);
	scale_sb->setDecimals(0);
	scale_sb->setValue(diagram->scaleDenominator());
	scale_sb->setPrefix(tr("1 : "));
	scale_sb->setEnabled(!diagram_is_read_only);

	// Detect whether the current value matches a known preset
	int preset_index = scale_preset_cb->count() - 1; // default: Custom
	for (int i = 0; i < standard_scales.size(); ++i) {
		if (qFuzzyCompare(diagram->scaleDenominator(), standard_scales.at(i))) {
			preset_index = i;
			break;
		}
	}
	scale_preset_cb->setCurrentIndex(preset_index);
	scale_sb->setVisible(preset_index == scale_preset_cb->count() - 1);
	scale_preset_cb->setEnabled(!diagram_is_read_only);

	connect(scale_preset_cb, QOverload<int>::of(&QComboBox::currentIndexChanged), scale_sb,
		[scale_preset_cb, scale_sb](int index) {
			qreal data = scale_preset_cb->itemData(index).toDouble();
			bool is_custom = (data < 0);
			scale_sb->setVisible(is_custom);
			if (!is_custom) {
				scale_sb->setValue(data);
			}
		});

	QFormLayout *scale_layout = new QFormLayout(scale_group);
	scale_layout->addRow(tr("Page size:"), page_size_cb);
	scale_layout->addRow(tr("Plot scale:"), scale_preset_cb);
	scale_layout->addRow(QString(), scale_sb); // hidden unless "Custom" is selected

		// Buttons
	QDialogButtonBox boutons(diagram_is_read_only ? QDialogButtonBox::Ok : QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(&boutons, &QDialogButtonBox::accepted, this, &DiagramPropertiesDialog::accept);
	connect(&boutons, &QDialogButtonBox::rejected, this, &DiagramPropertiesDialog::reject);

	QGridLayout *glayout = new QGridLayout;
	glayout->addWidget(border_infos,0,0);
	glayout->addWidget(titleblock_infos, 1, 0);
	glayout->addWidget(m_cpw, 0, 1, 0, 1);
	glayout->addWidget(scale_group, 2, 0); // OLVS-version edition — left column, below title block info

	QVBoxLayout vlayout(this);
	vlayout.addLayout(glayout);
	vlayout.addWidget(&boutons);

	// if dialog is accepted
	if (this -> exec() == QDialog::Accepted && !diagram_is_read_only)
	{
		TitleBlockProperties new_titleblock = titleblock_infos  -> properties();
		BorderProperties     new_border     = border_infos -> properties();
		ConductorProperties  new_conductors = m_cpw -> properties();

		// Title block have change
		if (new_titleblock != titleblock) {
			diagram -> undoStack().push(new ChangeTitleBlockCommand(diagram, titleblock, new_titleblock));
		}

		// Page size / plot scale change — OLVS-version edition
		auto new_page_size = static_cast<Diagram::PageSize>(page_size_cb->currentData().toInt());
		qreal new_scale = scale_sb->value();
		if (new_page_size != diagram->pageSize() || !qFuzzyCompare(new_scale, diagram->scaleDenominator())) {
    		#if TODO_LIST
    		#pragma message("@TODO implement an undo command for page size / scale changes")
    		#endif
    			diagram->setPageSize(new_page_size);
    			diagram->setScaleDenominator(new_scale);
		}

		// Border have change
		if (new_border != border) {
			diagram -> undoStack().push(new ChangeBorderCommand(diagram, border, new_border));
		}

		// Conducteur have change
		if (new_conductors != conductors) {
#if TODO_LIST
#pragma message("@TODO implement an undo command to allow the user to undo/redo this action")
#endif
			/// TODO implement an undo command to allow the user to undo/redo this action
			diagram -> defaultConductorProperties = new_conductors;
		}

			// Conductor autonum name
		if (autonum_combobox->currentText() != diagram->conductorsAutonumName())
		{
			diagram->setConductorsAutonumName (autonum_combobox->currentText());
			diagram->project()->conductorAutoNumChanged();
		}
	}
}

/**
	@brief DiagramPropertiesDialog::diagramPropertiesDialog
	Static method to get a DiagramPropertiesDialog.
	@param diagram : diagram to edit properties
	@param parent : parent widget
*/
void DiagramPropertiesDialog::diagramPropertiesDialog(Diagram *diagram, QWidget *parent) {
	DiagramPropertiesDialog dialog(diagram, parent);
}

/**
	@brief DiagramPropertiesDialog::editAutonum
	Open conductor autonum editor
*/
void DiagramPropertiesDialog::editAutonum()
{
	ProjectPropertiesDialog ppd (m_diagram->project(), this);
	ppd.setCurrentPage(ProjectPropertiesDialog::Autonum);
	ppd.exec();
	m_cpw->autonumComboBox()->clear();
	m_cpw->autonumComboBox()->addItems(m_diagram->project()->conductorAutoNum().keys());
}

/**
	@brief DiagramPropertiesDialog::editAutonum
	Open folio autonum editor
*/
void DiagramPropertiesDialog::editAutoFolioNum ()
{
	ProjectPropertiesDialog ppd (m_diagram->project(), this);
	ppd.setCurrentPage(ProjectPropertiesDialog::Autonum);
	ppd.changeToFolio();
	ppd.exec();
}
