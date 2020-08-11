#include "Login.h"

Login::Login(QWidget* parent)
	: QMainWindow(parent), m_messageSerializer(QSharedPointer<Serialize>(new Serialize())),
	m_socketHandler(QSharedPointer<SocketHandler>(new SocketHandler())),
	m_timer(QSharedPointer<QTimer>(new QTimer(this)))
{
	ui.setupUi(this);
	connect(m_socketHandler.get(), SIGNAL(SocketHandler::dataReceived(QJsonObject)), this, SLOT(loginResult(QJsonObject)));
	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(showErrorMessage()));
	m_socketHandler->connectToServer();
	m_fileBrowserWindow = Q_NULLPTR;
	m_newAccountWindow = Q_NULLPTR;
}

Login::~Login()
{
	
}

void Login::closeEvent(QCloseEvent* event)
{
	qApp->quit();
}

void Login::openFileBrowser() {
	QString username = ui.usernameTextLine->text();
	resetWindows();
	m_fileBrowserWindow = new FileBrowser(m_socketHandler, m_messageSerializer, this, username);
	m_fileBrowserWindow->show();
	this->newWindow = true;
	m_timer->stop();
	connect(m_fileBrowserWindow, &FileBrowser::showParent, this, &Login::childWindowClosed);
	this->hide();
}

void Login::on_loginButton_clicked()
{
	QString username = ui.usernameTextLine->text();
	QString password = ui.passwordTextLine->text();
	//QString loginInfo = "";
	//loginInfo.append(username).append(",").append(password);
	//SocketMessage m(MessageTypes::LoginMessage, loginInfo.toUtf8());
	QJsonObject message = m_messageSerializer->userSerialize(username, password, username, 1);
	bool result = m_socketHandler->writeData(m_messageSerializer->fromObjectToArray(message));
	if (true) {
		m_timer->setSingleShot(true);
		m_timer->setInterval(1000);
		m_timer->start();
		openFileBrowser(); //da commentare in seguito ed aggiustare le condizioni degli if
	}
	else {
		qDebug() << m_socketHandler->getSocketState();
	}
}

void Login::showErrorMessage() {
	qDebug() << "messaggio di errore per il login";
}

void Login::loginResult(QJsonObject response) {
	QStringList l = m_messageSerializer->responseUnserialize(response);
	int result = l[0].toInt();

	if (true) {
		openFileBrowser();
	}
	else {
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText(""); //mettere il messaggio di errore contenuto nel Json di risposta
		resultDialog.exec();
	}
}

void Login::on_newAccount_clicked() {
	resetWindows();
	m_newAccountWindow = new NewAccount(m_socketHandler, this);
	m_newAccountWindow->show();
	this->newWindow = true;
	connect(m_newAccountWindow, &NewAccount::showParent, this, &Login::childWindowClosed);
	this->hide();
}

void Login::childWindowClosed() {
	this->show();
}

void Login::resetWindows() {
	if (m_fileBrowserWindow != Q_NULLPTR) {
		delete m_fileBrowserWindow;
		m_fileBrowserWindow = Q_NULLPTR;
	}
	if (m_newAccountWindow != Q_NULLPTR) {
		delete m_newAccountWindow;
		m_newAccountWindow = Q_NULLPTR;
	}
}