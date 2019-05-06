#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QElapsedTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow),
	  m_screenTimer(this),
	  m_shell(this)
{
	ui->setupUi(this);

	connect(&m_screenTimer, &QTimer::timeout, [&](){
		QElapsedTimer timer;
		timer.start();
		AdbShell::Res res = m_shell.execute("/system/bin/screencap -j");
		qDebug() << "SCREEN frame retrieved in" << timer.elapsed() << "ms";
		QImage img(QImage::fromData(res.out).scaledToWidth(360, Qt::SmoothTransformation));
		ui->screen->setPixmap(QPixmap::fromImage(img));
	});
	m_screenTimer.start(40);
}

MainWindow::~MainWindow()
{
	delete ui;
}
