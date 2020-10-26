#pragma once

#include <QtWidgets/QMainWindow>
#include <QCloseEvent>
#include <QMessageBox>
#include <QThread>
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
	int clientID;
	QThread* m_thread;
	QString m_username;
	QString m_email;
	Ui::LoginClass ui;
	FileBrowser* m_fileBrowserWindow;
	NewAccount* m_newAccountWindow;
	QSharedPointer<SocketHandler> m_socketHandler;
	//timer da usare per mostrare un messaggio di errore nel caso in cui la risposta dal server non sia arrivata entro un determinato tempo
	QSharedPointer<QTimer> m_timer;

	
	void closeEvent(QCloseEvent* event);
	void resetWindows();
	void openFileBrowser(QSharedPointer<QPixmap> profileImage, QSharedPointer<QPixmap> profileImageResized, QColor userColor);
	

private slots:
	void on_loginButton_clicked();
	void on_newAccount_clicked();
	void loginResult(QJsonObject response);
	void showErrorMessage();
    void childWindowClosed();
	void on_textChanged();

protected:
	void hideEvent(QHideEvent* event);

signals:
	void dataToSend(QByteArray);
};
