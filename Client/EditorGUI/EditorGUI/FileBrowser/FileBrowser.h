#pragma once

#include <QMainWindow>
#include <QDesktopWidget>
#include <QApplication>
#include <QProcess>
#include <QFileSystemModel>
#include <QTreeView>
#include <QCloseEvent>
#include "Editor/Editor.h"
#include "ui_FileBrowser.h"

class FileBrowser : public QMainWindow
{
	Q_OBJECT

public:
	FileBrowser(QWidget *parent = Q_NULLPTR, QString username = "");
	~FileBrowser();

private:
	Editor* textEditor;
	QFileSystemModel model;
	QString username;
	Ui::FileBrowser ui;
	void closeEvent(QCloseEvent* event);

private slots:
	void on_treeView_doubleClicked(const QModelIndex& index);
	void on_logoutButton_clicked();
};
