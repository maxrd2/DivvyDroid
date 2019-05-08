#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "deviceinfo.h"
#include "devicetouchhandler.h"

#include <iostream>
#include <cstdarg>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow),
	  m_videoThread(nullptr)
{
	ui->setupUi(this);
}

void
MainWindow::init()
{
	// start adb server and wait for device
	if(!DeviceInfo::waitForDevice()) {
		QApplication::quit();
		return;
	}

//	DeviceList devices = DeviceInfo::deviceList();
//	DeviceInfo::connect(devices.firstKey());
	DeviceInfo::connect();

	// video thread
	m_videoThread = new VideoThread();
	connect(m_videoThread, &VideoThread::imageReady, this, &MainWindow::updateScreen);
	m_videoThread->start();

	// init touch
	DeviceTouchHandler *touchHandler = new DeviceTouchHandler(this);
	if(touchHandler->init())
		ui->screen->installEventFilter(touchHandler);

	// wake up
/*
	AdbClient::sendEvents(m_keyDevice[KEY_POWER], AdbEventList()
						  << AdbEvent(EV_KEY, KEY_POWER, 1)
						  << AdbEvent(EV_SYN)
						  << AdbEvent(EV_KEY, KEY_POWER, 0)
						  << AdbEvent(EV_SYN));
*/
}

void
MainWindow::updateScreen(const QImage &image)
{
	ui->screen->setPixmap(QPixmap::fromImage(image));
}

MainWindow::~MainWindow()
{
	delete ui;
}
