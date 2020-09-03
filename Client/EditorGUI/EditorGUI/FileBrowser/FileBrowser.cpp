#include "FileBrowser.h"

FileBrowser::FileBrowser(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<QPixmap> profileImage, QString username, QWidget* parent)
	: QMainWindow(parent), m_socketHandler(socketHandler), m_profileImage(profileImage)
{
	ui.setupUi(this);
	ui.usernameLabel->setText(" " + username);
	this->username = username;
	/*model.setRootPath(QDir::homePath());
	ui.treeView->setModel(&model);
	ui.treeView->setRootIndex(model.index(QDir::currentPath()));*/
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &FileBrowser::addFiles);
	ui.fileList->addItem("test file");
}

FileBrowser::~FileBrowser()
{
}

void FileBrowser::on_fileList_itemDoubleClicked(QListWidgetItem* item) {
	QString filename = item->text();
	auto it = m_textEditors.find(filename);
	Editor* editor;
	if (it == m_textEditors.end()) {
		if (filename == "test file") {
			QString path = QDir::currentPath().append("\\PROVA SCRITTURA.txt");
			editor = new Editor(m_socketHandler, path, username);
			m_textEditors.insert(std::pair<QString, Editor*>(path, editor));
		}
		else {
			editor = new Editor(m_socketHandler, filename, username);
			m_textEditors.insert(std::pair<QString, Editor*>(filename, editor));
		}
		connect(editor, &Editor::editorClosed, this, &FileBrowser::editorClosed);
		editor->show();
	}
	else {
		editor = it->second;
		editor->raise();
	}
}

void FileBrowser::on_newFile_Clicked() {
	/*auto model = ui.treeView->model();
	QModelIndex parent = model->index(0, 0);
	model->insertRow(0, parent);
	model->setData(model->index(0, 0, parent), QString("Child Item"));
	ui.treeView->setModel(model);*/
}

void FileBrowser::closeEvent(QCloseEvent* event) {
	qApp->quit();
}

void FileBrowser::on_logoutButton_clicked() {
	emit showParent();
	this->hide();
}

void FileBrowser::on_modifyProfile_clicked()
{
	this->m_modifyProfile = new ModifyProfile(m_socketHandler, this->username);
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

void FileBrowser::requestFiles() {
	//aggiungere messaggio per la richiesta della lista dei file al server
	//m_socketHandler->writeData();
}

void FileBrowser::addFiles(QJsonObject filesList) {
	QString filename = Serialize::fileNameUnserialize(filesList);
	ui.fileList->addItem(filename);
}
