#include "Editor.h"
#include <QtWidgets>
#include <QCloseEvent>
#include <QComboBox>
#include <QFontComboBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QColorDialog>
#include <QChar>
#include <Qdebug>

#define PADDING 10
#define	ICONSIZE 30
#define RADIUS ICONSIZE/2

Editor::Editor(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<QPixmap> profileImage,
	QString path, QString username, int fileId, int clientID , QWidget* parent)
	: QMainWindow(parent), m_socketHandler(socketHandler), m_fileId(fileId),
	m_timer(new QTimer(this)), m_username(username), m_showingEditingUsers(false),
	m_profileImage(profileImage)
{
	ui.setupUi(this);
	m_textEdit = new MyTextEdit(this);
	ui.gridLayout->addWidget(m_textEdit);
	m_textEdit->setStyleSheet("QTextEdit { padding-left:10; padding-top:10; padding-bottom:10; padding-right:10}");
	this->parent = parent;
	this->filePath = path;
	this->ID = clientID;
	//m_textEdit->setText(path);
	createActions();
	this->loadFile(this->filePath);
	QTextCursor TC = m_textEdit->textCursor();
	TC.setPosition(0);
	m_textEdit->setTextCursor(TC);
	setCurrentFile(QString());
	setUnifiedTitleAndToolBarOnMac(true);

	connect(m_textEdit, &QTextEdit::undoAvailable, this->actionUndo, &QAction::setEnabled);
	connect(m_textEdit, &QTextEdit::redoAvailable, this->actionRedo, &QAction::setEnabled);
	connect(m_textEdit, &QTextEdit::textChanged, this, &Editor::on_textEdit_textChanged);
	connect(m_textEdit, &QTextEdit::cursorPositionChanged, this, &Editor::on_textEdit_cursorPositionChanged);
	connect(m_textEdit, &MyTextEdit::clickOnTextEdit, this, &Editor::clickOnTextEdit);
	connect(m_socketHandler.get(), SIGNAL(SocketHandler::dataReceived(QJsonObject)), this, SLOT(messageReceived(QJsonObject)));

	this->alignmentChanged(this->m_textEdit->alignment());
	this->colorChanged(this->m_textEdit->textColor());

	//MATTIA--------------------------------------------------------------------------------------
	//qui vanno fatte tutte le connect che sono in main window a debora??
	connect(m_textEdit, &QTextEdit::cursorPositionChanged, this, &Editor::updateLastPosition);
	//connect(m_timer, SIGNAL(timeout()), this, SLOT(writeText()));
	

	this->_CRDT = new CRDT(this->ID);//METTERE L'ID DATO DAL SERVER!!!!!!!!!!!!!!!!!
	this->remoteEvent = false;
	lastCursor = 0;
	this->lastStart = this->lastEnd = 0;
	this->lastText = "";
	writeText();
	//FINE----------------------------------------------------------------------------------

#ifdef Q_OS_MACOS
	// Use dark text on light background on macOS, also in dark mode.
	QPalette pal = this->m_textEdit->palette();
	pal.setColor(QPalette::Base, QColor(Qt::white));
	pal.setColor(QPalette::Text, QColor(Qt::black));
	this->m_textEdit->setPalette(pal);
#endif
}

Editor::~Editor()
{
	//MATTIA-----------------
	//bisogna impl la regola dei 3??????
	delete this->_CRDT;
	this->_CRDT = nullptr;
	//FINE---------------------
}

void Editor::closeEvent(QCloseEvent* event) {
	emit editorClosed(this->filePath);
	this->close();
}

void Editor::loadFile(const QString& fileName) {
	//il file andr� caricato con quello inviato dal server
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Application"),
			tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
		return;
	}

	QTextStream in(&file);
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
	QString temp = in.readAll();
	m_textEdit->setPlainText(temp);
	m_timer->setSingleShot(true);
	m_timer->setInterval(2000);
	m_timer->start();

	//va fatta la lettura del file per il CRDT
	//QChar currentChar = textEdit.at
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif

	setCurrentFile(fileName);
	//statusBar()->showMessage(tr("File loaded"), 2000);
}

void Editor::open() {
	if (maybeSave()) {
		QString fileName = QFileDialog::getOpenFileName(this);
		if (!fileName.isEmpty())
			loadFile(fileName);
	}
}

void Editor::setCurrentFile(const QString& fileName) {
	this->curFile = fileName;
	m_textEdit->document()->setModified(false);
	setWindowModified(false);

	QString shownName = curFile;
	if (curFile.isEmpty())
		shownName = "untitled.txt";
	setWindowFilePath(shownName);
}

bool Editor::maybeSave() {
	if (!m_textEdit->document()->isModified())
		return true;
	const QMessageBox::StandardButton ret
		= QMessageBox::warning(this, tr("Application"),
			tr("The document has been modified.\n"
				"Do you want to save your changes?"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	switch (ret) {
	case QMessageBox::Save:
		//return save();
	case QMessageBox::Cancel:
		return false;
	default:
		break;
	}
	return true;
}

void Editor::createActions() {
	/*const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(""));
	QAction* newAct = new QAction(newIcon, tr("&New"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create a new file"));
	connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
	fileMenu->addAction(newAct);
	fileToolBar->addAction(newAct);*/

	ui.toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

	const QIcon openIcon = QIcon::fromTheme("document-open", QIcon("./Icons/plus.png"));
	this->openAct = new QAction(openIcon, tr("&Open..."), this);
	this->openAct->setShortcuts(QKeySequence::Open);
	this->openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, &QAction::triggered, this, &Editor::open);
	ui.menuFile->addAction(this->openAct);
	ui.toolBar->addAction(this->openAct);

	const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon("./Icons/013-cut.png"));
	this->cutAct = new QAction(cutIcon, tr("&Cut..."), this);
	this->cutAct->setShortcuts(QKeySequence::Cut);
	this->cutAct->setStatusTip(tr("Cut text"));
	ui.menuModifica->addAction(this->cutAct);
	ui.toolBar->addAction(this->cutAct);

	const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon("./Icons/011-copy.png"));
	this->copyAct = new QAction(copyIcon, tr("&Copy..."), this);
	this->copyAct->setShortcuts(QKeySequence::Cut);
	this->copyAct->setStatusTip(tr("Copy text"));
	ui.menuModifica->addAction(this->copyAct);
	ui.toolBar->addAction(this->copyAct);

#ifndef QT_NO_PRINTER
	//const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png"));
	this->printAction = ui.menuFile->addAction(QIcon("./Icons/export.png"), tr("&Print..."), this, &Editor::filePrint);
	this->printAction->setPriority(QAction::LowPriority);
	this->printAction->setShortcut(QKeySequence::Print);
	ui.toolBar->addAction(this->printAction);

	//const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png"));
	ui.menuFile->addAction(QIcon("./Icons/review.png"), tr("Print Preview..."), this, &Editor::filePrintPreview);

	const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon("./Icons/pdf.png"));
	this->exportPDFAction = ui.menuFile->addAction(exportPdfIcon, tr("&Export PDF..."), this, &Editor::filePrintPdf);
	this->exportPDFAction->setPriority(QAction::LowPriority);
	this->exportPDFAction->setShortcut(Qt::CTRL + Qt::Key_D);
	ui.toolBar->addAction(this->exportPDFAction);

	//menu->addSeparator();
#endif

	const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon("./Icons/058-undo.png"));
	this->actionUndo = ui.menuModifica->addAction(undoIcon, tr("&Undo"), m_textEdit, &QTextEdit::undo);
	this->actionUndo->setShortcut(QKeySequence::Undo);
	ui.toolBar->addAction(this->actionUndo);

	const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon("./Icons/044-redo.png"));
	this->actionRedo = ui.menuModifica->addAction(redoIcon, tr("&Redo"), m_textEdit, &QTextEdit::redo);
	this->actionRedo->setShortcut(QKeySequence::Redo);
	ui.toolBar->addAction(this->actionRedo);

	this->italicAct = new QAction(QIcon("./Icons/031-italic.png"), tr("&Corsivo"), this);
	this->italicAct->setCheckable(true);
	this->italicAct->setChecked(false);
	this->italicAct->setPriority(QAction::LowPriority);
	this->italicAct->setShortcut(Qt::CTRL + Qt::Key_I);
	connect(this->italicAct, &QAction::triggered, this, &Editor::makeItalic);
	ui.menuModifica->addAction(this->italicAct);
	ui.toolBar->addAction(this->italicAct);


	this->boldAct = new QAction(QIcon("./Icons/004-bold.png"), tr("&Grassetto"), this);
	this->boldAct->setCheckable(true);
	this->boldAct->setPriority(QAction::LowPriority);
	this->boldAct->setShortcut(Qt::CTRL + Qt::Key_B);
	connect(this->boldAct, &QAction::triggered, this, &Editor::makeBold);
	ui.menuModifica->addAction(this->boldAct);
	ui.toolBar->addAction(this->boldAct);

	//const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/textunder.png"));
	this->underLineAct = new QAction(QIcon("./Icons/057-underline.png"), tr("&Sottilinea"), this);
	this->underLineAct->setShortcut(Qt::CTRL + Qt::Key_U);
	this->underLineAct->setPriority(QAction::LowPriority);
	ui.menuModifica->addAction(this->underLineAct);
	ui.toolBar->addAction(this->underLineAct);
	this->underLineAct->setCheckable(true);
	connect(this->underLineAct, &QAction::triggered, this, &Editor::makeUnderlined);

	//this->actionImage = new QAction(cutIcon, tr("&Inserisci immagine"), this);
	//ui.menuModifica->addAction(this->actionImage);
	//ui.toolBar->addAction(this->actionImage);
	//connect(this->actionImage, &QAction::triggered, this, &Editor::insertImage);

	ui.toolBar = addToolBar(tr("Format Actions"));
	addToolBarBreak(Qt::TopToolBarArea);
	addToolBar(ui.toolBar);

	this->comboStyle = new QComboBox(ui.toolBar);
	ui.toolBar->addWidget(comboStyle);
	this->comboStyle->addItem("Standard");
	this->comboStyle->addItem("Bullet List (Disc)");
	this->comboStyle->addItem("Bullet List (Circle)");
	this->comboStyle->addItem("Bullet List (Square)");
	this->comboStyle->addItem("Ordered List (Decimal)");
	this->comboStyle->addItem("Ordered List (Alpha lower)");
	this->comboStyle->addItem("Ordered List (Alpha upper)");
	this->comboStyle->addItem("Ordered List (Roman lower)");
	this->comboStyle->addItem("Ordered List (Roman upper)");
	this->comboStyle->addItem("Heading 1");
	this->comboStyle->addItem("Heading 2");
	this->comboStyle->addItem("Heading 3");
	this->comboStyle->addItem("Heading 4");
	this->comboStyle->addItem("Heading 5");
	this->comboStyle->addItem("Heading 6");
	connect(this->comboStyle, QOverload<int>::of(&QComboBox::activated), this, &Editor::textStyle);

	this->comboFont = new QFontComboBox(ui.toolBar);
	ui.toolBar->addWidget(this->comboFont);
	connect(this->comboFont, QOverload<const QString&>::of(&QComboBox::activated), this, &Editor::textFamily);

	this->comboSize = new QComboBox(ui.toolBar);
	this->comboSize->setObjectName("comboSize");
	ui.toolBar->addWidget(this->comboSize);
	this->comboSize->setEditable(true);

	const QList<int> standardSizes = QFontDatabase::standardSizes();
	for (int size : standardSizes)
		this->comboSize->addItem(QString::number(size));
	this->comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

	connect(this->comboSize, QOverload<const QString&>::of(&QComboBox::activated), this, &Editor::textSize);

	QPixmap pix(16, 16);
	pix.fill(Qt::black);
	this->actionTextColor = ui.menuModifica->addAction(pix, tr("&Color..."), this, &Editor::textColor);
	ui.toolBar->addAction(actionTextColor);

#ifndef QT_NO_CLIPBOARD
	cutAct->setEnabled(false);
	copyAct->setEnabled(false);
	connect(m_textEdit, &QTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
	connect(m_textEdit, &QTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD

	//const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/textleft.png"));
	this->actionAlignLeft = new QAction(QIcon("./Icons/left-align.png"), tr("&Left"), this);
	this->actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
	this->actionAlignLeft->setCheckable(true);
	this->actionAlignLeft->setPriority(QAction::LowPriority);
	//const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/textcenter.png"));
	this->actionAlignCenter = new QAction(QIcon("./Icons/center-align.png"), tr("C&enter"), this);
	this->actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
	this->actionAlignCenter->setCheckable(true);
	this->actionAlignCenter->setPriority(QAction::LowPriority);
	//const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/textright.png"));
	this->actionAlignRight = new QAction(QIcon("./Icons/right-align.png"), tr("&Right"), this);
	this->actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
	this->actionAlignRight->setCheckable(true);
	this->actionAlignRight->setPriority(QAction::LowPriority);
	//const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/textjustify.png"));
	this->actionAlignJustify = new QAction(QIcon("./Icons/justify.png"), tr("&Justify"), this);
	this->actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
	this->actionAlignJustify->setCheckable(true);
	this->actionAlignJustify->setPriority(QAction::LowPriority);

	QActionGroup* alignGroup = new QActionGroup(this);
	connect(alignGroup, &QActionGroup::triggered, this, &Editor::textAlign);

	if (QApplication::isLeftToRight()) {
		alignGroup->addAction(actionAlignLeft);
		alignGroup->addAction(actionAlignCenter);
		alignGroup->addAction(actionAlignRight);
	}
	else {
		alignGroup->addAction(actionAlignRight);
		alignGroup->addAction(actionAlignCenter);
		alignGroup->addAction(actionAlignLeft);
	}
	alignGroup->addAction(actionAlignJustify);

	ui.toolBar->addActions(alignGroup->actions());
	ui.menuModifica->addActions(alignGroup->actions());

	QWidget* spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	ui.toolBar->addWidget(spacer);

	m_actionShowEditingUsers = new QAction(QIcon(*m_profileImage), tr(m_username.toUtf8()), this);
	m_actionShowEditingUsers->setPriority(QAction::LowPriority);
	ui.toolBar->addAction(m_actionShowEditingUsers);
	connect(m_actionShowEditingUsers, &QAction::triggered, this, &Editor::showEditingUsers);

	m_editingUsersList = new QListWidget(this);

	m_editingUsersList->move(QPoint(ui.toolBar->pos().x() + window()->geometry().width() - 160, ui.toolBar->pos().y() + ui.toolBar->geometry().height() + 43));
	m_editingUsersList->setMinimumWidth(150);
	m_editingUsersList->setMinimumHeight(100);
	m_editingUsersList->setIconSize(QSize(ICONSIZE, ICONSIZE));
	m_editingUsersList->hide();
}

void Editor::makeItalic() {
	QTextCharFormat fmt;
	fmt.setFontItalic(this->italicAct->isChecked());
	this->mergeFormatOnWordOrSelection(fmt);
}

void Editor::makeBold() {
	QTextCharFormat fmt;
	fmt.setFontWeight(this->boldAct->isChecked() ? QFont::Bold : QFont::Normal);
	this->mergeFormatOnWordOrSelection(fmt);
}

void Editor::makeUnderlined() {
	QTextCharFormat fmt;
	fmt.setFontUnderline(this->underLineAct->isChecked());
	mergeFormatOnWordOrSelection(fmt);
}

void Editor::textStyle(int styleIndex)
{
	QTextCursor cursor = m_textEdit->textCursor();
	QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;

	switch (styleIndex) {
	case 1:
		style = QTextListFormat::ListDisc;
		break;
	case 2:
		style = QTextListFormat::ListCircle;
		break;
	case 3:
		style = QTextListFormat::ListSquare;
		break;
	case 4:
		style = QTextListFormat::ListDecimal;
		break;
	case 5:
		style = QTextListFormat::ListLowerAlpha;
		break;
	case 6:
		style = QTextListFormat::ListUpperAlpha;
		break;
	case 7:
		style = QTextListFormat::ListLowerRoman;
		break;
	case 8:
		style = QTextListFormat::ListUpperRoman;
		break;
	default:
		break;
	}

	cursor.beginEditBlock();

	QTextBlockFormat blockFmt = cursor.blockFormat();

	if (style == QTextListFormat::ListStyleUndefined) {
		blockFmt.setObjectIndex(-1);
		int headingLevel = styleIndex >= 9 ? styleIndex - 9 + 1 : 0; // H1 to H6, or Standard
		blockFmt.setHeadingLevel(headingLevel);
		cursor.setBlockFormat(blockFmt);

		int sizeAdjustment = headingLevel ? 4 - headingLevel : 0; // H1 to H6: +3 to -2
		QTextCharFormat fmt;
		fmt.setFontWeight(headingLevel ? QFont::Bold : QFont::Normal);
		fmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
		cursor.select(QTextCursor::LineUnderCursor);
		cursor.mergeCharFormat(fmt);
		m_textEdit->mergeCurrentCharFormat(fmt);
	}
	else {
		QTextListFormat listFmt;
		if (cursor.currentList()) {
			listFmt = cursor.currentList()->format();
		}
		else {
			listFmt.setIndent(blockFmt.indent() + 1);
			blockFmt.setIndent(0);
			cursor.setBlockFormat(blockFmt);
		}
		listFmt.setStyle(style);
		cursor.createList(listFmt);
	}

	cursor.endEditBlock();
}

void Editor::mergeFormatOnWordOrSelection(const QTextCharFormat& format) {
	QTextCursor cursor = m_textEdit->textCursor();
	cursor.mergeCharFormat(format);
	m_textEdit->mergeCurrentCharFormat(format);
}

void Editor::textSize(const QString& p) {
	qreal pointSize = p.toFloat();
	if (p.toFloat() > 0) {
		QTextCharFormat fmt;
		fmt.setFontPointSize(pointSize);
		this->mergeFormatOnWordOrSelection(fmt);
	}
	QTextCursor TC = m_textEdit->textCursor();
	m_textEdit->updateTextSize();
	m_textEdit->setTextCursor(TC);
}

void Editor::textFamily(const QString& f) {
	QTextCharFormat fmt;
	fmt.setFontFamily(f);
	this->mergeFormatOnWordOrSelection(fmt);
}

void Editor::filePrint() {
#if QT_CONFIG(printdialog)
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog* dlg = new QPrintDialog(&printer, this);
	if (m_textEdit->textCursor().hasSelection())
		dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
	dlg->setWindowTitle(tr("Print Document"));
	if (dlg->exec() == QDialog::Accepted)
		m_textEdit->print(&printer);
	delete dlg;
#endif
}

void Editor::filePrintPreview() {
#if QT_CONFIG(printpreviewdialog)
	QPrinter printer(QPrinter::HighResolution);
	QPrintPreviewDialog preview(&printer, this);
	connect(&preview, &QPrintPreviewDialog::paintRequested, this, &Editor::printPreview);
	preview.exec();
#endif
}

void Editor::printPreview(QPrinter* printer) {
#ifdef QT_NO_PRINTER
	Q_UNUSED(printer);
#else
	m_textEdit->print(printer);
#endif
}

void Editor::filePrintPdf() {
#ifndef QT_NO_PRINTER
	//! [0]
	QFileDialog fileDialog(this, tr("Export PDF"));
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
	fileDialog.setDefaultSuffix("pdf");
	if (fileDialog.exec() != QDialog::Accepted)
		return;
	QString fileName = fileDialog.selectedFiles().first();
	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(fileName);
	m_textEdit->document()->print(&printer);
	statusBar()->showMessage(tr("Exported \"%1\"").arg(QDir::toNativeSeparators(fileName)));
	//! [0]
#endif
}

//MATTIA-----------------------------------------------------------------------------------------------------------

void Editor::on_textEdit_textChanged() {



	//QTextCursor TC = m_textEdit->textCursor();
	//QString temp = m_textEdit->toPlainText();
	//int pos = TC.position();
	//char chr;
	//
	//if(TC.position()==0)
	//	chr = m_textEdit->toPlainText().at(TC.position()).toLatin1();
	//else
	//	chr = m_textEdit->toPlainText().at(TC.position()-1).toLatin1();
	//FormatStructure FS;
	//FS.isUnderlined = TC.charFormat().fontUnderline();
	//FS.isItalic = TC.charFormat().fontItalic();
	//FS.weight = TC.charFormat().fontWeight();
	//FS.size = TC.charFormat().fontPointSize();
	//FS.font = TC.charFormat().fontFamily();


	//il messaggio va mandato al serializzatore

	if (this->remoteEvent)
		return;


	QTextCursor TC = m_textEdit->textCursor();
	//DEBUG
	int curr = TC.position();
	int last = this->lastCursor;
	if (this->lastText.compare(this->m_textEdit->toPlainText()) == 0)// se non � insert o delete--> change in the format
		this->localStyleChange();

	else if (TC.position() <= lastCursor) {
		//� una delete		
		localDelete();
	}
	else {
		//� una insert
		localInsert();
	}
	//aggiorno
	this->lastText = m_textEdit->toPlainText();
	this->lastCursor = TC.position();
}


void Editor::localInsert() {
	QTextCursor TC = m_textEdit->textCursor();
	int li = TC.anchor();

	//funziona sia per inserimento singolo che per inserimento multiplo--> incolla
	for (int i = lastCursor; i < TC.position(); i++) {
		if (i < 0)
			return;

		int pos = i;

		char chr = m_textEdit->toPlainText().at(pos).toLatin1();

		//std::vector<Message> vett;
		////debug purposes
		//if (chr == '�') {
		//	vett = this->_CRDT->readFromFile("C:/Users/Mattia Proietto/Desktop/prova_save.txt");

		//	QByteArray arr = Serialize::fromObjectToArray( Serialize::messageSerialize(vett[0], INSERT));

		//	qint64 len = arr.size();

		//	for (auto v : vett) {
		//		this->remoteAction(v);
		//		std::cout << "inseriton" << std::endl;
		//	}
		//	std::cout << "FINE" << std::endl;
		//	//this->_CRDT->saveOnFile("C:/Users/Mattia Proietto/Desktop/prova_save.txt");
		//	return;
		//}

		QTextCharFormat format = TC.charFormat();
		QFont font = format.font();
		QColor color = format.foreground().color();
		Qt::AlignmentFlag alignment = this->getAlignementFlag(m_textEdit->alignment());

		Message m = this->_CRDT->localInsert(pos, chr, font, color, alignment);
		QJsonObject packet = Serialize::messageSerialize(m, MESSAGE);
		m_socketHandler->writeData(Serialize::fromObjectToArray(packet)); // -> socket

		//std::string prova = m.getSymbol().getFont().toString().toStdString();
		//std::cout << "prova" << std::endl;
	}
}

void Editor::localDelete() {
	QTextCursor TC = m_textEdit->textCursor();

	int start, end;
	if (this->lastStart != 0 && this->lastEnd != 0) {

		start = this->lastStart;
		end = this->lastEnd;
	}
	else {
		
	if (m_textEdit->toPlainText().size() == 0) {
			//vuol dire che ho eliminato tutto
			start = 0;
			end = this->lastText.size();
		}
		else {
			start = TC.position();
			end = lastCursor;
		}

	}


	for (int i = end; i > start; i--) {
		Message m = this->_CRDT->localErase(i - 1);
		QJsonObject packet = Serialize::messageSerialize(m, MESSAGE);
		m_socketHandler->writeData(Serialize::fromObjectToArray(packet)); // -> socket
	}

	//this->lastStart = 0;
	//this -> lastEnd = 0;
	//for (int i = lastCursor; i > TC.position(); i--) {
	//	Message m = this->_CRDT->localErase(i - 1);
	//	//send to socket

	//}
	//if (lastCursor == TC.position()) {//se sono uguale sono nel caso particolare in cui seeziono da dx a sx
	//								// e quando cancello il cursore non cambia
	//	deleteDxSx();
	//}

}
//
//void Editor::deleteDxSx() {
//
//	QTextCursor TC = m_textEdit->textCursor();
//	int start = this->lastStart;
//	int end = this->lastEnd;
//	for (int i = end; i > start; i--) {
//		Message m = this->_CRDT->localErase(i - 1);
//		//send to socketaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
//	}
//}

void Editor::remoteAction(Message m)
{
	this->remoteEvent = true;//serve ad evitare che l'ontextchange venga triggerato e che quindi vada a mod il cursore e le altre var
	//provato anche con connect e disconnect del segnale 

	//inserisce auth nel CRDT
	__int64 index = this->_CRDT->process(m);//index rappresenta l'indice della lettere nel testo corrente dove fare insert/delete

	/*--------------------------------------------------------------------------------
	queste due funzioni in base al valore dell'indice a cui inserire e a quello del cursore corrente decidono
	se � il caso o meno di andare a modificare il cursore incrementandolo o decrementandolo
	++ iff this<last
	-- iff this<last
	solo se sto inserendo ad un indice inferiore a quello a cui sto srivendo poiche avr� un char in meno o in piu
	-----------------------------------------------------------------------------------------*/
	//switch (m.getAction()) {
	//case INSERT:
	//	updateViewAfterInsert(m, index);
	//	maybeincrement(index);
	//	break;
	//case DELETE_S:
	//	updateViewAfterDelete(m, index);
	//	maybedecrement(index);
	//	break;
	//case CHANGE:
	//	updateViewAfterStyleChange(m, index);
	//	break;
	//default:
	//	break;
	//}
	QTextCursor TC = m_textEdit->textCursor();
	int pos = TC.position();
	disconnect(m_textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
	m_textEdit->handleMessage(m.getSenderId(), m, index); //gestione dei messaggi remoti spostata in CustomCursor
	connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
	TC.setPosition(pos);
	m_textEdit->setTextCursor(TC);

	switch (m.getAction()) {
	case INSERT:
		maybeincrement(index);
		break;
	case DELETE_S:
		maybedecrement(index);
		break;
	default:
		break;
	}

	this->lastText = m_textEdit->toPlainText();
	//Augusto: bisogna aggiungere anche la condizione se si riceve un'immagine dal server ed una per i messaggi 
	//dei cursori degli altri client

	this->remoteEvent = false;
}

void Editor::maybeincrement(__int64 index)
{
	//se sono prima di dove sto scrivendo il cursore deve aumentare di 1
	if (index < this->lastCursor)
		lastCursor++;
}

void Editor::maybedecrement(__int64 index)
{
	//se sono prima di dove sto scrivendo il cursore deve diminuire di 1
	if (index < this->lastCursor)
		lastCursor--;
}

void Editor::updateLastPosition()
{
	QTextCursor TC = m_textEdit->textCursor();
	if (TC.hasSelection()) {

		lastStart = TC.selectionStart();
		lastEnd = TC.selectionEnd();

	}
	else
	{
		lastStart = 0;
		lastEnd = 0;
	}

	if (this->remoteEvent)
		return;

	if (this->lastText.compare(this->m_textEdit->toPlainText()))//aggiorno  il cursore solo se non � delete or insert
		return;


	int in = TC.position();
	int l = lastCursor;
	this->lastCursor = TC.position();
	//emit per dire che mi sono spostato-->aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

}

void Editor::updateViewAfterInsert(Message m, __int64 index)
{
	disconnect(m_textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
	//retrieving remote state
	QChar chr(m.getSymbol().getChar());
	QFont r_font = m.getSymbol().getFont();
	QColor r_color = m.getSymbol().getColor();
	Qt::AlignmentFlag alignment = m.getSymbol().getAlignment();
	/// 
	/// la parte qui sotto potrbbe essere inutile per poter scrivere sul text editor 
	/// conviene usare la Qchartextedit--> vedi nota vocale su telegram a me stesso
	//o vedere changeViewAfterInsert in mainwindow.cpp debora

	QTextCursor TC = m_textEdit->textCursor();
	// saving current state
	//QFont font = m_textEdit->currentFont();
	//QColor color = m_textEdit->textColor();

	//colore del char che arriva
	QTextCharFormat format;
	format.setFont(r_font);
	format.setForeground(r_color);
	//m_textEdit->setFont(r_font);
	//m_textEdit->setTextColor(r_color);

	TC.setPosition(index);
	TC.insertText(chr, format);

	QTextBlockFormat blockFormat = TC.blockFormat();
	blockFormat.setAlignment(alignment);

	TC.mergeBlockFormat(blockFormat);


	//ripristino
	TC.setPosition(this->lastCursor);
	//m_textEdit->setFont(font);
	//m_textEdit->setTextColor(color);
	connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void Editor::updateViewAfterDelete(Message m, __int64 index)
{
	if (index == -1)
		return;//non devo fare niente in questo caso ho provato ad eliminare ma non ho trovato il carattere-->MADARE ERROR, ECCEZIONE??????

	disconnect(m_textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
	QTextCursor TC = m_textEdit->textCursor();
	TC.setPosition(index);
	TC.deleteChar();
	//TC.deletePreviousChar();//oppure � questo se il primo non funziona
	TC.setPosition(this->lastCursor);
	connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void Editor::updateViewAfterStyleChange(Message m, __int64 index)
{
	disconnect(m_textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
	QTextCursor TC = m_textEdit->textCursor();
	TC.setPosition(index);
	TC.deleteChar();

	QChar chr(m.getSymbol().getChar());
	QFont r_font = m.getSymbol().getFont();
	QColor r_color = m.getSymbol().getColor();
	Qt::AlignmentFlag alignment = m.getSymbol().getAlignment();

	//change color, font and style in general
	QTextCharFormat format;
	format.setFont(r_font);
	format.setForeground(r_color);
	TC.mergeCharFormat(format);

	//change alignment
	QTextBlockFormat blockFormat = TC.blockFormat();
	blockFormat.setAlignment(alignment);

	TC.mergeBlockFormat(blockFormat);
	TC.insertText(chr, format);

	TC.setPosition(this->lastCursor);

	connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}


void Editor::localStyleChange()
{
	int start, end;

	start = this->lastStart;
	end = this->lastEnd;
	QTextCursor TC = m_textEdit->textCursor();


	for (int i = end; i > start; i--) {
		int pos = i - 1;
		TC.setPosition(pos + 1);
		QTextCharFormat format = TC.charFormat();
		char chr = m_textEdit->toPlainText().at(pos).toLatin1();
		QFont font = format.font();
		QColor color = format.foreground().color();
		Qt::AlignmentFlag alignment = this->getAlignementFlag(m_textEdit->alignment());


		Symbol s = this->_CRDT->getSymbol(pos);
		
		if (s.getAlignment()!=alignment || s.getColor()!= color || s.getFont() != font) {

			//scrivo sul socket solo se c'e stato un vero cambio --> meno banda e carico per il server
			Message m = this->_CRDT->localChange(pos, chr, font, color, alignment);
			QJsonObject packet = Serialize::messageSerialize(m, MESSAGE);
			m_socketHandler->writeData(Serialize::fromObjectToArray(packet)); // -> socket
		}

	}
	int cazzi = 0;
	//this->lastStart = 0;
	//this->lastEnd = 0;
}

//FINE-------------------------------------------------------------------------------------------------------------

void Editor::keyPressEvent(QKeyEvent* e) {
	//NON SO FARLO FUNZIONARE
	//switch (e->key())
	//{
	//case Qt::Key_Backspace:
	//	this->Edelete(0);
	//	break;
	//case Qt::Key_Delete:
	//case Qt::Key_Cancel:
	//	//EditorDelete
	//	this->Edelete(1);
	//	break;

	//default:
	//	//editor insert-->crea una funzione che fa quello che per ora � in texchanged
	//	this->Einsert();
	//	break;
	//	
	//}
	//au
//	ui.label->setText(e->text());
}

void Editor::insertImage() {
	QString url = QFileDialog::getOpenFileName(this, tr("Scegli immagine"), QDir::homePath(), "Immagini (*.jpg *.png *.jpeg)");
	if (url != "") {
		QUrl uri(url);
		QPixmap image(url);
		QTextDocument* textDocument = m_textEdit->document();
		textDocument->addResource(QTextDocument::ImageResource, uri, QVariant(image));
		QTextCursor cursor = m_textEdit->textCursor();
		QTextImageFormat imageFormat;
		imageFormat.setWidth(image.width());
		imageFormat.setHeight(image.height());
		imageFormat.setName(uri.toString());
		cursor.insertImage(imageFormat);
		QJsonObject imageSerialized = Serialize::imageSerialize(image, 2);
		//serve un messaggio che abbia anche la posizione del cursore per l'immagine, oltre che il ridimensionamento
	}
}

void Editor::textAlign(QAction* a) {
	if (a == actionAlignLeft)
		this->m_textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
	else if (a == actionAlignCenter)
		this->m_textEdit->setAlignment(Qt::AlignHCenter);
	else if (a == actionAlignRight)
		this->m_textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
	else if (a == actionAlignJustify)
		this->m_textEdit->setAlignment(Qt::AlignJustify);
}

void Editor::alignmentChanged(Qt::Alignment a) {
	if (a & Qt::AlignLeft)
		this->actionAlignLeft->setChecked(true);
	else if (a & Qt::AlignHCenter)
		this->actionAlignCenter->setChecked(true);
	else if (a & Qt::AlignRight)
		this->actionAlignRight->setChecked(true);
	else if (a & Qt::AlignJustify)
		this->actionAlignJustify->setChecked(true);
}

void Editor::on_textEdit_cursorPositionChanged() {
	this->alignmentChanged(m_textEdit->alignment());
	QTextCursor TC = m_textEdit->textCursor();
	QTextList* list = TC.currentList();
	if (list) {
		switch (list->format().style()) {
		case QTextListFormat::ListDisc:
			this->comboStyle->setCurrentIndex(1);
			break;
		case QTextListFormat::ListCircle:
			this->comboStyle->setCurrentIndex(2);
			break;
		case QTextListFormat::ListSquare:
			this->comboStyle->setCurrentIndex(3);
			break;
		case QTextListFormat::ListDecimal:
			this->comboStyle->setCurrentIndex(4);
			break;
		case QTextListFormat::ListLowerAlpha:
			this->comboStyle->setCurrentIndex(5);
			break;
		case QTextListFormat::ListUpperAlpha:
			this->comboStyle->setCurrentIndex(6);
			break;
		case QTextListFormat::ListLowerRoman:
			this->comboStyle->setCurrentIndex(7);
			break;
		case QTextListFormat::ListUpperRoman:
			this->comboStyle->setCurrentIndex(8);
			break;
		default:
			this->comboStyle->setCurrentIndex(-1);
			break;
		}
	}
	else {
		int headingLevel = this->m_textEdit->textCursor().blockFormat().headingLevel();
		this->comboStyle->setCurrentIndex(headingLevel ? headingLevel + 8 : 0);
	}

	this->comboFont->setCurrentFont(m_textEdit->currentFont());
	QPixmap pix(16, 16);
	pix.fill(m_textEdit->textColor());
	//this->actionTextColor->setIcon(QIcon("./Icons/040-paint bucket.png"));
	this->actionTextColor->setIcon(pix);
	const QList<int> standardSizes = QFontDatabase::standardSizes();
	int size = m_textEdit->font().pointSize();
	QString t = TC.selectedText();
	this->comboSize->setCurrentIndex(standardSizes.indexOf(size));

    /*Message m(TC.position(), CURSOR, _CRDT->getId());
	m_socketHandler->writeData(Serialize::fromObjectToArray(Serialize::messageSerialize(m, 0)));
	QTextCharFormat format = TC.charFormat();
	int pos = TC.position();
	char c = m_textEdit->toPlainText().at(TC.position()-1).toLatin1();
	QColor color = format.foreground().color();
	Message m2 = _CRDT->localChange(TC.position()-1, c, format.font(), color, this->getAlignementFlag(m_textEdit->alignment()));
	m_socketHandler->writeData(Serialize::fromObjectToArray(Serialize::messageSerialize(m2, 0)));*/
}

void Editor::colorChanged(const QColor& c) {
	QPixmap pix(16, 16);
	pix.fill(c);
	this->actionTextColor->setIcon(pix);
}

void Editor::textColor()
{
	QColor col = QColorDialog::getColor(m_textEdit->textColor(), this);
	if (!col.isValid())
		return;
	QTextCharFormat fmt;
	fmt.setForeground(col);
	mergeFormatOnWordOrSelection(fmt);
	colorChanged(col);
}

Qt::AlignmentFlag Editor::getAlignementFlag(Qt::Alignment al) {
	if (al.testFlag(Qt::AlignLeft))
		return Qt::AlignLeft;
	else if (al.testFlag(Qt::AlignRight))
		return Qt::AlignRight;
	else if (al.testFlag(Qt::AlignCenter) || al.testFlag(Qt::AlignHCenter))
		return Qt::AlignCenter;
	else return Qt::AlignJustify;
}

void Editor::messageReceived(QJsonObject packet) {
	Message m = Serialize::messageUnserialize(packet);
	//TODO check se il messaggio che arriva � relativo al file gestito da questo editor
	QTextCursor TC = m_textEdit->textCursor();
	int pos = TC.position();
	remoteAction(m);
	TC.setPosition(pos);
	m_textEdit->setTextCursor(TC);
}

void Editor::writeText() {
	QTextCursor TC = m_textEdit->textCursor();
	int pos = TC.position();
	addEditingUser(0, "prova", Qt::blue);
	m_textEdit->setCursorPosition(0, 10);
	//m_textEdit->insertText(0, QString("ciao"));
	TC.setPosition(pos);
	m_textEdit->setTextCursor(TC);
}

void Editor::addEditingUser(int id, QString username, QColor userColor) { //da usare ogni volta che un nuove utente accede al file
	m_textEdit->addCursor(id, userColor, username, 0);
	m_editingUsers.push_back(username);

	QPixmap pm(ICONSIZE, ICONSIZE);
	pm.fill(Qt::white);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing, true);
	QPen pen(userColor, 2);
	p.setPen(pen);
	QBrush brush(userColor);
	p.setBrush(brush);
	p.drawEllipse(10, 7, RADIUS, RADIUS);
	QIcon userColorIcon(pm);
	QListWidgetItem* lwi = new QListWidgetItem(userColorIcon, tr(username.toUtf8()), m_editingUsersList);
	lwi->setTextAlignment(Qt::AlignCenter);
	lwi->setSizeHint(QSize(100, 50));
	lwi->setFlags(lwi->flags() & ~Qt::ItemIsSelectable);
	QFont lwiFont = lwi->font();
	lwiFont.setPointSize(lwiFont.pointSize() + 3);
	lwi->setFont(lwiFont);
}

void Editor::removeEditingUser(int id, QString username) {
	m_textEdit->removeCursor(id);
	auto it = m_editingUsers.begin();
	for (it; it != m_editingUsers.end(); it++) {
		if (it->compare(username)) {
			m_editingUsers.erase(it);
			break;
		}
	}

	QList<QListWidgetItem*> lwi = m_editingUsersList->findItems(username, Qt::MatchExactly);
	m_editingUsersList->removeItemWidget(lwi.at(0));
}

void Editor::showEditingUsers() {
	m_showingEditingUsers = true;
	m_editingUsersList->show();
}

void Editor::mousePressEvent(QMouseEvent* e) {
	if (m_showingEditingUsers) {
		m_editingUsersList->hide();
		m_showingEditingUsers = false;
	}
}

void Editor::clickOnTextEdit() {
	if (m_showingEditingUsers) {
		m_editingUsersList->hide();
		m_showingEditingUsers = false;
	}
}