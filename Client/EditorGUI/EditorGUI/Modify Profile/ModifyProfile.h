#pragma once

#include <QWidget>
#include "ui_ModifyProfile.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QRubberBand>
#include <QProcess>
#include <QSharedPointer>
#include <QTimer>
#include <QMessageBox>
#include "SocketHandler/SocketHandler.h"
#include "Serialization/Serialize.h"

class ModifyProfile : public QMainWindow
{
	Q_OBJECT

public:
	ModifyProfile(QSharedPointer<SocketHandler> socketHandler, QString username, QMainWindow* parent = Q_NULLPTR);
	~ModifyProfile();

private:
	Ui::ModifyProfile ui;
	QSharedPointer<SocketHandler> m_socketHandler;
	QTimer* m_timer;
	QPixmap* m_croppedImage;
	QPixmap* m_selectedImage;
	QPixmap* m_resizedImage;
	QRubberBand* m_selectionArea;
	bool move_rubberband;
	QPoint rubberband_offset;
	QPoint myPoint;
	QRect newSelection;
	QSize m_originalSize;
	QString username;

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

signals:
	void showParent();
};
