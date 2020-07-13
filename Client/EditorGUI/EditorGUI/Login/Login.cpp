#include "Login.h"

Login::Login(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//this->setAttribute(Qt::WA_DeleteOnClose);
}

Login::~Login()
{
}

void Login::closeEvent(QCloseEvent* event)
{
	if (this->newWindow) {
		event->accept();
	}
	else {
		qApp->quit();
	}
}

void Login::on_loginButton_clicked()
{
	QString username = ui.usernameTextLine->text();
	QString password = ui.passwordTextLine->text();
	this->FileBrowserWindow = new FileBrowser(Q_NULLPTR, username);
	this->FileBrowserWindow->show();
	this->newWindow = true;
	this->close();
}

void Login::on_newAccount_clicked() {
	this->NewAccountWindow = new NewAccount(Q_NULLPTR);
	this->NewAccountWindow->show();
	this->newWindow = true;
	this->close();
}
