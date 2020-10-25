#pragma once

#include <QWidget>
#include <QDialog>
#include "ui_ModifyProfile.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QRubberBand>
#include <QProcess>
#include <QSharedPointer>
#include <QTimer>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPainter>
#include "SocketHandler/SocketHandler.h"
#include "Serialization/Serialize.h"
#include "Modify Password/ModifyPassword.h"


class ModifyProfile : public QMainWindow
{
	Q_OBJECT

public:
	ModifyProfile(QSharedPointer<SocketHandler> socketHandler, QString username, QString email, QSharedPointer<QPixmap> profileImage, QMainWindow* parent = Q_NULLPTR);
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
	int clientID;
	QString m_username;
	QString m_email;
	QSharedPointer<QPixmap> m_image;

	void closeEvent(QCloseEvent* event);

protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);

private slots:
	void on_selectImageButton_clicked();
	void on_modifyPasswordButton_clicked();
	void on_submit_clicked();
	void on_cancel_clicked();
	void ModifyProfileResult(QJsonObject);
	void showErrorMessage();
	void dialogClosed(QAbstractButton* button);
	void adjustTextColor(); //ilio
	void on_texte_changed(QString newText);
	void on_image_changed(QSharedPointer<QPixmap> newImage);

signals:
	void showParent();
	void showParentUpdated(QString m_username, QString m_email, QSharedPointer<QPixmap> m_image);
	void dataToSend(QByteArray);
	void imageChanged(QSharedPointer<QPixmap> m_image);
};
