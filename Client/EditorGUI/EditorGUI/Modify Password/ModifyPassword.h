//#pragma once
//
//#include <QWidget>
//#include "ui_ModifyPassword.h"
//
//class ModifyPassword : public QWidget
//{
//	Q_OBJECT
//
//public:
//	ModifyPassword(QWidget *parent = Q_NULLPTR);
//	~ModifyPassword();
//
//private:
//	Ui::ModifyPassword ui;
//};


#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QFileDialog>
#include <QRubberBand>
#include <QProcess>
#include <QSharedPointer>
#include <QTimer>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "ui_ModifyPassword.h"
#include "SocketHandler/SocketHandler.h"

class ModifyPassword : public QDialog
{
	Q_OBJECT

public:
	ModifyPassword(QSharedPointer<SocketHandler> socketHandler, QWidget* parent = Q_NULLPTR);
	~ModifyPassword();

private:
	QSharedPointer<SocketHandler> m_socketHandler;
	QTimer* m_timer;
	Ui::ModifyPassword ui;
	QRubberBand* m_selectionArea;
	bool move_rubberband;
	QPoint rubberband_offset;
	QPoint myPoint;
	QRect newSelection;
	QSize m_originalSize;

	void closeEvent(QCloseEvent* event);

protected:
	void mousePressEvent(QMouseEvent* e);
	//void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);

private slots:
	void on_okButton_clicked();
	void on_cancel_clicked();
	void changeResult(QJsonObject);
	void showErrorMessage();
	void dialogClosed(QAbstractButton* button);

};