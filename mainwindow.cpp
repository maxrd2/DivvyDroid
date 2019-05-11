/*
   DivvyDroid - Application to screencast and remote control Android devices.

   Copyright (C) 2019 - Mladen Milinkovic <maxrd2@smoothware.net>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "device/deviceinfo.h"
#include "input/devicetouchhandler.h"
#include "input/devicebuttonhandler.h"
#include "input/shellkeyboardhandler.h"
#include "input/monkeyhandler.h"

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
	ui->screen->setFocusPolicy(Qt::StrongFocus);
	ui->screen->setFocus();
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

	// init monkey
	MonkeyHandler *monkeyHandler = new MonkeyHandler(this);
	const bool monkeyOk = monkeyHandler->init(WidgetKeyMap{
			   { ui->btnHome, KEY_HOMEPAGE },
			   { ui->btnMenu, KEY_MENU },
			   { ui->btnBack, KEY_BACK },
			   { ui->btnVolumeDown, KEY_VOLUMEDOWN },
			   { ui->btnVolumeUp, KEY_VOLUMEUP },
			   { ui->btnPower, KEY_POWER },
			   { ui->screen, BTN_TOUCH }});
	if(!monkeyOk) {
		DeviceInfo::initInput();
		// init keyboard
		ShellKeyboardHandler *keyboardHandler = new ShellKeyboardHandler(this);
		if(keyboardHandler->init())
			ui->screen->installEventFilter(keyboardHandler);

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
	}

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
