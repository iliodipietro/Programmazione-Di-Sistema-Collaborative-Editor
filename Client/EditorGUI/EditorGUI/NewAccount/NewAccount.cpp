#include "NewAccount.h"
#include "CropDialog/CropDialog.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>

#define RUBBER_SIZE 125
NewAccount::NewAccount(QSharedPointer<SocketHandler> socketHandler, QWidget* parent)
	: QMainWindow(parent), m_socketHandler(socketHandler),
	m_timer(new QTimer(this))
{
	ui.setupUi(this);
	this->move_rubberband = false;
	m_selectionArea = Q_NULLPTR;
	m_croppedImage = Q_NULLPTR;
	m_originalSize = ui.imageLabel->size();
	this->setAttribute(Qt::WA_DeleteOnClose);
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &NewAccount::registrationResult);
	connect(this, &NewAccount::dataToSend, m_socketHandler.get(), &SocketHandler::writeData, Qt::QueuedConnection);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(showErrorMessage()));
	//ilio
	QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);
	ui.emailLine->setValidator(new QRegularExpressionValidator(rx, this));
	connect(ui.emailLine, &QLineEdit::textChanged, this, &NewAccount::adjustTextColor);

	QString url = QDir::currentPath().append("/").append("user.png");
	m_selectedImage = new QPixmap(url);
	m_croppedImage = new QPixmap(m_selectedImage->scaled(QSize(RUBBER_SIZE, RUBBER_SIZE), Qt::KeepAspectRatio));
	QPixmap target(QSize(RUBBER_SIZE , RUBBER_SIZE ));
	target.fill(Qt::transparent);
	QPainter painter(&target);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	QPainterPath path;
	path.addRoundedRect(0, 0, RUBBER_SIZE , RUBBER_SIZE, RUBBER_SIZE / 2, RUBBER_SIZE / 2);
	painter.setClipPath(path);
	painter.drawPixmap(0, 0, *m_croppedImage);
	QPixmap roundedImage(target);
	ui.imageLabel->setPixmap(target);

	//Ilio
	ui.submit->setEnabled(false);
	connect(ui.usernameLine, &QLineEdit::textChanged, this, &NewAccount::on_texte_changed);
	connect(ui.emailLine, &QLineEdit::textChanged, this, &NewAccount::on_texte_changed);
	connect(ui.passwordLine, &QLineEdit::textChanged, this, &NewAccount::on_texte_changed);
	connect(ui.rePasswordLine, &QLineEdit::textChanged, this, &NewAccount::on_texte_changed);
}

NewAccount::~NewAccount()
{
}

void NewAccount::closeEvent(QCloseEvent* event)
{
	exit(0);
}

void NewAccount::on_selectImageButton_clicked() {
	ui.imageLabel->setFixedWidth(m_originalSize.width());
	ui.imageLabel->setFixedHeight(m_originalSize.height());
	QString url = QFileDialog::getOpenFileName(this, tr("Scegli immagine"), QDir::homePath(), "Immagini (*.jpg *.png *.jpeg)");
	if (url.compare("") != 0) {
		m_selectedImage = new QPixmap(url);
		CropDialog* dialog = new CropDialog(m_selectedImage, this);
		dialog->setModal(true);
		if (dialog->exec() == QDialog::Accepted) {
			m_croppedImage = dialog->getCroppedImage();
			QPixmap* resizedImage = new QPixmap(m_croppedImage->scaled(ui.imageLabel->size(), Qt::KeepAspectRatio));
			QPixmap target(QSize(RUBBER_SIZE, RUBBER_SIZE));
			target.fill(Qt::transparent);
			QPainter painter(&target);
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
			QPainterPath path;
			path.addRoundedRect(0, 0, RUBBER_SIZE, RUBBER_SIZE, RUBBER_SIZE /2, RUBBER_SIZE /2);
			painter.setClipPath(path);
			painter.drawPixmap(0, 0, *resizedImage);
			QPixmap roundedImage(target);
			ui.imageLabel->setPixmap(target);
		}
	}
}

void NewAccount::on_submit_clicked() {
	//mandare le informazioni al serializzatore

	QString username = ui.usernameLine->text();
	QString password = ui.passwordLine->text();
	QString password_re = ui.rePasswordLine->text();
	QString email = ui.emailLine->text();
	if (username != "" && email != "") {
		if (password.compare(password_re) == 0) {
			QJsonObject userInfoSerialized = Serialize::userSerialize(username, password, email, REGISTER, m_croppedImage); //ilio
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
			//if (m_croppedImage != Q_NULLPTR) {
			//	QPoint areaPos = m_selectionArea->geometry().topLeft();
			//	areaPos.setX(areaPos.x() - ui.imageLabel->pos().x());
			//	areaPos.setY(areaPos.y() - ui.imageLabel->pos().y());
			//	m_croppedImage = new QPixmap(m_resizedImage->copy(areaPos.x(), areaPos.y(), RUBBER_SIZE, RUBBER_SIZE));
			//	//ui.crop->setPixmap(*m_croppedImage);
			//	

			//}
			//ilio-------------------------------- immagine di default 
			//else {
			//	//ui.imageLabel->setFixedWidth(m_originalSize.width());
			//	//ui.imageLabel->setFixedHeight(m_originalSize.height());
			//	QString url = QDir::currentPath().append("/").append("user.png");
			//	m_selectedImage = new QPixmap(url);
			//	m_resizedImage = new QPixmap(m_selectedImage->scaled(ui.imageLabel->size(), Qt::KeepAspectRatio));
			//	
			//	QSize rubberSize(RUBBER_SIZE, RUBBER_SIZE);
			//	QPoint point(ui.imageLabel->pos());
			//	QRect size(point, rubberSize);
			//	if (m_selectionArea == Q_NULLPTR)
			//		m_selectionArea = new QRubberBand(QRubberBand::Rectangle, this);
			//	m_selectionArea->setGeometry(size);
			//	ui.imageLabel->setPixmap(*m_resizedImage);
			//	ui.imageLabel->setFixedWidth(m_resizedImage->width());
			//	ui.imageLabel->setFixedHeight(m_resizedImage->height());


			//	QPoint areaPos = m_selectionArea->geometry().topLeft();
			//	areaPos.setX(areaPos.x() - ui.imageLabel->pos().x());
			//	areaPos.setY(areaPos.y() - ui.imageLabel->pos().y());
			//	m_croppedImage = new QPixmap(m_resizedImage->copy(areaPos.x(), areaPos.y(), RUBBER_SIZE, RUBBER_SIZE));
			//	//ui.crop->setPixmap(*m_croppedImage);

			//}
			//ilio-----------------------------------

			//if (m_croppedImage != Q_NULLPTR) { ilio
				
				
				//QMessageBox::information(this, "NewAccount", "New Account Created");

			//} ilio
			//else {
				//QMessageBox::warning(this, "NewAccount", "A picture is needed");
			//}
		}
		else {
			QMessageBox::warning(this, "NewAccount", "The password is incorrect!");
		}
	}
	else {
		QMessageBox::warning(this, "NewAccount", "Username e/o email mancanti");
	}

}

void NewAccount::mousePressEvent(QMouseEvent* e)
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

void NewAccount::mouseMoveEvent(QMouseEvent* e)
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

void NewAccount::mouseReleaseEvent(QMouseEvent* e)
{
	if (m_selectionArea != Q_NULLPTR) {
		this->move_rubberband = false;
		this->newSelection = m_selectionArea->geometry();
	}
}

void NewAccount::on_cancel_clicked() {
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &NewAccount::registrationResult);
	emit showParent();
	this->hide();
}

void NewAccount::registrationResult(QJsonObject response) {
	m_timer->stop();
	QStringList serverMessage = Serialize::responseUnserialize(response);
	bool result = serverMessage[0] == "true" ? true : false;
	if (result) {
		QMessageBox resultDialog(this);
		connect(&resultDialog, &QMessageBox::buttonClicked, this, &NewAccount::dialogClosed);
		resultDialog.setInformativeText("Registazione effettuata con successo");
		resultDialog.exec();
	}
	else {
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText(serverMessage[1]); //mettere il messaggio di errore contenuto nel Json di risposta
		resultDialog.exec();
	}
}

void NewAccount::showErrorMessage() {
	QMessageBox resultDialog(this);
	resultDialog.setInformativeText("Errore di connessione");
}

void NewAccount::dialogClosed(QAbstractButton* button) {
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &NewAccount::registrationResult);
	emit showParent();
	this->hide();
}

void NewAccount::adjustTextColor() {
	if (!ui.emailLine->hasAcceptableInput())
		ui.emailLine->setStyleSheet("QLineEdit { color: red;}");
	else
		ui.emailLine->setStyleSheet("QLineEdit { color: black;}");
}

void NewAccount::on_texte_changed(){
	if (this->ui.usernameLine->text().compare("")   != 0 &&
		this->ui.emailLine->text().compare("")      != 0 &&
		this->ui.passwordLine->text().compare("")   != 0 &&
		this->ui.rePasswordLine->text().compare("") != 0) {

		ui.submit->setEnabled(true);
	}
	else {
		ui.submit->setEnabled(false);
	}
}
