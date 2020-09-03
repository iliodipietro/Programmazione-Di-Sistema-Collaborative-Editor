#include "Login/Login.h"
#include <QtWidgets/QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false);
	Login w;
	w.show();
	return a.exec();
}
