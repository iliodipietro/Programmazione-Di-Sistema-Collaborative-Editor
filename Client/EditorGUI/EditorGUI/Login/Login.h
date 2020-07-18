#pragma once

#include <QtWidgets/QMainWindow>
#include <QCloseEvent>
#include <QMessageBox>
#include "ui_Login.h"
#include "FileBrowser/FileBrowser.h"
#include "NewAccount/NewAccount.h"

class Login : public QMainWindow
{
	Q_OBJECT

public:
	Login(QWidget *parent = Q_NULLPTR);
	~Login();

private:
	bool newWindow = false;
	Ui::LoginClass ui;
	FileBrowser* FileBrowserWindow;
	NewAccount* NewAccountWindow;
	void closeEvent(QCloseEvent* event);

private slots:
	void on_loginButton_clicked();
	void on_newAccount_clicked();
};
