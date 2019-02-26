#include "mainwindow.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QSignalTransition>
#include <QDebug>

namespace poscalc {
	MainWindow::MainWindow() {
		setWindowTitle( tr( "Position Calculator" ) );

		auto screenRect = QApplication::desktop()->screenGeometry();

		int wnd_width   = 480;
		int wnd_height  = 640;

		// center on screen
		int wnd_x       = ( screenRect.width() / 2 ) - ( wnd_width / 2 );
		int wnd_y       = ( screenRect.height() / 2 ) - ( wnd_height / 2 );

		setGeometry( wnd_x, wnd_y, wnd_width, wnd_height );
		setMinimumSize( wnd_width, wnd_height );

		// setCentralWidget( m_main_widget );
	}
};