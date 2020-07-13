#pragma once

#include <QMainWindow>
#include <QFileDialog>
#include <QRubberBand>
#include <QProcess>
#include "ui_NewAccount.h"

class NewAccount : public QMainWindow
{
	Q_OBJECT

public:
	NewAccount(QWidget *parent = Q_NULLPTR);
	~NewAccount();

private:
	Ui::NewAccount ui;
	QPixmap* croppedImage;
	QPixmap* img;
	QRubberBand* selectionArea;
	bool move_rubberband;
	QPoint rubberband_offset;
	QRect newSelection;
	void closeEvent(QCloseEvent* event);

protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);

private slots:
	void on_selectImageButton_clicked();
	void on_submit_clicked();
	void on_cancel_clicked();
};
