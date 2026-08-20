#pragma once

#include <QDialog>
#include "../diagram.h"

class QETProject;
class QListWidget;

/// OLVS-version edition
/**
	@brief The FolioGroupOrderDialog class
	Lets the user reorder the folio type groups (Diagrams / Panel Layout /
	Documentation) displayed in the project tree.
*/
class FolioGroupOrderDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit FolioGroupOrderDialog(QETProject *project, QWidget *parent = nullptr);
		~FolioGroupOrderDialog() override;

		/// Shows the dialog and, if accepted, applies the new order to \a project.
		static void editOrder(QETProject *project, QWidget *parent = nullptr);

	private slots:
		void moveSelectedUp();
		void moveSelectedDown();

	private:
		static QString labelForType(Diagram::FolioType type);

		QETProject *m_project;
		QListWidget *m_list_widget;
};
