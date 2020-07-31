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

Editor::Editor(QSharedPointer<Serialize> messageSerializer, QWidget* parent, QString path)
	: QMainWindow(parent), m_socketHandler(QSharedPointer<SocketHandler>(new SocketHandler(this))), m_messageSerializer(messageSerializer)
{
	ui.setupUi(this);
	this->parent = parent;
	this->filePath = path;
	//ui.textEdit->setText(path);
	createActions();
	this->loadFile(this->filePath);
	setCurrentFile(QString());
	setUnifiedTitleAndToolBarOnMac(true);

	connect(ui.textEdit, &QTextEdit::undoAvailable, this->actionUndo, &QAction::setEnabled);
	connect(ui.textEdit, &QTextEdit::redoAvailable, this->actionRedo, &QAction::setEnabled);
	connect(m_socketHandler.get(), SIGNAL(SocketHandler::dataReceived(QJsonObject)), this, SLOT(messageReceived(QJsonObject)));

	this->alignmentChanged(this->ui.textEdit->alignment());
	this->colorChanged(this->ui.textEdit->textColor());

	//MATTIA--------------------------------------------------------------------------------------
	//qui vanno fatte tutte le connect che sono in main window a debora
	connect(ui.textEdit, &QTextEdit::cursorPositionChanged, this, &Editor::updateLastPosition);


	this->_CRDT = new CRDT(this->ID);//METTERE L'ID DATO DAL SERVER!!!!!!!!!!!!!!!!!
	this->remoteEvent = false;
	lastCursor = 0;
	//FINE----------------------------------------------------------------------------------

#ifdef Q_OS_MACOS
	// Use dark text on light background on macOS, also in dark mode.
	QPalette pal = this->ui.textEdit->palette();
	pal.setColor(QPalette::Base, QColor(Qt::white));
	pal.setColor(QPalette::Text, QColor(Qt::black));
	this->ui.textEdit->setPalette(pal);
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
	this->close();
}

void Editor::loadFile(const QString& fileName) {
	//il file andrà caricato con quello inviato dal server
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
	ui.textEdit->setPlainText(temp);
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
	ui.textEdit->document()->setModified(false);
	setWindowModified(false);

	QString shownName = curFile;
	if (curFile.isEmpty())
		shownName = "untitled.txt";
	setWindowFilePath(shownName);
}

bool Editor::maybeSave() {
	if (!ui.textEdit->document()->isModified())
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

	const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(""));
	this->openAct = new QAction(openIcon, tr("&Open..."), this);
	this->openAct->setShortcuts(QKeySequence::Open);
	this->openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, &QAction::triggered, this, &Editor::open);
	ui.menuFile->addAction(this->openAct);
	ui.toolBar->addAction(this->openAct);

	const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(""));
	this->cutAct = new QAction(cutIcon, tr("&Cut..."), this);
	this->cutAct->setShortcuts(QKeySequence::Cut);
	this->cutAct->setStatusTip(tr("Cut text"));
	ui.menuModifica->addAction(this->cutAct);
	ui.toolBar->addAction(this->cutAct);

	const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(""));
	this->copyAct = new QAction(cutIcon, tr("&Copy..."), this);
	this->copyAct->setShortcuts(QKeySequence::Cut);
	this->copyAct->setStatusTip(tr("Copy text"));
	ui.menuModifica->addAction(this->copyAct);
	ui.toolBar->addAction(this->copyAct);

#ifndef QT_NO_PRINTER
	//const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png"));
	this->printAction = ui.menuFile->addAction(cutIcon, tr("&Print..."), this, &Editor::filePrint);
	this->printAction->setPriority(QAction::LowPriority);
	this->printAction->setShortcut(QKeySequence::Print);
	ui.toolBar->addAction(this->printAction);

	//const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png"));
	ui.menuFile->addAction(cutIcon, tr("Print Preview..."), this, &Editor::filePrintPreview);

	//const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png"));
	this->exportPDFAction = ui.menuFile->addAction(cutIcon, tr("&Export PDF..."), this, &Editor::filePrintPdf);
	this->exportPDFAction->setPriority(QAction::LowPriority);
	this->exportPDFAction->setShortcut(Qt::CTRL + Qt::Key_D);
	ui.toolBar->addAction(this->exportPDFAction);

	//menu->addSeparator();
#endif

	//const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png"));
	this->actionUndo = ui.menuModifica->addAction(cutIcon, tr("&Undo"), ui.textEdit, &QTextEdit::undo);
	this->actionUndo->setShortcut(QKeySequence::Undo);
	ui.toolBar->addAction(this->actionUndo);

	this->actionRedo = ui.menuModifica->addAction(cutIcon, tr("&Redo"), ui.textEdit, &QTextEdit::redo);
	this->actionRedo->setShortcut(QKeySequence::Redo);
	ui.toolBar->addAction(this->actionRedo);

	this->italicAct = new QAction(cutIcon, tr("&Corsivo"), this);
	this->italicAct->setCheckable(true);
	this->italicAct->setChecked(false);
	this->italicAct->setPriority(QAction::LowPriority);
	this->italicAct->setShortcut(Qt::CTRL + Qt::Key_I);
	connect(this->italicAct, &QAction::triggered, this, &Editor::makeItalic);
	ui.menuModifica->addAction(this->italicAct);
	ui.toolBar->addAction(this->italicAct);

	this->boldAct = new QAction(cutIcon, tr("&Grassetto"), this);
	this->boldAct->setCheckable(true);
	this->boldAct->setPriority(QAction::LowPriority);
	this->boldAct->setShortcut(Qt::CTRL + Qt::Key_B);
	connect(this->boldAct, &QAction::triggered, this, &Editor::makeBold);
	ui.menuModifica->addAction(this->boldAct);
	ui.toolBar->addAction(this->boldAct);

	//const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/textunder.png"));
	this->underLineAct = new QAction(cutIcon, tr("&Sottilinea"), this);
	this->underLineAct->setShortcut(Qt::CTRL + Qt::Key_U);
	this->underLineAct->setPriority(QAction::LowPriority);
	ui.menuModifica->addAction(this->underLineAct);
	ui.toolBar->addAction(this->underLineAct);
	this->underLineAct->setCheckable(true);
	connect(this->underLineAct, &QAction::triggered, this, &Editor::makeUnderlined);

	this->actionImage = new QAction(cutIcon, tr("&Inserisci immagine"), this);
	ui.menuModifica->addAction(this->actionImage);
	ui.toolBar->addAction(this->actionImage);
	connect(this->actionImage, &QAction::triggered, this, &Editor::insertImage);

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
	connect(ui.textEdit, &QTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
	connect(ui.textEdit, &QTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD

	//const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/textleft.png"));
	this->actionAlignLeft = new QAction(cutIcon, tr("&Left"), this);
	this->actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
	this->actionAlignLeft->setCheckable(true);
	this->actionAlignLeft->setPriority(QAction::LowPriority);
	//const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/textcenter.png"));
	this->actionAlignCenter = new QAction(cutIcon, tr("C&enter"), this);
	this->actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
	this->actionAlignCenter->setCheckable(true);
	this->actionAlignCenter->setPriority(QAction::LowPriority);
	//const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/textright.png"));
	this->actionAlignRight = new QAction(cutIcon, tr("&Right"), this);
	this->actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
	this->actionAlignRight->setCheckable(true);
	this->actionAlignRight->setPriority(QAction::LowPriority);
	//const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/textjustify.png"));
	this->actionAlignJustify = new QAction(cutIcon, tr("&Justify"), this);
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
	QTextCursor cursor = ui.textEdit->textCursor();
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
		ui.textEdit->mergeCurrentCharFormat(fmt);
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
	QTextCursor cursor = ui.textEdit->textCursor();
	if (!cursor.hasSelection())
		cursor.select(QTextCursor::WordUnderCursor);
	cursor.mergeCharFormat(format);
	ui.textEdit->mergeCurrentCharFormat(format);
}

void Editor::textSize(const QString& p) {
	qreal pointSize = p.toFloat();
	if (p.toFloat() > 0) {
		QTextCharFormat fmt;
		fmt.setFontPointSize(pointSize);
		this->mergeFormatOnWordOrSelection(fmt);
	}
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
	if (ui.textEdit->textCursor().hasSelection())
		dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
	dlg->setWindowTitle(tr("Print Document"));
	if (dlg->exec() == QDialog::Accepted)
		ui.textEdit->print(&printer);
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
	ui.textEdit->print(printer);
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
	ui.textEdit->document()->print(&printer);
	statusBar()->showMessage(tr("Exported \"%1\"").arg(QDir::toNativeSeparators(fileName)));
	//! [0]
#endif
}

//MATTIA-----------------------------------------------------------------------------------------------------------

void Editor::on_textEdit_textChanged() {



	//QTextCursor TC = ui.textEdit->textCursor();
	//QString temp = ui.textEdit->toPlainText();
	//int pos = TC.position();
	//char chr;
	//
	//if(TC.position()==0)
	//	chr = ui.textEdit->toPlainText().at(TC.position()).toLatin1();
	//else
	//	chr = ui.textEdit->toPlainText().at(TC.position()-1).toLatin1();
	//FormatStructure FS;
	//FS.isUnderlined = TC.charFormat().fontUnderline();
	//FS.isItalic = TC.charFormat().fontItalic();
	//FS.weight = TC.charFormat().fontWeight();
	//FS.size = TC.charFormat().fontPointSize();
	//FS.font = TC.charFormat().fontFamily();

	//Message M = this->_CRDT->localInsert(TC.position(), chr, FS);

	//il messaggio va mandato al serializzatore

	if (this->remoteEvent)
		return;
	QTextCursor TC = ui.textEdit->textCursor();
	//DEBUG
	int curr = TC.position();
	int last = lastCursor;
	if (TC.position() <= lastCursor) {
		//è una delete		
		Edelete();
	}
	else {
		//è una insert
		Einsert();
	}
	//aggiorno
	this->lastText = ui.textEdit->toPlainText();
	this->lastCursor = TC.position();
}


void Editor::Einsert() {
	QTextCursor TC = ui.textEdit->textCursor();
	int li = TC.anchor();

	//funziona sia per inserimento singolo che per inserimento multiplo--> incolla
	for (int i = lastCursor; i < TC.position(); i++) {
		if (i < 0)
			return;

		int pos = i;

		char chr = ui.textEdit->toPlainText().at(pos).toLatin1();
		QFont font = ui.textEdit->currentFont();
		QColor color = ui.textEdit->textColor();
		Qt::AlignmentFlag alignment = this->getAlignementFlag(ui.textEdit->alignment());

		Message m = this->_CRDT->localInsert(pos, chr, font, color, alignment);
		m_socketHandler->writeData(m_messageSerializer->messageSerialize(m, 0)); // -> socket

		//std::string prova = m.getSymbol().getFont().toString().toStdString();
		//std::cout << "prova" << std::endl;
	}
}

void Editor::Edelete() {

	QTextCursor TC = ui.textEdit->textCursor();

	int li = TC.position();

	for (int i = lastCursor; i > TC.position(); i--) {
		Message m = this->_CRDT->localErase(i - 1);
		//send to socket
		m_socketHandler->writeData(m_messageSerializer->messageSerialize(m, 0));  // -> socket
		return;
	}
	if (lastCursor == TC.position()) {//se sono uguale sono nel caso particolare in cui seeziono da dx a sx
									// e quando cancello il cursore non cambia
		deleteDxSx();
	}
}

void Editor::deleteDxSx() {

	QTextCursor TC = ui.textEdit->textCursor();
	int start = this->lastStart;
	int end = this->lastEnd;
	for (int i = end; i > start; i--) {
		Message m = this->_CRDT->localErase(i - 1);
		m_socketHandler->writeData(m_messageSerializer->messageSerialize(m, 0)); // -> socket
	}
}

void Editor::remoteAction(Message m)
{
	this->remoteEvent = true;//serve ad evitare che l'ontextchange venga triggerato e che quindi vada a mod il cursore e le altre var
	//provato anche con connect e disconnect del segnale 

	//inserisce auth nel CRDT
	__int64 index = this->_CRDT->process(m);//index rappresenta l'indice della lettere nel testo corrente dove fare insert/delete

	/*--------------------------------------------------------------------------------
	queste due funzioni in base al valore dell'indice a cui inserire e a quello del cursore corrente decidono
	se è il caso o meno di andare a modificare il cursore incrementandolo o decrementandolo
	++ iff this<last
	-- iff this<last
	solo se sto inserendo ad un indice inferiore a quello a cui sto srivendo poiche avrò un char in meno o in piu
	-----------------------------------------------------------------------------------------*/
	switch (m.getAction())
	{
	case INSERT:
		updateViewAfterInsert(m, index);
		maybeincrement(index);
		break;
	case DELETE:
		updateViewAfterDelete(m, index);
		maybedecrement(index);

		break;
	default:
		break;
	}
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
	QTextCursor TC = ui.textEdit->textCursor();
	lastStart = TC.selectionStart();
	lastEnd = TC.selectionEnd();

	if (this->remoteEvent)
		return;

	if (this->lastText.compare(this->ui.textEdit->toPlainText()))//aggiorno solo se non è delete or insert
		return;


	int in = TC.position();
	int l = lastCursor;
	this->lastCursor = TC.position();
	//emit per dire che mi sono spostato-->aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

}

void Editor::updateViewAfterInsert(Message m, __int64 index)
{
	disconnect(ui.textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
	//retrieving remote state
	QChar chr = m.getSymbol().getChar();
	QFont r_font = m.getSymbol().getFont();
	QColor r_color = m.getSymbol().getColor();
	Qt::AlignmentFlag alignment = m.getSymbol().getAlignment();
	/// 
	/// la parte qui sotto potrbbe essere inutile per poter scrivere sul text editor 
	/// conviene usare la Qchartextedit--> vedi nota vocale su telegram a me stesso
	//o vedere changeViewAfterInsert in mainwindow.cpp debora

	QTextCursor TC = ui.textEdit->textCursor();
	// saving current state
	QFont font = ui.textEdit->currentFont();
	QColor color = ui.textEdit->textColor();

	//colore del char che arriva
	QTextCharFormat format;
	format.setFont(r_font);
	format.setForeground(r_color);
	//ui.textEdit->setFont(r_font);
	//ui.textEdit->setTextColor(r_color);

	TC.setPosition(index);
	TC.insertText(chr, format);

	QTextBlockFormat blockFormat = TC.blockFormat();
	blockFormat.setAlignment(alignment);

	TC.mergeBlockFormat(blockFormat);


	//ripristino
	TC.setPosition(this->lastCursor);
	//ui.textEdit->setFont(font);
	//ui.textEdit->setTextColor(color);
	connect(ui.textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
}

void Editor::updateViewAfterDelete(Message m, __int64 index)
{
	disconnect(ui.textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
	QTextCursor TC = ui.textEdit->textCursor();
	TC.setPosition(index);
	TC.deleteChar();
	//TC.deletePreviousChar();//oppure è questo se il primo non funziona
	TC.setPosition(this->lastCursor);
	connect(ui.textEdit, SIGNAL(textChanged()), this, SLOT(on_textEdit_textChanged()));
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
	//	//editor insert-->crea una funzione che fa quello che per ora è in texchanged
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
		QTextDocument* textDocument = ui.textEdit->document();
		textDocument->addResource(QTextDocument::ImageResource, uri, QVariant(image));
		QTextCursor cursor = ui.textEdit->textCursor();
		QTextImageFormat imageFormat;
		imageFormat.setWidth(image.width());
		imageFormat.setHeight(image.height());
		imageFormat.setName(uri.toString());
		cursor.insertImage(imageFormat);
		QString imageSerialized = m_messageSerializer->imageSerialize(image, 2);
		//bisogna aggiungere anche la posizione del cursore
	}
}

void Editor::textAlign(QAction* a) {
	if (a == actionAlignLeft)
		this->ui.textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
	else if (a == actionAlignCenter)
		this->ui.textEdit->setAlignment(Qt::AlignHCenter);
	else if (a == actionAlignRight)
		this->ui.textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
	else if (a == actionAlignJustify)
		this->ui.textEdit->setAlignment(Qt::AlignJustify);
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
	this->alignmentChanged(ui.textEdit->alignment());
	QTextCursor TC = ui.textEdit->textCursor();
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
		int headingLevel = this->ui.textEdit->textCursor().blockFormat().headingLevel();
		this->comboStyle->setCurrentIndex(headingLevel ? headingLevel + 8 : 0);
	}

	this->comboFont->setCurrentFont(ui.textEdit->currentFont());
	QPixmap pix(16, 16);
	pix.fill(ui.textEdit->textColor());
	this->actionTextColor->setIcon(pix);
	const QList<int> standardSizes = QFontDatabase::standardSizes();
	int size = ui.textEdit->font().pointSize();
	QString t = TC.selectedText();
	this->comboSize->setCurrentIndex(standardSizes.indexOf(size));
}

void Editor::colorChanged(const QColor& c) {
	QPixmap pix(16, 16);
	pix.fill(c);
	this->actionTextColor->setIcon(pix);
}

void Editor::textColor()
{
	QColor col = QColorDialog::getColor(ui.textEdit->textColor(), this);
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
	Message m = m_messageSerializer->messageUnserialize(packet);
	remoteAction(m);
}