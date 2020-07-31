#include "NewAccount.h"
#include <QMouseEvent>
#include <QMessageBox>

NewAccount::NewAccount(QSharedPointer<SocketHandler> socketHandler, QWidget* parent)
	: QMainWindow(parent), m_socketHandler(socketHandler),
	m_messageSerializer(QSharedPointer<Serialize>(new Serialize(this))),
	m_timer(new QTimer(this))
{
	ui.setupUi(this);
	this->move_rubberband = false;
	m_selectionArea = Q_NULLPTR;
	m_croppedImage = Q_NULLPTR;
	m_originalSize = ui.imageLabel->size();
	this->setAttribute(Qt::WA_DeleteOnClose);
	connect(m_socketHandler.get(), SIGNAL(SocketHandler::dataReceived(QJsonObject)), this, SLOT(registrationResult(QJsonObject)));
	connect(m_timer, SIGNAL(timeout()), this, SLOT(showErrorMessage()));
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
		m_resizedImage = new QPixmap(m_selectedImage->scaled(ui.imageLabel->size(), Qt::KeepAspectRatio));
		QSize rubberSize(50, 50);
		QPoint point(ui.imageLabel->pos());
		QRect size(point, rubberSize);
		if (m_selectionArea == Q_NULLPTR)
			m_selectionArea = new QRubberBand(QRubberBand::Rectangle, this);
		m_selectionArea->setGeometry(size);
		m_selectionArea->show();
		ui.imageLabel->setPixmap(*m_resizedImage);
		ui.imageLabel->setFixedWidth(m_resizedImage->width());
		ui.imageLabel->setFixedHeight(m_resizedImage->height());
	}
}

void NewAccount::on_submit_clicked() {
	//mandare le informazioni al serializzatore
	if (m_croppedImage != Q_NULLPTR) {
		delete m_croppedImage;
		m_croppedImage = Q_NULLPTR;
	}

	QString username = ui.nickNameLine->text();
	QString password = ui.passwordLine->text();
	QString email = ui.emailLine->text();
	QPoint areaPos = m_selectionArea->geometry().topLeft();
	areaPos.setX(areaPos.x() - ui.imageLabel->pos().x());
	areaPos.setY(areaPos.y() - ui.imageLabel->pos().y());
	m_croppedImage = new QPixmap(m_resizedImage->copy(areaPos.x(), areaPos.y(), 50, 50));
	ui.crop->setPixmap(*m_croppedImage);
	QString imageSerialized = m_messageSerializer->imageSerialize(*m_croppedImage, 2);
	QString userInfoSerialized = m_messageSerializer->userSerialize(username, password, username, 2);
	bool result1 = m_socketHandler->writeData(imageSerialized);
	bool result2 = m_socketHandler->writeData(userInfoSerialized);
	if (result1 && result2) {
		m_timer->setSingleShot(true);
		m_timer->setInterval(1000);
		m_timer->start();
	}
	else {
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText("Errore di connessione");
		resultDialog.exec();
	}
}
	try {
	this->croppedImage = new QPixmap(this->img->copy(this->selectionArea->geometry()));
	ui.crop->setPixmap(*this->croppedImage);
	}
	catch(std::exception e){
		QMessageBox::warning(this, "NewAccount", "A picture is needed");
		//mettere qui di default la prima lettera del nickname
	}

	
	if (password.compare(password_re) == 0) {

		//emit per il socket
		QMessageBox::information(this, "NewAccount", "New Account Created");
	
	}
	else {
		QMessageBox::warning(this, "NewAccount", "The password is incorrect!");
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
	emit showParent();
	this->hide();
}

void NewAccount::registrationResult(QJsonObject response) {
	int result = m_messageSerializer->responseUnserialize(response);
	if (true) {
		QMessageBox resultDialog(this);
		connect(&resultDialog, &QMessageBox::buttonClicked, this, &NewAccount::dialogClosed);
		resultDialog.setInformativeText("Success");
		resultDialog.exec();
	}
	else {
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText(""); //mettere il messaggio di errore contenuto nel Json di risposta
		resultDialog.exec();
	}
}

void NewAccount::showErrorMessage() {
	QMessageBox resultDialog(this);
	resultDialog.setInformativeText("Errore di connessione");
}

void NewAccount::dialogClosed(QAbstractButton* button) {
	emit showParent();
	this->hide();
}