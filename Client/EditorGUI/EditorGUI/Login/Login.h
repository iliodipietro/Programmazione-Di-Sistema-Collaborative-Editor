#pragma once

#include <QtWidgets/QMainWindow>
#include <QCloseEvent>
#include <QMessageBox>
#include "ui_Login.h"
#include "FileBrowser/FileBrowser.h"
#include "NewAccount/NewAccount.h"
#include "SocketHandler/SocketHandler.h"
#include "Serialization/Serialize.h"

class Login : public QMainWindow
{
	Q_OBJECT

public:
	Login(QWidget* parent = Q_NULLPTR);
	~Login();

private:
	bool newWindow = false;
	Ui::LoginClass ui;
	FileBrowser* m_fileBrowserWindow;
	NewAccount* m_newAccountWindow;
	QSharedPointer<SocketHandler> m_socketHandler;
	QSharedPointer<QTimer> m_timer;

	void openFileBrowser(QSharedPointer<QPixmap> profileImage);
	void closeEvent(QCloseEvent* event);
	void resetWindows();

private slots:
	void on_loginButton_clicked();
	void on_newAccount_clicked();
	void loginResult(QJsonObject response);
	void showErrorMessage();
	void childWindowClosed();

protected:
	void hideEvent(QHideEvent* event);
};
