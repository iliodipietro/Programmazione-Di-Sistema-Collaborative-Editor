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

void FileBrowser::closeEvent(QCloseEvent* event){
	qApp->quit();
}

void FileBrowser::on_logoutButton_clicked() {
	emit showParent();
	this->hide();
}

void FileBrowser::on_modifyProfile_clicked()
{
	this->m_modifyProfile = new ModifyProfile(m_socketHandler,this->username);
	m_modifyProfile->show();
	connect(m_modifyProfile, &ModifyProfile::showParent, this, &FileBrowser::childWindowClosed);
	this->hide();
}


void FileBrowser::childWindowClosed() {
	this->show();
}
void FileBrowser::editorClosed(QString file) {
	Editor* editor = m_textEditors.find(file)->second;
	editor->deleteLater();
	m_textEditors.erase(file);
}

void FileBrowser::mousePressEvent(QMouseEvent* event) {
	show();
}
