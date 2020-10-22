#pragma once

#include <QWidget>
#include <QDialog>
#include "ui_Dialog.h"

class Dialog : public QDialog
{
	Q_OBJECT

public:
	Dialog(QString link, QWidget *parent = Q_NULLPTR);
	~Dialog();

private:
	Ui::Dialog ui;
	QString link;

	void closeEvent(QCloseEvent* event);

private slots:
	void on_pushButton_clicked();
	void on_cancel_clicked();
	//void dialogClosed(QAbstractButton* button);
};
