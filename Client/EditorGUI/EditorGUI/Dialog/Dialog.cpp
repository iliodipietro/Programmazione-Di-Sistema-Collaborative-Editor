#include "Dialog.h"

Dialog::Dialog(QString link, QWidget* parent)
	: QDialog(parent), link(link)
{

	ui.setupUi(this);
	//ui.plainTextEdit->setPlainText(link);
	ui.textEdit->setText(link);
}

Dialog::~Dialog()
{
}

void Dialog::closeEvent(QCloseEvent* event) {
	this->reject();  //devo fare dialog close credo
}

void Dialog::on_pushButton_clicked() {
	
	this->accept();
}

void Dialog::on_cancel_clicked() {
	this->reject();
}
