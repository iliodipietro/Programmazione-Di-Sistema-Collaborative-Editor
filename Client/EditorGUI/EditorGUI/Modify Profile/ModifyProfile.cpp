#include "ModifyProfile.h"
#include "CropDialog/CropDialog.h"
#include <QMouseEvent>
#include <QMessageBox>
#define RUBBER_SIZE 125

ModifyProfile::ModifyProfile(QSharedPointer<SocketHandler> socketHandler, QString username, QString email, QSharedPointer<QPixmap> profileImage, QMainWindow* parent) : QMainWindow(parent), m_socketHandler(socketHandler),
m_timer(new QTimer(this)) {


	ui.setupUi(this);
	this->move_rubberband = false;
	m_selectionArea = Q_NULLPTR;
	//m_croppedImage = Q_NULLPTR;
	m_resizedImage = Q_NULLPTR;
	m_selectedImage = Q_NULLPTR;
	this->m_username = username;
	this->m_email = email;
	m_croppedImage = profileImage.get();
	qDebug() << username;  
	
	ui.usernameLine_3->setText(username);
	ui.emailLine_3->setText(email);
	QPixmap scaledImage = profileImage->scaled(QSize(RUBBER_SIZE, RUBBER_SIZE), Qt::KeepAspectRatio);
	ui.imageLabel->setPixmap(scaledImage);
	

	m_originalSize = ui.imageLabel->size();
	this->setAttribute(Qt::WA_DeleteOnClose);
	//connect(m_socketHandler.get(), SIGNAL(SocketHandler::dataReceived(QJsonObject)), this, SLOT(ModifyProfileResult(QJsonObject)));
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyProfile::ModifyProfileResult);
	connect(this, &ModifyProfile::dataToSend, m_socketHandler.get(), &SocketHandler::writeData, Qt::QueuedConnection);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(showErrorMessage()));
	//ilio
	ui.submit->setEnabled(false);
	QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);
	ui.emailLine_3->setValidator(new QRegularExpressionValidator(rx, this));
	connect(ui.emailLine_3, &QLineEdit::textChanged, this, &ModifyProfile::adjustTextColor);
	connect(ui.emailLine_3, &QLineEdit::textChanged, this, &ModifyProfile::on_texte_changed);
	connect(ui.usernameLine_3, &QLineEdit::textChanged, this, &ModifyProfile::on_texte_changed);
}

ModifyProfile::~ModifyProfile()
{
}



void ModifyProfile::closeEvent(QCloseEvent* event)
{
	exit(0);
}

void ModifyProfile::on_selectImageButton_clicked() {
	ui.imageLabel->setFixedWidth(m_originalSize.width());
	ui.imageLabel->setFixedHeight(m_originalSize.height());
	QString url = QFileDialog::getOpenFileName(this, tr("Scegli immagine"), QDir::homePath(), "Immagini (*.jpg *.png *.jpeg)");
	if (url.compare("") != 0) {
		m_selectedImage = new QPixmap(url);
		CropDialog* dialog = new CropDialog(m_selectedImage, this);
		dialog->setModal(true);
		if (dialog->exec() == QDialog::Accepted) {
			m_croppedImage = dialog->getCroppedImage();
			QPixmap* resizedImage = new QPixmap(m_croppedImage->scaled(QSize(RUBBER_SIZE, RUBBER_SIZE), Qt::KeepAspectRatio));
			QPixmap target(QSize(RUBBER_SIZE , RUBBER_SIZE));
			target.fill(Qt::transparent);
			QPainter painter(&target);
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
			QPainterPath path;
			path.addRoundedRect(0, 0, RUBBER_SIZE , RUBBER_SIZE , RUBBER_SIZE /2, RUBBER_SIZE /2);
			painter.setClipPath(path);
			painter.drawPixmap(0, 0, *resizedImage);
			QPixmap roundedImage(target);
			ui.imageLabel->setPixmap(target);
			ui.submit->setEnabled(true);
		}
	}
	else {
		ui.submit->setEnabled(false);
	}
}

void ModifyProfile::on_modifyPasswordButton_clicked()
{
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyProfile::ModifyProfileResult);
	//aprire il dialog modifica password
	ModifyPassword* m_modifyPassword = new ModifyPassword(m_socketHandler, this);
	if (m_modifyPassword->exec() == QDialog::Accepted) {

	}
	else {

	}
	//disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyProfile::ModifyProfileResult);
}

void ModifyProfile::on_submit_clicked() {
	//mandare le informazioni al serializzatore
	//if (m_croppedImage != Q_NULLPTR) {
		//delete m_croppedImage;
		//m_croppedImage = Q_NULLPTR;
		//m_croppedImage = 
	//}

	QString newEmail = ui.emailLine_3->text();
	QString newUser = ui.usernameLine_3->text();

	if (newEmail != "") {
		if (newUser != "") {
			QJsonObject userInfoSerialized = Serialize::changeProfileSerialize(this->m_username, newUser, this->m_email, newEmail, m_croppedImage, CHANGE_PROFILE);//type da definire in define.h  devo usare changeProfileSerialize
			//bool result = m_socketHandler->writeData(Serialize::fromObjectToArray(userInfoSerialized));
			//if (result) {
			//	m_timer->setSingleShot(true);
			//	m_timer->setInterval(4000);
			//	m_timer->start();
			//}
			//else {
			//	QMessageBox resultDialog(this);
			//	resultDialog.setInformativeText("Errore di connessione");
			//	resultDialog.exec();
			//}
			emit dataToSend(Serialize::fromObjectToArray(userInfoSerialized));
			//QMessageBox::information(this, "NewAccount", "New Account Created");
		}
		else {
			QMessageBox::warning(this, "Modifica Profilo", "Missing username!");
		}
	}
	else {
		QMessageBox::warning(this, "Modifica Profilo", "email mancante");
	}
	//disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyProfile::ModifyProfileResult); questa disconnect non fa chiudere la finestra
}

void ModifyProfile::mousePressEvent(QMouseEvent* e)
{
	if (m_selectionArea != Q_NULLPTR && m_selectionArea->geometry().contains(e->pos()))
	{
		this->rubberband_offset = e->pos() - m_selectionArea->pos();
		this->move_rubberband = true;
	}
	if (ui.imageLabel->underMouse()) {
		myPoint = e->pos();
	}
}

void ModifyProfile::mouseMoveEvent(QMouseEvent* e)
{
	if (this->move_rubberband)
	{
		QPoint mousePosition = e->pos();
		QPoint movement = e->pos() - this->rubberband_offset;
		int movementX = movement.x();
		int movementY = movement.y();
		int selectioAreaWidth = m_selectionArea->geometry().width() + movementX;
		int selectioAreaHeight = m_selectionArea->geometry().height() + movementY;
		int imageX = ui.imageLabel->pos().x();
		int imageY = ui.imageLabel->pos().y();
		int imageWidth = ui.imageLabel->pos().x() + ui.imageLabel->size().width();
		int imageHeight = ui.imageLabel->pos().y() + ui.imageLabel->size().height();

		if (movementX > imageX && movement.y() > imageY && selectioAreaWidth < imageWidth && selectioAreaHeight < imageHeight) {
			m_selectionArea->move(movement);
		}
		else if (movementX <= imageX && movementY > imageY && selectioAreaHeight < imageHeight) {
			m_selectionArea->move(imageX, movementY);
		}
		else if (movementX > imageX && movementY <= imageY && selectioAreaWidth < imageWidth) {
			m_selectionArea->move(movementX, imageY);
		}
		else if (movement.y() > imageY && selectioAreaWidth >= imageWidth && selectioAreaHeight < imageHeight) {
			m_selectionArea->move(imageWidth - m_selectionArea->geometry().width(), movementY);
		}
		else if (movementX > imageX && selectioAreaWidth < imageWidth && selectioAreaHeight >= imageHeight) {
			m_selectionArea->move(movementX, imageHeight - m_selectionArea->geometry().height());
		}

		this->ui.imageLabel->update();
	}
}

void ModifyProfile::mouseReleaseEvent(QMouseEvent* e)
{
	if (m_selectionArea != Q_NULLPTR) {
		this->move_rubberband = false;
		this->newSelection = m_selectionArea->geometry();
	}
}

void ModifyProfile::on_cancel_clicked() {
	emit showParent();
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyProfile::ModifyProfileResult);
	this->hide();
}

void ModifyProfile::ModifyProfileResult(QJsonObject response) {
	QStringList serverMessage = Serialize::changeProfileResponseUnserialize(response);
	bool result = serverMessage[0] == "true" ? true : false;

	if (result) {

		this->m_email = serverMessage[2];
		this->m_username = serverMessage[1];
		QString profileImageBase64 = serverMessage[3];
		QPixmap profileImage;
		profileImage.loadFromData(QByteArray::fromBase64(profileImageBase64.toLatin1()));
		QPixmap resizedProfileImage = profileImage.scaled(QSize(60, 60), Qt::KeepAspectRatio);
		QPixmap target(QSize(60, 60));
		target.fill(Qt::transparent);
		QPainter painter(&target);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
		QPainterPath path;
		path.addRoundedRect(0, 0, 60, 60, 30, 30);
		painter.setClipPath(path);
		painter.drawPixmap(0, 0, resizedProfileImage);
		this->m_image = QSharedPointer<QPixmap>(new QPixmap(target));
		QColor userColor(serverMessage[3]);

		QMessageBox resultDialog(this);
		connect(&resultDialog, &QMessageBox::buttonClicked, this, &ModifyProfile::dialogClosed);
		resultDialog.setInformativeText("Success");
		resultDialog.exec();

	}
	else {
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText(serverMessage[4]); //mettere il messaggio di errore contenuto nel Json di risposta
		resultDialog.exec();
	}
}

void ModifyProfile::showErrorMessage() {
	QMessageBox resultDialog(this);
	resultDialog.setInformativeText("Errore di connessione");
}

void ModifyProfile::dialogClosed(QAbstractButton* button) {
	emit showParentUpdated(this->m_username, this->m_email, this->m_image);
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyProfile::ModifyProfileResult);
	this->hide();
}

void ModifyProfile::adjustTextColor() {
	if (!ui.emailLine_3->hasAcceptableInput())
		ui.emailLine_3->setStyleSheet("QLineEdit { color: red;}");
	else
		ui.emailLine_3->setStyleSheet("QLineEdit { color: black;}");
}

void ModifyProfile::on_texte_changed(QString newText){// il newText puo essere la nuova email o il nuovo username, quindi controllo che sia differente da entrambi i campi di testo 
	if (newText.compare(this->m_email) != 0 && newText.compare(this->m_username) != 0 ) {
		ui.submit->setEnabled(true);
	}
	else {
		ui.submit->setEnabled(false);
	}
}



