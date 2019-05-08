#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "device/deviceinfo.h"
#include "input/devicetouchhandler.h"
#include "input/devicebuttonhandler.h"

#include <iostream>
#include <cstdarg>

#include <QMouseEvent>
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

	// make window fixed size
	QPixmap img(IMAGE_WIDTH, IMAGE_WIDTH * aDev->screenHeight() / aDev->screenWidth());
	img.fill(Qt::black);
	ui->screen->setPixmap(img);
	adjustSize();
	setFixedSize(sizeHint());

	// video thread
	m_videoThread = new VideoThread();
	connect(m_videoThread, &VideoThread::imageReady, this, &MainWindow::updateScreen);
	m_videoThread->start();

	// init touch
	DeviceTouchHandler *touchHandler = new DeviceTouchHandler(this);
	if(touchHandler->init())
		ui->screen->installEventFilter(touchHandler);

	// init bottom keys
	(new DeviceButtonHandler(this))->init(
		aDev->inputHome(),
		WidgetKeyMap{{ ui->btnHome, KEY_HOMEPAGE }});
	(new DeviceButtonHandler(this))->init(
		aDev->inputBack(),
		WidgetKeyMap{{ ui->btnMenu, KEY_MENU }, { ui->btnBack, KEY_BACK }});

	// init volume keys
	(new DeviceButtonHandler(this))->init(
		aDev->inputVolume(),
		WidgetKeyMap{{ ui->btnVolumeDown, KEY_VOLUMEDOWN }, { ui->btnVolumeUp, KEY_VOLUMEUP }});

	// init power key
	(new DeviceButtonHandler(this))->init(
		aDev->inputPower(),
		WidgetKeyMap{{ ui->btnPower, KEY_POWER }});

	// init unlock key
	connect(ui->btnUnlock, &QPushButton::clicked, [&](){
		if(!aDev->isScreenAwake()) {
			const QPoint pos(ui->btnPower->geometry().center());
			QCoreApplication::sendEvent(ui->btnPower,
				new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
			QCoreApplication::sendEvent(ui->btnPower,
				new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
		}
		AdbClient::shell("wm dismiss-keyguard");
	});
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
