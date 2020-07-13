#include "FileBrowser.h"

FileBrowser::FileBrowser(QWidget *parent, QString username)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.usernameLabel->setText(username);
	this->username = username;
	this->textEditor = Q_NULLPTR;
	model.setRootPath(QDir::homePath());
	ui.treeView->setModel(&model);
	ui.treeView->setRootIndex(model.index(QDir::currentPath()));
	this->setAttribute(Qt::WA_DeleteOnClose);
}

FileBrowser::~FileBrowser()
{
	
}

void FileBrowser::on_treeView_doubleClicked(const QModelIndex& index) {
	/*if (this->textEditor != Q_NULLPTR) {
		delete this->textEditor;
		this->textEditor = Q_NULLPTR;
	}*/
	
	QString path = this->model.filePath(index);

	this->textEditor = new Editor(this, path);
	textEditor->show();
	this->hide();
}

void FileBrowser::closeEvent(QCloseEvent* event)
{
	qApp->quit();
}

void FileBrowser::on_logoutButton_clicked() {
	//aggiungere 
	QProcess process;
	process.startDetached("EditorGUI.exe", QStringList());
	qApp->quit();
}
