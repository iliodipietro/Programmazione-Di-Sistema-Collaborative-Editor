#include "FileBrowser.h"

FileBrowser::FileBrowser(QSharedPointer<SocketHandler> socketHandler, QString username, QWidget* parent)
	: QMainWindow(parent), m_socketHandler(socketHandler)
{
	ui.setupUi(this);
	ui.usernameLabel->setText(username);
	this->username = username;
	model.setRootPath(QDir::homePath());
	ui.treeView->setModel(&model);
	ui.treeView->setRootIndex(model.index(QDir::currentPath()));
}

FileBrowser::~FileBrowser()
{
}

void FileBrowser::on_treeView_doubleClicked(const QModelIndex& index) {
	QString path = this->model.filePath(index);
	auto it = m_textEditors.find(path);
	Editor* editor;
	if (it == m_textEditors.end()) {
		editor = new Editor(path, "prova");
		m_textEditors.insert(std::pair<QString, Editor*>(path, editor));
		connect(editor, &Editor::editorClosed, this, &FileBrowser::editorClosed);
		editor->show();
	}
	else {
		editor = it->second;
		editor->raise();
	}
	
}

void FileBrowser::on_newFile_Clicked() {
	auto model = ui.treeView->model();
	QModelIndex parent = model->index(0, 0);
	model->insertRow(0, parent);
	model->setData(model->index(0, 0, parent), QString("Child Item"));
	ui.treeView->setModel(model);
}

void FileBrowser::closeEvent(QCloseEvent* event){
	qApp->quit();
}

void FileBrowser::on_logoutButton_clicked() {
	emit showParent();
	this->hide();
}

void FileBrowser::editorClosed(QString file) {
	Editor* editor = m_textEditors.find(file)->second;
	editor->deleteLater();
	m_textEditors.erase(file);
}

void FileBrowser::mousePressEvent(QMouseEvent* event) {
	show();
}
