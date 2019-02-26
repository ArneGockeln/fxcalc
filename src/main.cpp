#include <iostream>

#include <QtCore>
#include <QApplication>
#include <QFile>
#include <QString>
#include <QFontDatabase>
#include <QDebug>

#include "mainwindow.h"

int main(int argc, char *argv[])
{	
	// init
	Q_INIT_RESOURCE( resources );
	QApplication app(argc, argv);

	// load main window
	poscalc::MainWindow wnd;
	wnd.show();

	return app.exec();
}