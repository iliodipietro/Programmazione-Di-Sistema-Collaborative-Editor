#pragma once

#include <QMainWindow>
#include "ui_Editor.h"
#include "CRDT/CRDT.h"
#include "../Structures/FormatStructure.h"
#include <QKeyEvent>
class QFontComboBox;
class QPrinter;
//class QTextEdit;

class Editor : public QMainWindow, public Ui::Editor
{
	Q_OBJECT

public:
	Editor(QWidget *parent = Q_NULLPTR, QString path = "");
	~Editor();
	void loadFile(const QString& fileName);

private:
	Ui::Editor ui;
	QWidget *parent;
	QString filePath, curFile;
	QAction* italicAct;
	QAction* openAct;
	QAction* cutAct;
	QAction* copyAct;
	QAction* boldAct;
	QAction* underLineAct;
	QAction* printAction;
	QAction* exportPDFAction;
	QAction* actionUndo;
	QAction* actionRedo;
	QAction* actionImage;
	QAction* actionAlignLeft;
	QAction* actionAlignRight;
	QAction* actionAlignCenter;
	QAction* actionAlignJustify;
	QAction* actionTextColor;
	QFontComboBox* comboFont;
	QComboBox* comboStyle;
	QComboBox* comboSize;
	int selectionStart, selectionEnd, flagItalic = 0, changeItalic = 0;

	//MATTIA---------------------------------------------------------------------------------
	CRDT* _CRDT;

	int ID;//è quello dato dal server--> da dare al crdt come id per crere i caratteri univoci

	QString lastText;/*usato per capire quando mi sposto solamento o il testo varia
					 ossia se il testo corrente è uguale a quello corrente ho solo mosso il cursore*/

	int lastCursor;//ultima posizione del cursore

	bool remoteEvent;/*flag da settare quando mi arriva un messaggio prima di inserire nell'editor per evitare che
					 venga triggerato l'on_text_change*/


					 //servono a mantenere gli estremi di quando seleziono --> caso particolare nella delete
	int lastStart;
	int lastEnd;

	//FINE-------------------------------------------------------------------------------------------------------

	void closeEvent(QCloseEvent* event);
	void createActions();
	void open();
	void setCurrentFile(const QString& fileName);
	bool maybeSave();
	void makeItalic();
	void makeBold();
	void makeUnderlined();
	void textStyle(int style);
	void textSize(const QString& p);
	void textFamily(const QString& f);
	void mergeFormatOnWordOrSelection(const QTextCharFormat& format);
	void filePrint();
	void filePrintPreview();
	void printPreview(QPrinter* printer);
	void filePrintPdf();
	void insertImage();
	void textAlign(QAction* a);
	void alignmentChanged(Qt::Alignment a);
	void colorChanged(const QColor& c);

	//Mattia-----------------------------------------------------------------------------------------------------------

	void localInsert();//Editor Local insert
	void localDelete();//Editor local delete
	void localStyleChange();//Editor local style change
	void updateLastPosition();
	//void deleteDxSx();//caso particolare per la delete con selezione--> sfrutto last start e last end-->solved

	void remoteAction(Message m);
	void maybeincrement(__int64 index);
	void maybedecrement(__int64 index);
	Qt::AlignmentFlag getAlignementFlag(Qt::Alignment a);
	void updateViewAfterInsert(Message m, __int64 index);
	void updateViewAfterDelete(Message m, __int64 index);
	void updateViewAfterStyleChange(Message m, __int64 index);

	//FINE----------------------------------------------------------------------
protected:
	void keyPressEvent(QKeyEvent *e);

private slots:
	void on_textEdit_textChanged();
	void on_textEdit_cursorPositionChanged();
	void textColor();

//---------------------------------------------------------------------------------------------------

};
