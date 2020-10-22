#include "CropDialog.h"

#include <QMouseEvent>

#define RUBBER_SIZE 200

CropDialog::CropDialog(QPixmap* image, QWidget* parent) : QDialog(parent), m_image(image)
{
	ui.setupUi(this);

	int labelWidth = ui.imageLabel->width();
	int labelHeight = ui.imageLabel->height();
	m_imageWidth = image->width();
	m_imageHeight = image->height();
	float imageRatio = (float)m_imageWidth / (float)m_imageHeight;

	m_resizedImage = new QPixmap(image->scaled(ui.imageLabel->size(), Qt::KeepAspectRatio));

	m_resizedWidth = m_resizedImage->width();
	m_resizedHeight = m_resizedImage->height();

	QSize rubberSize(RUBBER_SIZE, RUBBER_SIZE);
	QPoint point(ui.imageLabel->pos());
	int offsetX = (ui.imageLabel->width() - m_resizedWidth) / 2;
	int offsetY = (ui.imageLabel->height() - m_resizedHeight) / 2;
	point.setX(point.x() + offsetX);
	point.setY(point.y() + offsetY);
	QRect size(point, rubberSize);

	m_selectionArea = new QRubberBand(QRubberBand::Rectangle, this);
	m_selectionArea->setGeometry(size);
	m_selectionArea->show();

	ui.imageLabel->setPixmap(*m_resizedImage);
}

void CropDialog::mousePressEvent(QMouseEvent* e)
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

void CropDialog::mouseMoveEvent(QMouseEvent* e)
{
	if (this->move_rubberband)
	{
		QPoint mousePosition = e->pos();
		QPoint movement = e->pos() - this->rubberband_offset;
		int movementX = movement.x();
		int movementY = movement.y();
		int selectioAreaWidth = m_selectionArea->geometry().width() + movementX;
		int selectioAreaHeight = m_selectionArea->geometry().height() + movementY;
		int offsetX = (ui.imageLabel->width() - m_resizedWidth) / 2;
		int offsetY = (ui.imageLabel->height() - m_resizedHeight) / 2;
		int imageX = ui.imageLabel->pos().x() + offsetX;
		int imageY = ui.imageLabel->pos().y() + offsetY;
		int imageWidth = ui.imageLabel->pos().x() + ui.imageLabel->width() - offsetX;
		int imageHeight = ui.imageLabel->pos().y() + ui.imageLabel->height() - offsetY;


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

void CropDialog::mouseReleaseEvent(QMouseEvent* e)
{
	if (m_selectionArea != Q_NULLPTR) {
		this->move_rubberband = false;
		this->newSelection = m_selectionArea->geometry();
	}
}

void CropDialog::on_okButton_clicked() {
	int imageLeft = ui.imageLabel->pos().x() + ((ui.imageLabel->width() - m_resizedWidth) / 2);
	int imageTop = ui.imageLabel->pos().y() + ((ui.imageLabel->height() - m_resizedHeight) / 2);

	QPoint areaPos = m_selectionArea->geometry().topLeft();
	int actualLeft = ((areaPos.x() - imageLeft) * m_imageWidth) / (m_resizedWidth);
	int actualTop = ((areaPos.y() - imageTop) * m_imageHeight) / (m_resizedHeight);
	int actualRight = (RUBBER_SIZE * m_imageWidth) / m_resizedWidth;
	int actualBottom = (RUBBER_SIZE * m_imageHeight) / m_resizedHeight;

	m_croppedImage = new QPixmap(m_image->copy(actualLeft, actualTop, actualRight, actualBottom));
	this->accept();
}

void CropDialog::on_cancelButton_clicked() {
	this->reject();
}

CropDialog::~CropDialog()
{
}
