#pragma once

#include <QMainWindow>
#include <QDesktopWidget>
#include <QApplication>
#include <QProcess>
#include <QFileSystemModel>
#include <QTreeView>
#include <QCloseEvent>
#include "Editor/Editor.h"
#include "SocketHandler/SocketHandler.h"
#include "Serialization/Serialize.h"
#include "ModifyProfile.h"
#include "ui_FileBrowser.h"

class FileBrowser : public QMainWindow
{
	Q_OBJECT

public:
	FileBrowser(QSharedPointer<SocketHandler> socketHandler, QString username = "", QWidget* parent = Q_NULLPTR);
	~FileBrowser();

private:
	QSharedPointer<SocketHandler> m_socketHandler;
	std::map<QString, Editor*> m_textEditors;
	QFileSystemModel model;
	QString username;

	ModifyProfile* m_modifyProfile;
	Ui::FileBrowser ui;
	void closeEvent(QCloseEvent* event);

private slots:
	void on_treeView_doubleClicked(const QModelIndex& index);
	void on_logoutButton_clicked();
	void on_modifyProfile_clicked();
	void editorClosed(QString);
	void childWindowClosed();

signals:
	void showParent();

protected:
	void mousePressEvent(QMouseEvent* event);
};
