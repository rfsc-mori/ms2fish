#include "mainwindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	MainWindow w;
	w.show();

	return a.exec();
}

// space = up 3-5 px
// y < 90 red, top
// 90 <= y < 130 blue, middle
// 130 <= x red, bottom
