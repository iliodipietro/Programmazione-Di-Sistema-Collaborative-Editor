#include "FileBrowser.h"

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
			editor = new Editor(m_socketHandler, m_profileImage, path, username,0,clientID);
			m_textEditors.insert(std::pair<QString, Editor*>(path, editor));
		}
		else {
			editor = new Editor(m_socketHandler, m_profileImage, filename, username, clientID);
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
	bool ok;
	QString filename = QInputDialog::getText(this, tr("New File"),
		tr("File name:"), QLineEdit::Normal,
		QDir::home().dirName(), &ok);
	if (ok && !filename.isEmpty()) {
		std::cout << "ok";
		//send to server
		Serialize::newFileSerialize(filename,this->username,NEWFILE);
	}
	else {
		QMessageBox resultDialog(this);
		QString res_text = "File name needed";
		resultDialog.setInformativeText(res_text); 
		resultDialog.exec();
	}
		
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

void FileBrowser::handleNewMessage(QJsonObject message)
{
	int type = message.value("type").toInt();
	
	switch (type)
	{
	case MESSAGE:
		processEditorMessage(message);
		break;
	case SERVER_ANSWER:
		addFiles(message);
	default:
		break;
	}
}

void FileBrowser::processEditorMessage(QJsonObject message)
{
	Message m = Serialize::messageUnserialize(message);

	Editor* edit = m_textEditors.at(QDir::currentPath().append("\\PROVA SCRITTURA.txt"));
	edit->remoteAction(m);
}
