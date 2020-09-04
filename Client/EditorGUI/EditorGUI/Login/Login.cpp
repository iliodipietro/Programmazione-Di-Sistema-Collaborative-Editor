#include "Login.h"

Login::Login(QWidget* parent)
	: QMainWindow(parent),
	m_socketHandler(QSharedPointer<SocketHandler>(new SocketHandler())),
	m_timer(QSharedPointer<QTimer>(new QTimer(this)))
{
	ui.setupUi(this);
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &Login::loginResult);
	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(showErrorMessage()));
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

void Login::openFileBrowser(QSharedPointer<QPixmap> profileImage) {
	m_timer->stop();
	QString username = ui.usernameTextLine->text();
	resetWindows();
	m_fileBrowserWindow = new FileBrowser(m_socketHandler, profileImage, username);
	m_fileBrowserWindow->show();
	this->newWindow = true;
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
	QJsonObject message = Serialize::userSerialize(username, password, username, LOGIN);
	bool result = m_socketHandler->writeData(Serialize::fromObjectToArray(message));
	if (result) {
		m_timer->setSingleShot(true);
		m_timer->setInterval(3000);
		m_timer->start();
		//openFileBrowser(); //da commentare in seguito ed aggiustare le condizioni degli if
	}
	else {
		qDebug() << m_socketHandler->getSocketState();
	}
}

void Login::showErrorMessage() {
	qDebug() << "messaggio di errore per il login";
}

void Login::loginResult(QJsonObject response) {
	m_timer->stop();
	QStringList serverMessage = Serialize::responseUnserialize(response);
	bool result = serverMessage[0] == "true" ? true : false;
	if (result) {
		QString profileImageBase64 = serverMessage[1];
		QSharedPointer<QPixmap> profileImage = QSharedPointer<QPixmap>(new QPixmap());
		profileImage->loadFromData(QByteArray::fromBase64(profileImageBase64.toLatin1()));
		openFileBrowser(profileImage);
	}
	else {
		QMessageBox resultDialog(this);
		QString res_text = response.value("message").toString();
		resultDialog.setInformativeText(res_text); //mettere il messaggio di errore contenuto nel Json di risposta
		resultDialog.exec();
	}
}

void Login::on_newAccount_clicked() {
	resetWindows();
	m_newAccountWindow = new NewAccount(m_socketHandler);
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
		m_fileBrowserWindow->deleteLater();
		m_fileBrowserWindow = Q_NULLPTR;
	}
	if (m_newAccountWindow != Q_NULLPTR) {
		m_newAccountWindow->deleteLater();
		m_newAccountWindow = Q_NULLPTR;
	}
}

void Login::hideEvent(QHideEvent* event) {
	//setWindowState(Qt::WindowMinimized);
}