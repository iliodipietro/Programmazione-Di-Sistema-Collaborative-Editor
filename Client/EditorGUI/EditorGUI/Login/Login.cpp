#include "Login.h"

Login::Login(QWidget* parent)
	: QMainWindow(parent),
	m_socketHandler(QSharedPointer<SocketHandler>(new SocketHandler())),
	m_timer(QSharedPointer<QTimer>(new QTimer(this))), m_thread(new QThread(this))
{
	ui.setupUi(this);
	m_socketHandler->moveToThread(m_thread);
	connect(m_thread, &QThread::started, m_socketHandler.get(), &SocketHandler::run);
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &Login::loginResult);
	connect(this, &Login::dataToSend, m_socketHandler.get(), &SocketHandler::writeData, Qt::QueuedConnection);
	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(showErrorMessage()));
	m_thread->start();
	m_fileBrowserWindow = Q_NULLPTR;
	m_newAccountWindow = Q_NULLPTR;
	//ilio
	ui.loginButton->setEnabled(false);
	connect(ui.usernameTextLine, &QLineEdit::textChanged, this, &Login::on_textChanged);
	connect(ui.passwordTextLine, &QLineEdit::textChanged, this, &Login::on_textChanged);
}

Login::~Login()
{
	m_thread->quit();
	m_thread->deleteLater();
}

void Login::closeEvent(QCloseEvent* event)
{
	qApp->quit();
}

void Login::openFileBrowser(QSharedPointer<QPixmap> profileImage, QSharedPointer<QPixmap> profileImageResized, QColor userColor) {
	m_timer->stop();
	resetWindows();
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &Login::loginResult);// altrimenti arriva sempre al login anche quando non deve
	m_fileBrowserWindow = new FileBrowser(m_socketHandler, profileImage, profileImageResized, userColor,  m_email, m_username, clientID);
	m_fileBrowserWindow->show();
	this->newWindow = true;
	connect(m_fileBrowserWindow, &FileBrowser::showParent, this, &Login::childWindowClosed);
	
	this->hide();
}

void Login::on_loginButton_clicked()
{
	m_username = ui.usernameTextLine->text();
	QString password = ui.passwordTextLine->text();
	if (m_username != "" && password != "") {
		//QString loginInfo = "";
		//loginInfo.append(username).append(",").append(password);
		//SocketMessage m(MessageTypes::LoginMessage, loginInfo.toUtf8());
		QJsonObject message = Serialize::userSerialize(m_username, password, m_username, LOGIN);
		//bool result = m_socketHandler->writeData(Serialize::fromObjectToArray(message));
		emit dataToSend(Serialize::fromObjectToArray(message));
		//if (result) {
		//	m_timer->setSingleShot(true);
		//	m_timer->setInterval(3000);
		//	m_timer->start();
		//	//openFileBrowser(); //da commentare in seguito ed aggiustare le condizioni degli if
		//}
		//else {
		//	qDebug() << m_socketHandler->getSocketState();
		//}
	}
	else {
		//QMessageBox errorDialog(this);
		//errorDialog.setInformativeText("L'username e la password non possono essere vuoti");
		//errorDialog.exec();
	}
}

//dialog per mostrare un errore di connessione se la risposta dal server non arriva in tempo
void Login::showErrorMessage() {
	//QMessageBox errorDialog(this);
	//errorDialog.setInformativeText("errore di connessione");
	//errorDialog.exec();
	//qDebug() << "messaggio di errore per il login";
}

void Login::loginResult(QJsonObject response) {
	//m_timer->stop();
	QStringList serverMessage = Serialize::responseUnserialize(response);
	bool result = serverMessage[0] == "true" ? true : false;

	if (result) {
		this->clientID = serverMessage[2].toInt();
		this->m_email = serverMessage[4];
		QString profileImageBase64 = serverMessage[1];
		QPixmap profileImage;
		QByteArray latin = profileImageBase64.toLatin1();
		profileImage.loadFromData(QByteArray::fromBase64(profileImageBase64.toLatin1()));
		QPixmap resizedProfileImage = profileImage.scaled(QSize(60,60), Qt::KeepAspectRatio);
		profileImage = profileImage.scaled(QSize(125, 125), Qt::KeepAspectRatio);
		QPixmap target(QSize(60, 60));
		target.fill(Qt::transparent);
		QPainter painter(&target);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
		QPainterPath path;
		path.addRoundedRect(0, 0, 60, 60, 30, 30);
		painter.setClipPath(path);
		painter.drawPixmap(0, 0, resizedProfileImage);
		QSharedPointer<QPixmap> roundedProfileImageResized = QSharedPointer<QPixmap>(new QPixmap(target));
		QPixmap target2(QSize(125, 125));
		target2.fill(Qt::transparent);
		QPainter painter2(&target2);
		painter2.setRenderHint(QPainter::Antialiasing, true);
		painter2.setRenderHint(QPainter::SmoothPixmapTransform, true);
		QPainterPath path2;
		path2.addRoundedRect(0, 0, 125, 125, 125/2, 125/2);
		painter2.setClipPath(path2);
		painter2.drawPixmap(0, 0, profileImage);
		QSharedPointer<QPixmap> roundedProfileImage = QSharedPointer<QPixmap>(new QPixmap(target2));
		QColor userColor(serverMessage[3]);
		//dato che ho successo elimino username e password dalla gui
		openFileBrowser(roundedProfileImage, roundedProfileImageResized, userColor);
	}
	else {
		//dialog per mostrare il messaggio di errore ricevuto dal server
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText(serverMessage[1]);
		resultDialog.exec();
	}
}

void Login::on_newAccount_clicked() {
	resetWindows();
	m_newAccountWindow = new NewAccount(m_socketHandler);
	m_newAccountWindow->show();
	this->newWindow = true;
	connect(m_newAccountWindow, &NewAccount::showParent, this, &Login::childWindowClosed);
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &Login::loginResult);
	this->hide();
}

//funzione per ri-aprire la finestra di login dopo che il filebrowser è stato chiuso
void Login::childWindowClosed() {
	ui.usernameTextLine->setText("");
	ui.passwordTextLine->setText("");
	this->show();
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &Login::loginResult);
}

void Login::on_textChanged(){
	if (this->ui.usernameTextLine->text().compare("") != 0 && this->ui.passwordTextLine->text().compare("") != 0) {
		ui.loginButton->setEnabled(true);
	}
	else {
		ui.loginButton->setEnabled(false);
	}
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