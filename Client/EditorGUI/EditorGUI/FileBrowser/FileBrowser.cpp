#include "FileBrowser.h"
#include <QMap>

FileBrowser::FileBrowser(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<QPixmap> profileImage, QColor userColor, QString username,
	int clientID, QWidget* parent)
	: QMainWindow(parent), m_socketHandler(socketHandler), m_profileImage(profileImage), m_userColor(userColor)
{
	ui.setupUi(this);
	//ui.usernameLabel->setText(" " + username);
	ui.profileImage->setPixmap(*m_profileImage);
	this->username = username;
	this->clientID = clientID;
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
	auto it = m_textEditors.find(this->filename_id.value(filename));
	Editor* editor;
	//controllo per vedere se l'editor del file che sto cercando di aprire esista gi� o no
	if (it == m_textEditors.end()) {
		//caso in cui si fa doppio click sul file di test
		if (filename == "test file") {
			QString path = QDir::currentPath().append("\\PROVA SCRITTURA.txt");
			editor = new Editor(m_socketHandler, m_profileImage, m_userColor, path, username, 0, clientID);
			m_textEditors.insert(std::pair<int, Editor*>(-1, editor));
		}
		//caso in cui si apra un file che � presente sul server
		else {
			editor = new Editor(m_socketHandler, m_profileImage, m_userColor, filename, username, this->filename_id.value(filename), clientID);
			m_textEditors.insert(std::pair<int, Editor*>(this->filename_id.value(filename), editor));
			QByteArray data = Serialize::fromObjectToArray(Serialize::openCloseDeleteFileSerialize(editor->getFileId(), OPEN));
			this->m_socketHandler->writeData(data);
		}
		connect(editor, &Editor::editorClosed, this, &FileBrowser::editorClosed);
		editor->show();
		//chiamare la FILEOPEN
	}
	//nel caso in cui l'editor del file che ho cercato di aprire sia gi� aperto, questo vien portato in primo piano
	else {
		editor = it->second;
		editor->raise();
	}
}

//creazione di un nuovo file
void FileBrowser::on_newFile_Clicked() {
	/*auto model = ui.treeView->model();
	QModelIndex parent = model->index(0, 0);
	model->insertRow(0, parent);
	model->setData(model->index(0, 0, parent), QString("Child Item"));
	ui.treeView->setModel(model);*/
	bool ok;

	//viene aperto un dialog dove immettere il nome del nuovo file
	QString filename = QInputDialog::getText(this, tr("New File"),
		tr("File name:"), QLineEdit::Normal,
		QDir::home().dirName(), &ok);
	if (ok && !filename.isEmpty()) {
		std::cout << "ok";
		//send to server
		QByteArray data = Serialize::fromObjectToArray(Serialize::newFileSerialize(filename, NEWFILE));
		this->m_socketHandler->writeData(data);
	}
	else {
		QMessageBox resultDialog(this);
		QString res_text = "File name needed";
		resultDialog.setInformativeText(res_text);
		resultDialog.exec();
	}

}

//cancellazione del file selezionato nella lista di file
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
	QByteArray data = Serialize::fromObjectToArray(Serialize::openCloseDeleteFileSerialize(id, DELETE_FILE));
	this->m_socketHandler->writeData(data);
	QListWidgetItem* item = ui.fileList->takeItem(ui.fileList->row(current_item));
	if (item != nullptr) {
		delete item;
		item = nullptr;
		current_item = nullptr;
	}
}

void FileBrowser::closeEvent(QCloseEvent* event) {
	QByteArray message = Serialize::fromObjectToArray(Serialize::logoutUserSerialize(LOGOUT));
	m_socketHandler->writeData(message);
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
	QByteArray message = Serialize::fromObjectToArray(Serialize::logoutUserSerialize(LOGOUT));
	m_socketHandler->writeData(message);
	emit showParent();
	this->hide();
}

//viene aperta la finestra per la modifica dei dati dell'utente
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

void FileBrowser::editorClosed(int fileId) {
	Editor* editor = m_textEditors.at(fileId);
	editor->deleteLater();
	m_textEditors.erase(fileId);
	QByteArray message = Serialize::fromObjectToArray(Serialize::removeEditingUserSerialize(this->clientID, fileId, REMOVEEDITINGUSER));
	m_socketHandler->writeData(message);
}

void FileBrowser::mousePressEvent(QMouseEvent* event) {
	show();
}

void FileBrowser::requestFiles() {
	//aggiungere messaggio per la richiesta della lista dei file al server
	//m_socketHandler->writeData();
}

void FileBrowser::addFiles(QJsonObject filesList) {

	QMap<int, QString> map = Serialize::fileListUnserialize(filesList);

	for (auto id : map.keys()) {

		this->filename_id.insert(map.value(id), id);
		ui.fileList->addItem(map.value(id));
	}
	removeBlank();
}

void FileBrowser::addFile(QJsonObject file) {
	QPair<int, QString> pair = Serialize::newFileUnserialize(file);
	this->filename_id.insert(pair.second, pair.first);
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
		break;
	case NEWFILE:
		addFile(message);
		break;
	case NEWEDITINGUSER: {
		QPair<int, QStringList> fileUserInfo = Serialize::addEditingUserUnserialize(message);
		auto it = m_textEditors.find(fileUserInfo.first);
		if (it != m_textEditors.end()) {
			it->second->addEditingUser(fileUserInfo.second);
		}
		break;
	}
	case REMOVEEDITINGUSER: {
		QPair<int, int> userToRemove = Serialize::removeEditingUserUnserialize(message);
		auto it = m_textEditors.find(userToRemove.first);
		if (it != m_textEditors.end()) {
			it->second->removeEditingUser(userToRemove.second);
		}
		break;
	}
	default:
		break;
	}
}

void FileBrowser::processEditorMessage(QJsonObject message)
{
	QPair<int, Message> m = Serialize::messageUnserialize(message);

	auto it = m_textEditors.find(m.first);//cambiare la mappa per usare il file id
	if (it != m_textEditors.end()) {
		it->second->messageReceived(m.second);
	}
}
