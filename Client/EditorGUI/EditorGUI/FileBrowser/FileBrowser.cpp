#include "FileBrowser.h"
#include <QMap>

FileBrowser::FileBrowser(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<QPixmap> profileImage, QString username, int clientID, QWidget* parent)
	: QMainWindow(parent), m_socketHandler(socketHandler), m_profileImage(profileImage)
{
	ui.setupUi(this);
	//ui.usernameLabel->setText(" " + username);
	ui.profileImage->setPixmap(*m_profileImage);
	this->username = username;
	this->clientID = clientID;
	/*model.setRootPath(QDir::homePath());
	ui.treeView->setModel(&model);
	ui.treeView->setRootIndex(model.index(QDir::currentPath()));*/
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &FileBrowser::handleNewMessage);
	connect(ui.newFile, SIGNAL(clicked()), this, SLOT(on_newFile_Clicked()));
	connect(ui.deleteFile, SIGNAL(clicked()), this, SLOT(on_deleteFile_Clicked()));
	//ui.fileList->addItem("test file");
	
}

FileBrowser::~FileBrowser()
{
}

void FileBrowser::on_fileList_itemDoubleClicked(QListWidgetItem* item) {
	QString filename = item->text();
	if (filename == "") {
		return;
	}
	auto it = m_textEditors.find(filename);
	Editor* editor;
	if (it == m_textEditors.end()) {
		if (filename == "test file") {
			QString path = QDir::currentPath().append("\\PROVA SCRITTURA.txt");
			editor = new Editor(m_socketHandler, m_profileImage, path, username,0,clientID);
			m_textEditors.insert(std::pair<QString, Editor*>(path, editor));
		}
		else {
			editor = new Editor(m_socketHandler, m_profileImage, filename, username, this->filename_id.value(filename),clientID);
			m_textEditors.insert(std::pair<QString, Editor*>(filename, editor));
		}
		connect(editor, &Editor::editorClosed, this, &FileBrowser::editorClosed);
		editor->show();
		//chiamare la FILEOPEN
		QByteArray data = Serialize::fromObjectToArray(Serialize::openCloseDeleteFileSerialize(editor->getFileId(),OPEN));
		this->m_socketHandler->writeData(data);
	}
	else {
		editor = it->second;
		editor->raise();
		QByteArray data = Serialize::fromObjectToArray(Serialize::openCloseDeleteFileSerialize(editor->getFileId(), OPEN));
		this->m_socketHandler->writeData(data);
	}
}

void FileBrowser::on_newFile_Clicked() {
	/*auto model = ui.treeView->model();
	QModelIndex parent = model->index(0, 0);
	model->insertRow(0, parent);
	model->setData(model->index(0, 0, parent), QString("Child Item"));
	ui.treeView->setModel(model);*/
	bool ok;
	QString filename = QInputDialog::getText(this, tr("New File"),
		tr("File name:"), QLineEdit::Normal,
		QDir::home().dirName(), &ok);
	if (ok && !filename.isEmpty()) {
		std::cout << "ok";
		//send to server
		QByteArray data = Serialize::fromObjectToArray( Serialize::newFileSerialize(filename,NEWFILE));
		this->m_socketHandler->writeData(data);
	}
	else {
		QMessageBox resultDialog(this);
		QString res_text = "File name needed";
		resultDialog.setInformativeText(res_text); 
		resultDialog.exec();
	}
		
}

void FileBrowser::on_deleteFile_Clicked()
{
	QListWidgetItem* current_item = ui.fileList->currentItem();
	if (current_item == nullptr) {
		QMessageBox resultDialog(this);
		QString res_text = "Select a file";
		resultDialog.setInformativeText(res_text);
		resultDialog.exec();
		return;
	}
	QString filename = current_item->text();
	int id = this->filename_id.value(filename);
	QByteArray data = Serialize::fromObjectToArray( Serialize::openCloseDeleteFileSerialize(id,DELETE_FILE));
	this->m_socketHandler->writeData(data);
	QListWidgetItem* item = ui.fileList->takeItem(ui.fileList->row(current_item));
	if (item != nullptr) {
		delete item;
		item = nullptr;
		current_item = nullptr;
	}
}

void FileBrowser::closeEvent(QCloseEvent* event) {
	qApp->quit();
}

void FileBrowser::removeBlank()
{
	QList<QListWidgetItem*> items = ui.fileList->findItems("", Qt::MatchExactly);
	for (auto it : items) {
		ui.fileList->takeItem(ui.fileList->row(it));
		delete it;
		it = nullptr;
	}

}

void FileBrowser::on_logoutButton_clicked() {
	//eseguire il logout prima
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
	
	QMap<int,QString> map =  Serialize::fileListUnserialize(filesList);
	
	for (auto id : map.keys()) {

		this->filename_id.insert(map.value(id),id);
		ui.fileList->addItem(map.value(id));
	}
	removeBlank();
}

void FileBrowser::addFile(QJsonObject file) {
	QPair<int,QString> pair = Serialize::newFileUnserialize(file);
	this->filename_id.insert(pair.second,pair.first);
	ui.fileList->addItem(pair.second);
	removeBlank();
}

void FileBrowser::handleNewMessage(QJsonObject message)
{
	int type = message.value("type").toInt();
	
	switch (type)
	{
	case MESSAGE:
		processEditorMessage(message);
		break;
	case SEND_FILES:
		addFiles(message);
	case NEWFILE:
		addFile(message);
		break;
	default:
		break;
	}
}

void FileBrowser::processEditorMessage(QJsonObject message)
{
	QPair<int, Message> m = Serialize::messageUnserialize(message);

	auto it = m_textEditors.find(QDir::currentPath().append("\\PROVA SCRITTURA.txt"));//cambiare la mappa per usare il file id
	if (it != m_textEditors.end()) {
		it->second->messageReceived(m.second);
	}
}
