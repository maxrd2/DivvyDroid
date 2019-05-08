#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QFile file(QStringLiteral(":/divvydroid.qss"));
    file.open(QFile::ReadOnly);
    a.setStyleSheet(file.readAll());

	MainWindow w;
	w.show();
	w.init();

	return a.exec();
}
