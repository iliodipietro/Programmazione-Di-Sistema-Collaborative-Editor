#pragma once

#include <QMainWindow>
#include <QFileDialog>
#include <QRubberBand>
#include <QProcess>
#include <QSharedPointer>
#include <QTimer>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "ui_NewAccount.h"
#include "SocketHandler/SocketHandler.h"

class NewAccount : public QMainWindow
{
	Q_OBJECT

public:
	NewAccount(QSharedPointer<SocketHandler> socketHandler, QWidget* parent = Q_NULLPTR);
	~NewAccount();

private:
	QSharedPointer<SocketHandler> m_socketHandler;
	QTimer* m_timer;
	Ui::NewAccount ui;
	QPixmap* m_croppedImage;
	QPixmap* m_selectedImage;
	QPixmap* m_resizedImage;
	QRubberBand* m_selectionArea;
	bool move_rubberband;
	QPoint rubberband_offset;
	QPoint myPoint;
	QRect newSelection;
	QSize m_originalSize;



	void closeEvent(QCloseEvent* event);

protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	

private slots:
	void on_selectImageButton_clicked();
	void on_submit_clicked();
	void on_cancel_clicked();
	void registrationResult(QJsonObject);
	void showErrorMessage();
	void dialogClosed(QAbstractButton* button);
	void adjustTextColor();//ilio
	

signals:
	void showParent();
};
