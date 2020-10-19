#include "FileBrowser.h"
#include <QMap>

FileBrowser::FileBrowser(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<QPixmap> profileImage, QColor userColor, QString email, QString username,
	int clientID, QWidget* parent)
	: QMainWindow(parent), m_socketHandler(socketHandler), m_profileImage(profileImage), m_userColor(userColor), m_timer(new QTimer(this)),
	m_openAfterUri(false), email(email), username(username)
{
	ui.setupUi(this);
	ui.username->setText(username);
	ui.profileImage->setPixmap(*m_profileImage);
	

	

	//this->username = username;
	this->clientID = clientID;
	connect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &FileBrowser::handleNewMessage);
	connect(m_timer, &QTimer::timeout, this, &FileBrowser::showErrorMessage);

	//ilio
	ui.renameFile->setVisible(false);
	ui.deleteFile->setVisible(false);
	ui.addSharedFileButton->setVisible(false);
	connect(ui.fileList, &QListWidget::itemSelectionChanged, this, &FileBrowser::on_file_clicked);
	connect(ui.uriLineEdit, &QLineEdit::textChanged, this, &FileBrowser::on_URI_set);


}

FileBrowser::~FileBrowser()
{
	//qDebug()<<"closing file browser";
}

void FileBrowser::on_fileList_itemDoubleClicked(QListWidgetItem* item) {
	QString filename = item->text();
	if (filename == "") {
		return;
	}
	auto it = m_textEditors.find(item->data(Qt::UserRole).toInt());
	Editor* editor;
	//controllo per vedere se l'editor del file che sto cercando di aprire esista già o no
	if (it == m_textEditors.end()) {
		//caso in cui si fa doppio click sul file di test
		if (filename == "test file") {
			QString path = QDir::currentPath().append("\\PROVA SCRITTURA.txt");
			editor = new Editor(m_socketHandler, m_profileImage, m_userColor, path, username, 0, clientID);
			m_textEditors.insert(std::pair<int, Editor*>(-1, editor));
		}
		//caso in cui si apra un file che è presente sul server
		else {
			int id = item->data(Qt::UserRole).toInt();
			editor = new Editor(m_socketHandler, m_profileImage, m_userColor, filename, username, id, clientID);
			m_textEditors.insert(std::pair<int, Editor*>(id, editor));
			QByteArray data = Serialize::fromObjectToArray(Serialize::openDeleteFileSerialize(id, OPEN));
			this->m_socketHandler->writeData(data);
		}
		connect(editor, &Editor::editorClosed, this, &FileBrowser::editorClosed);
		editor->show();
	}
	//nel caso in cui l'editor del file che ho cercato di aprire sia già aperto, questo vien portato in primo piano
	else {
		editor = it->second;
		editor->raise();
	}
}

//creazione di un nuovo file
void FileBrowser::on_newFile_clicked() {
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
void FileBrowser::on_deleteFile_clicked()
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
	int id = current_item->data(Qt::UserRole).toInt();
	QByteArray data = Serialize::fromObjectToArray(Serialize::openDeleteFileSerialize(id, DELETE));
	this->m_socketHandler->writeData(data);
	QListWidgetItem* item = ui.fileList->takeItem(ui.fileList->row(current_item));
	if (item != nullptr) {
		delete item;
		item = nullptr;
		current_item = nullptr;
	}
}

void FileBrowser::on_renameFile_clicked()
{
	QListWidgetItem* current_item = ui.fileList->currentItem();
	if (current_item == nullptr) {
		QMessageBox resultDialog(this);
		QString res_text = "Select a file";
		resultDialog.setInformativeText(res_text);
		resultDialog.exec();
		return;
	}
	//vecchio nome
	QString filename = current_item->text();
	int id = current_item->data(Qt::UserRole).toInt();

	//prendo il nuovo nome
	bool ok;
	QString new_filename = QInputDialog::getText(this, tr("New Nmae"),
		tr("New name:"), QLineEdit::Normal,
		QDir::home().dirName(), &ok);
	if (ok && !new_filename.isEmpty()) {
		//send to server
		QByteArray data = Serialize::fromObjectToArray(Serialize::renameFileSerialize(id,new_filename,RENAME));
		this->m_socketHandler->writeData(data);
		current_item->setText(new_filename);
	}
	else {
		QMessageBox resultDialog(this);
		QString res_text = "File name needed";
		resultDialog.setInformativeText(res_text);
		resultDialog.exec();
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
	disconnect(m_socketHandler.get(), &SocketHandler::dataReceived, this, &FileBrowser::handleNewMessage);
	QByteArray message = Serialize::fromObjectToArray(Serialize::logoutUserSerialize(LOGOUT));
	m_socketHandler->writeData(message);
	emit showParent();
	this->hide();
}

//viene aperta la finestra per la modifica dei dati dell'utente
void FileBrowser::on_modifyProfile_clicked()
{
	this->m_modifyProfile = new ModifyProfile(m_socketHandler, this->username, this->email , this->m_profileImage); //devo passargli l'email devo capire dove prenderla l'immagine è uno shared pointer
	m_modifyProfile->show();
	connect(m_modifyProfile, &ModifyProfile::showParentUpdated, this, &FileBrowser::childWindowClosedAndUpdate);
	this->hide();
}

void FileBrowser::childWindowClosed() {
	this->show();
}

void FileBrowser::childWindowClosedAndUpdate(QString m_username, QString m_email, QSharedPointer<QPixmap> m_profileImage){
	this->username = m_username;
	this->email = email;
	this->m_profileImage = m_profileImage;
	ui.username->setText(m_username);
	ui.profileImage->setPixmap(*m_profileImage);
	this->show();
}

void FileBrowser::editorClosed(int fileId, int siteCounter) {
	Editor* editor = m_textEditors.at(fileId);
	editor->deleteLater();
	QByteArray message = Serialize::fromObjectToArray(Serialize::removeEditingUserSerialize(this->clientID, fileId, REMOVEEDITINGUSER));
	m_socketHandler->writeData(message);
	QByteArray data = Serialize::fromObjectToArray(Serialize::closeFileSerialize(fileId, siteCounter, CLOSE));
	this->m_socketHandler->writeData(data);
	filename_id.remove(fileId);
	m_textEditors.erase(fileId);
	this->raise();
}

void FileBrowser::mousePressEvent(QMouseEvent* event) {
	
		if (ui.fileList->currentItem() == nullptr) {
			ui.renameFile->setVisible(false);
			ui.deleteFile->setVisible(false);
	}
	

	show();
}

void FileBrowser::requestFiles() {
	//aggiungere messaggio per la richiesta della lista dei file al server
	//m_socketHandler->writeData();
}



void FileBrowser::addFiles(QJsonObject filesList) {
	m_timer->stop();
	QMap<int, QString> map = Serialize::fileListUnserialize(filesList);

	for (auto id : map.keys()) {
		QListWidgetItem *item = new QListWidgetItem(map.value(id), ui.fileList);
		item->setData(Qt::UserRole, id);
		this->filename_id.insert(id, map.value(id));
	}
	removeBlank();
}

void FileBrowser::addFile(QJsonObject file) {
	QPair<int, QString> pair = Serialize::newFileUnserialize(file);
	this->filename_id.insert(pair.first, pair.second);
	QListWidgetItem* item = new QListWidgetItem(pair.second, ui.fileList);
	item->setData(Qt::UserRole, pair.first);
	
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
	case SERVER_ANSWER: {
		if (m_openAfterUri) {
			m_timer->stop();
			m_openAfterUri = false;
			QStringList serverMessage = Serialize::responseUnserialize(message);
			QMessageBox resultDialog(this);
			resultDialog.setInformativeText(serverMessage[1]);
			resultDialog.exec();
		}
		break;
	}
	case SHARE: {
		//funzione per mostrare uri
		showURI(message);
		/*QStringList serverMessage = Serialize::responseUnserialize(message);
		QMessageBox resultDialog(this);
		resultDialog.setInformativeText(serverMessage[1]);
		resultDialog.exec();*/
		break;

	}
	case SITECOUNTER: {
		QPair<int, int> fileId_siteCounter = Serialize::siteCounterUnserialize(message);
		auto it = m_textEditors.find(fileId_siteCounter.first);
		if (it != m_textEditors.end()) {
			it->second->setSiteCounter(fileId_siteCounter.second);
		}
		break;
	}
	case RENAME: {
		QStringList list = Serialize::renameFileUnserialize(message);

		QString oldName = list.at(0);
		QString newName = list.at(1);


		QList<QListWidgetItem*> items = ui.fileList->findItems(oldName, Qt::MatchFixedString);
		if (items.size() == 1) {
			items.at(0)->setText(newName);
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

	auto it = m_textEditors.find(m.first);
	if (it != m_textEditors.end()) {
		it->second->remoteAction(m.second);
	}
}

void FileBrowser::showURI(QJsonObject msg) {
	QString serverMessage = Serialize::URIUnserialize(msg);
	QInputDialog resultDialog(this);
	resultDialog.setLabelText("File Link");
	resultDialog.setTextValue(serverMessage);

	/*QDialog* dial = new QDialog(this);
	QLabel label(serverMessage);
	QPushButton button("copia", dial);
	connect(&button, &QPushButton::clicked, this, &FileBrowser::copia);*/

	
	/*dial->exec();*/

	resultDialog.exec();
	

	/*QClipBoard *clipboard = QApplication::clipboard();*/
}

void FileBrowser::on_file_clicked(){
	bool current_item = false;
	current_item = ui.fileList->currentItem()->isSelected();
	if (current_item) {
		ui.renameFile->setVisible(true);
		ui.deleteFile->setVisible(true);
		//connect(ui.centralWidget, &QListWidget::item, this, &FileBrowser::on_file_clicked);
	}
	else {
		ui.renameFile->setVisible(false);
		ui.deleteFile->setVisible(false);
	}
}

void FileBrowser::on_URI_set(){
	if (ui.uriLineEdit->text() != "") {
		ui.addSharedFileButton->setVisible(true);
	}
	else {
		ui.addSharedFileButton->setVisible(false);
	}
}

void FileBrowser::on_addSharedFileButton_clicked() {
	QString uri = ui.uriLineEdit->text();
	if (uri != "") {
		QByteArray message = Serialize::fromObjectToArray(Serialize::openSharedFileSerialize(uri, OPENSHARE));
		bool result = m_socketHandler->writeData(message);
		if (result) {
			m_timer->setInterval(2000);
			m_timer->setSingleShot(true);
			m_timer->start();
		}
		else {
			qDebug() << m_socketHandler->getSocketState();
		}
	}
}


//dialog per mostrare un errore di connessione se la risposta dal server non arriva in tempo
void FileBrowser::showErrorMessage() {
	QMessageBox errorDialog(this);
	errorDialog.setInformativeText("errore di connessione");
	errorDialog.exec();
	qDebug() << "messaggio di errore per il login";
}


