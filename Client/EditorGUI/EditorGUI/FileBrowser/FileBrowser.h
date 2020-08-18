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
	Ui::FileBrowser ui;
	QLineEdit* m_newFileLabel;
	void closeEvent(QCloseEvent* event);

private slots:
	void on_treeView_doubleClicked(const QModelIndex& index);
	void on_logoutButton_clicked();
	void on_newFile_Clicked();
	void editorClosed(QString);

signals:
	void showParent();

protected:
	void mousePressEvent(QMouseEvent* event);
};
