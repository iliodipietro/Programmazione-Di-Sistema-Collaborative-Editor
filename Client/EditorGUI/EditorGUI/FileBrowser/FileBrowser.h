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
	FileBrowser(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<QPixmap> profileImage, QColor userColor, QString username = "", int clientID = 0,
		QWidget* parent = Q_NULLPTR);
	~FileBrowser();

private:
	QSharedPointer<SocketHandler> m_socketHandler;
	std::map<int, Editor*> m_textEditors;
	QFileSystemModel model;
	QString username;
	int clientID;
	ModifyProfile* m_modifyProfile;
	Ui::FileBrowser ui;
	QLineEdit* m_newFileLabel;
	QSharedPointer<QPixmap> m_profileImage;
	QMap<QString, int> filename_id;
	QColor m_userColor;

	void closeEvent(QCloseEvent* event);
	void removeBlank();
	void requestFiles();

private slots:
	void on_fileList_itemDoubleClicked(QListWidgetItem* item);
	void on_logoutButton_clicked();
	void on_modifyProfile_clicked();
	void on_newFile_Clicked();
	void on_deleteFile_Clicked();
	void editorClosed(int);
	void on_renameFile_Clicked();
	void childWindowClosed();
	void addFiles(QJsonObject message);
	void addFile(QJsonObject message);
	void handleNewMessage(QJsonObject message);
	void processEditorMessage(QJsonObject message);

signals:
	void showParent();

protected:
	void mousePressEvent(QMouseEvent* event);
};
