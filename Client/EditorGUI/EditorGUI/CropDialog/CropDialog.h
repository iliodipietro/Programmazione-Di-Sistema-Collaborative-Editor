#pragma once

#include <QDialog>
#include <QPixmap>
#include <QRubberBand>
#include "ui_CropDialog.h"

class CropDialog : public QDialog
{
	Q_OBJECT

public:
	CropDialog(QPixmap* image, QWidget *parent = Q_NULLPTR);

	inline QPixmap* getCroppedImage() { return m_croppedImage; };

	~CropDialog();

private:
	Ui::CropDialog ui;
	QRubberBand* m_selectionArea;
	QPixmap* m_image;
	QPixmap* m_resizedImage;
	QPixmap* m_croppedImage;
	bool move_rubberband;
	QPoint rubberband_offset;
	QPoint myPoint;
	QRect newSelection;
	int m_imageWidth;
	int m_imageHeight;
	int m_resizedWidth;
	int m_resizedHeight;

protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);


private slots:
	void on_okButton_clicked();
	void on_cancelButton_clicked();
};
