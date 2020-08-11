#include "FileBrowser.h"

FileBrowser::FileBrowser(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<Serialize> messageSerializer, QWidget *parent, QString username)
	: QMainWindow(parent), m_socketHandler(socketHandler), m_messageSerializer(messageSerializer)
{
	ui.setupUi(this);
	ui.usernameLabel->setText(username);
	this->username = username;
	model.setRootPath(QDir::homePath());
	ui.treeView->setModel(&model);
	ui.treeView->setRootIndex(model.index(QDir::currentPath()));
	this->modifyProfile_page = Q_NULLPTR;
}

FileBrowser::~FileBrowser()
{
}

void FileBrowser::on_treeView_doubleClicked(const QModelIndex& index) {
	QString path = this->model.filePath(index);
	auto it = m_textEditors.find(path);
	Editor* editor;
	if (it == m_textEditors.end()) {
		editor = new Editor(m_messageSerializer, this, path);
		m_textEditors.insert(std::pair<QString, Editor*>(path, editor));
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

void FileBrowser::on_modifyProfileButton_clicked()
{
	if (this->modifyProfile_page != Q_NULLPTR) {
		delete this->modifyProfile_page;
		this->modifyProfile_page = Q_NULLPTR;
	}

	this->modifyProfile_page = new ModifyProfile(m_socketHandler, username,this);
	this->modifyProfile_page->show();
	connect(modifyProfile_page, &ModifyProfile::showParent, this, &FileBrowser::childWindowClosed);
	this->hide();
}

void FileBrowser::childWindowClosed()
{
	this->show();
}


