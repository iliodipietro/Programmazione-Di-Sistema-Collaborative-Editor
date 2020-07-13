#include "NewAccount.h"
#include <QMouseEvent>

NewAccount::NewAccount(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->selectionArea = Q_NULLPTR;
	this->croppedImage = Q_NULLPTR;
	this->move_rubberband = false;
	this->setAttribute(Qt::WA_DeleteOnClose);
}

NewAccount::~NewAccount()
{
}

void NewAccount::closeEvent(QCloseEvent* event)
{
	exit(0);
}

void NewAccount::on_selectImageButton_clicked() {
	QString url = QFileDialog::getOpenFileName(this, tr("Scegli immagine"), QDir::homePath(), "Immagini (*.jpg *.png *.jpeg)");
	if (url.compare("") != 0) {
		this->img = new QPixmap(url);
		QSize resize(355, 200);
		QSize rubberSize(175, 175);
		QPoint point(ui.imageLabel->pos());
		QRect size(point, rubberSize);
		if (this->selectionArea == Q_NULLPTR) this->selectionArea = new QRubberBand(QRubberBand::Rectangle, this);
		this->selectionArea->setGeometry(size);
		QPoint temp = this->selectionArea->pos();
		this->selectionArea->show();
		ui.imageLabel->setPixmap((*img).scaled(resize));
	}
}

void NewAccount::on_submit_clicked() {
	//mandare le informazioni al serializzatore
	if (this->croppedImage != Q_NULLPTR) {
		delete this->croppedImage;
		this->croppedImage = Q_NULLPTR;
	}

	this->croppedImage = new QPixmap(this->img->copy(this->selectionArea->geometry()));
	ui.crop->setPixmap(*this->croppedImage);
}

void NewAccount::mousePressEvent(QMouseEvent* e)
{
	if (this->selectionArea!=Q_NULLPTR && this->selectionArea->geometry().contains(e->pos()))
	{
		this->rubberband_offset = e->pos() - this->selectionArea->pos();
		this->move_rubberband = true;
	}
}

void NewAccount::mouseMoveEvent(QMouseEvent* e)
{
	if (this->move_rubberband)
	{
		QPoint mousePosition = e->pos();
		QPoint movement = e->pos() - this->rubberband_offset;
		int imagex = ui.imageLabel->pos().x();
		int selectx = this->selectionArea->pos().x() + movement.x();
		if (movement.x() >= ui.imageLabel->pos().x() && movement.y() >= ui.imageLabel->pos().y() &&
			this->selectionArea->geometry().width() + movement.x() <= ui.imageLabel->pos().x() + ui.imageLabel->size().width() &&
			this->selectionArea->geometry().height() + movement.y() <= ui.imageLabel->pos().y() + ui.imageLabel->size().height()) {
			this->selectionArea->move(movement);
		}
		this->ui.imageLabel->update();
	}
}

void NewAccount::mouseReleaseEvent(QMouseEvent* e)
{
	if (this->selectionArea != Q_NULLPTR) {
		this->move_rubberband = false;
		this->newSelection = this->selectionArea->geometry();
	}
}

void NewAccount::on_cancel_clicked() {
	QProcess process;
	process.startDetached("EditorGUI.exe", QStringList());
	qApp->quit();
}