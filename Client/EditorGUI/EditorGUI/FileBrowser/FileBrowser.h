#pragma once

#include <QMainWindow>
#include <QDesktopWidget>
#include <QApplication>
#include <QProcess>
#include <QFileSystemModel>
#include <QTreeView>
#include <QCloseEvent>
#include <QLineEdit>
#include "Editor/Editor.h"
#include "SocketHandler/SocketHandler.h"
#include "Serialization/Serialize.h"
#include "ModifyProfile.h"
#include "ui_FileBrowser.h"

class FileBrowser : public QMainWindow
{
	Q_OBJECT

public:
	FileBrowser(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<QPixmap> profileImage, QString username = "", QWidget* parent = Q_NULLPTR);
	~FileBrowser();

private:
	QSharedPointer<SocketHandler> m_socketHandler;
	std::map<QString, Editor*> m_textEditors;
	QFileSystemModel model;
	QString username;

	ModifyProfile* m_modifyProfile;
	Ui::FileBrowser ui;
	QLineEdit* m_newFileLabel;
	QSharedPointer<QPixmap> m_profileImage;
	void closeEvent(QCloseEvent* event);

	void requestFiles();

private slots:
	void on_fileList_itemDoubleClicked(QListWidgetItem* item);
	void on_logoutButton_clicked();
	void on_modifyProfile_clicked();
	void on_newFile_Clicked();
	void editorClosed(QString);
	void childWindowClosed();
	void addFiles(QJsonObject message);

signals:
	void showParent();

protected:
	void mousePressEvent(QMouseEvent* event);
};
