#include "ModifyPassword.h"
#include <QMouseEvent>
#include <QMessageBox>



ModifyPassword::ModifyPassword(QSharedPointer<SocketHandler> socketHandler, QWidget* parent)
	:QDialog(parent), m_socketHandler(socketHandler),
	m_timer(new QTimer(this))
{

	ui.setupUi(this);
	this->move_rubberband = false;
	m_selectionArea = Q_NULLPTR;
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyPassword::changeResult);
	connect(this, &ModifyPassword::dataToSend, m_socketHandler.get(), &SocketHandler::writeData, Qt::QueuedConnection);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(showErrorMessage()));

}


ModifyPassword::~ModifyPassword()
{

}

void ModifyPassword::closeEvent(QCloseEvent* event) {
	this->reject();
}

void ModifyPassword::on_okButton_clicked() {
	QString oldPassword = ui.oldPasswordLineEdit->text();
	QString newPassword = ui.newPasswordLineEdit->text();
	QString confirmPassword = ui.confirmPasswordLineEdit->text();
	

	if (oldPassword != "" && newPassword != "" && confirmPassword != "") {
		if (newPassword.compare(confirmPassword) == 0) {
			QJsonObject passwordSerialize = Serialize::changePasswordSerialize(oldPassword, newPassword, CHANGE_PASSWORD); 
			//bool result = m_socketHandler->writeData(Serialize::fromObjectToArray(passwordSerialize)); //da risultato falso questo perchè?

			/*if (result) {
				m_timer->setSingleShot(true);
				m_timer->setInterval(4000);
				m_timer->start();

			}
			else {
				QMessageBox resultDialog(this);
				resultDialog.setInformativeText("Errore di connessione");
				resultDialog.exec();
			}*/
			emit dataToSend(Serialize::fromObjectToArray(passwordSerialize));
		}
		else {
			QMessageBox::warning(this, "ModifyPassword", "Password non coincidenti!");
		}
	}
	else {
		QMessageBox::warning(this, "ModifyPassword", "I campi non possono essere vuoti");
	}
	
	
	this->accept();
}

void ModifyPassword::on_cancel_clicked() {
	//emit showParent();
	//this->hide();
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyPassword::changeResult);
	this->reject();
}

void ModifyPassword::changeResult(QJsonObject response)
{
	m_timer->stop();
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &ModifyPassword::changeResult);
	QStringList serverMessage = Serialize::responseUnserialize(response);
	bool result = serverMessage[0] == "true" ? true : false;

	if (!result) {
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText(serverMessage[1]);
		resultDialog.exec();
	}
	else {
		//accept? non credo
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText("Password Modificata con Successo");
		resultDialog.exec();
	}
}

void ModifyPassword::mousePressEvent(QMouseEvent* e)
{
	if (m_selectionArea != Q_NULLPTR && m_selectionArea->geometry().contains(e->pos()))
	{
		this->rubberband_offset = e->pos() - m_selectionArea->pos();
		this->move_rubberband = true;
	}
	/*if (ui.imageLabel->underMouse()) {
		myPoint = e->pos();
	}*/
}

//void ModifyPassword::mouseMoveEvent(QMouseEvent* e)
//{
//	if (this->move_rubberband)
//	{
//		QPoint mousePosition = e->pos();
//		QPoint movement = e->pos() - this->rubberband_offset;
//		int movementX = movement.x();
//		int movementY = movement.y();
//		int selectioAreaWidth = m_selectionArea->geometry().width() + movementX;
//		int selectioAreaHeight = m_selectionArea->geometry().height() + movementY;
//		int imageX = ui.imageLabel->pos().x();
//		int imageY = ui.imageLabel->pos().y();
//		int imageWidth = ui.imageLabel->pos().x() + ui.imageLabel->size().width();
//		int imageHeight = ui.imageLabel->pos().y() + ui.imageLabel->size().height();
//
//		if (movementX > imageX && movement.y() > imageY && selectioAreaWidth < imageWidth && selectioAreaHeight < imageHeight) {
//			m_selectionArea->move(movement);
//		}
//		else if (movementX <= imageX && movementY > imageY && selectioAreaHeight < imageHeight) {
//			m_selectionArea->move(imageX, movementY);
//		}
//		else if (movementX > imageX && movementY <= imageY && selectioAreaWidth < imageWidth) {
//			m_selectionArea->move(movementX, imageY);
//		}
//		else if (movement.y() > imageY && selectioAreaWidth >= imageWidth && selectioAreaHeight < imageHeight) {
//			m_selectionArea->move(imageWidth - m_selectionArea->geometry().width(), movementY);
//		}
//		else if (movementX > imageX && selectioAreaWidth < imageWidth && selectioAreaHeight >= imageHeight) {
//			m_selectionArea->move(movementX, imageHeight - m_selectionArea->geometry().height());
//		}
//
//		//this->ui.imageLabel->update();
//	}
//}

void ModifyPassword::mouseReleaseEvent(QMouseEvent* e)
{
	if (m_selectionArea != Q_NULLPTR) {
		this->move_rubberband = false;
		//this->newSelection = m_selectionArea->geometry();
	}
}

void ModifyPassword::showErrorMessage() {
	QMessageBox resultDialog(this);
	resultDialog.setInformativeText("Errore di connessione");
}

void ModifyPassword::dialogClosed(QAbstractButton* button) {
	//emit showParent();
	this->hide();
}
