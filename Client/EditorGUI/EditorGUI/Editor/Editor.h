#pragma once

#include "ui_Editor.h"
#include <QMainWindow>
#include "CRDT/CRDT.h"
#include <QKeyEvent>
#include <Qt>
#include <iostream>
#include <QFontComboBox>
#include <QtWidgets>
#include "Structures/FormatStructure.h"
#include "SocketHandler/SocketHandler.h"
#include "Serialization/Serialize.h"
#include "MyTextEdit.h"
#include "UserInterval.h"

class QFontComboBox;
class QPrinter;
//class QTextEdit;

class Editor : public QMainWindow, public Ui::Editor
{
	Q_OBJECT

public:
	Editor(QSharedPointer<SocketHandler> socketHandler, QSharedPointer<QPixmap> profileImage, QColor userColor,
		QString path = "", QString username = "", int fileId = 0, int clientID = 0, QWidget* parent = Q_NULLPTR);
	~Editor();
	void loadFile(const QString& fileName);
	void remoteAction(Message m);
	int getFileId();
	void addEditingUser(QStringList userInfo);
	void removeEditingUser(int id);
	void setSiteCounter(int siteCounter);

private:
	Ui::Editor ui;
	MyTextEdit* m_textEdit;
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
	QAction* m_actionShowEditingUsers;
	QAction* m_showUsersIntervals;
	QFontComboBox* comboFont;
	QComboBox* comboStyle;
	QComboBox* comboSize;
	QListWidget* m_editingUsersList;
	QSharedPointer<SocketHandler> m_socketHandler;
	QSharedPointer<QPixmap> m_profileImage;
	QTimer* m_timer;
	QLabel* m_usernameLabel;
	QString m_username;
	int selectionStart, selectionEnd, flagItalic = 0, changeItalic = 0;
	int m_fileId;
	QMap<int, QString> m_editingUsers;
	bool m_showingEditingUsers;
	QColor m_userColor;

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

	//serve ad impedire che l'ontextchange venga triggerato due volte di seguito quando ho cami di stile
	bool styleBounce = false;

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
	bool isAKeySequence(QKeyEvent*e);
	//void deleteDxSx();//caso particolare per la delete con selezione--> sfrutto last start e last end-->solved


	void maybeincrement(__int64 index);
	void maybedecrement(__int64 index);
	Qt::AlignmentFlag getAlignementFlag(Qt::Alignment a);
	void updateViewAfterInsert(Message m, __int64 index);
	void updateViewAfterDelete(Message m, __int64 index);
	void updateViewAfterStyleChange(Message m, __int64 index);

	//FINE----------------------------------------------------------------------
	
	void initialFileLoad(Message m, __int64 index);

protected:
		void mousePressEvent(QMouseEvent* event);

public slots:
	void keyPressEvent(int e);
	void keyRelaseEvent(QKeyEvent* e);
	void tastoPremuto(QKeyEvent* e);

private slots:
	void on_textEdit_textChanged();
	void on_textEdit_cursorPositionChanged();
	void textColor();
	void showEditingUsers();
	void updateCursorPosition(bool isSelection);
	void showHideUsersIntervals();

//---------------------------------------------------------------------------------------------------
signals:
	void editorClosed(int, int);
	void styleChange();
	void showHideUsersIntervalsSignal();
	void updateUsersIntervals();
};
