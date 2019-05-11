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
#include "monkeyhandler.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QHostAddress>
#include <QThread>
#include <QCoreApplication>
#include <QWidget>
#include <QDebug>

#include "device/deviceinfo.h"
#include "input/input_to_adroid_keys.h"
#include "input/android_keycodes.h"

MonkeyHandler::MonkeyHandler(QObject *parent)
	: InputHandler(parent),
	  m_inputMouseDown(false),
	  m_lastTouchId(33)
{
}

MonkeyHandler::~MonkeyHandler()
{
	m_monkey.write("quit\n");
}

bool
MonkeyHandler::init(const WidgetKeyMap &keyMap)
{
	m_keyMap = keyMap;

	// start monkey daemon
	if(!m_shell.connectToDevice())
		return false;
	if(!m_shell.send(QByteArray("shell:monkey --port 33333")))
		return false;

	disconnect(&m_shell, &AdbClient::readyRead, nullptr, nullptr);
	connect(&m_shell, &AdbClient::readyRead, [&](){
		qDebug() << "MONKEYHANDLER monkey:" << m_shell.readLine().trimmed();
	});

	// wait a bit so monkey daemon has time to start
	for(int i = 0; i < 16; i++) {
		QThread::msleep(100);
		QCoreApplication::processEvents();
	}

	// connect to device's tcp
	if(!m_monkey.connectToDevice())
		return false;
	if(!m_monkey.send(QByteArray("tcp:33333"))) {
		m_monkey.close();
		if(!m_monkey.connectToDevice())
			return false;
		if(!m_monkey.send(QByteArray("shell:telnet ::1 33333")))
			return false;
	}
	disconnect(&m_monkey, &AdbClient::readyRead, nullptr, nullptr);
	connect(&m_monkey, &AdbClient::readyRead, [&](){
		const QByteArray res = m_monkey.readLine().trimmed();
		if(res != "OK")
			qDebug() << "MONKEYHANDLER tcp:" << res;
	});

	for(QObject *obj : m_keyMap.keys())
		obj->installEventFilter(this);

	return true;
}

bool
MonkeyHandler::eventFilter(QObject *obj, QEvent *ev)
{
	const quint16 keyCode = m_keyMap[obj];

	QWidget *widget = reinterpret_cast<QWidget *>(obj);

	QMouseEvent *mev = reinterpret_cast<QMouseEvent *>(ev);
	QKeyEvent *kev = reinterpret_cast<QKeyEvent *>(ev);

	switch(ev->type()) {
	case QEvent::MouseButtonPress:
		if(keyCode == BTN_TOUCH) {
			m_inputMouseDown = true;
			const int x = mev->x() * aDev->screenWidth() / widget->width();
			const int y = mev->y() * aDev->screenHeight() / widget->height();
			qDebug() << "MOUSE DOWN" << x << "," << y;
			m_monkey.write(QByteArray("touch down ")
						   .append(QString::number(x)).append(' ')
						   .append(QString::number(y)).append("\n"));
		} else {
			qDebug() << "KEY DOWN" << keyCode;
			m_monkey.write(QByteArray("key down ").append(QString::number(keyToAndroidCode[keyCode])).append("\n"));
		}
		return true;

	case QEvent::MouseButtonRelease:
		if(keyCode == BTN_TOUCH) {
			m_inputMouseDown = false;
			const int x = mev->x() * aDev->screenWidth() / widget->width();
			const int y = mev->y() * aDev->screenHeight() / widget->height();
			qDebug() << "MOUSE UP" << x << "," << y;
			m_monkey.write(QByteArray("touch up ")
						   .append(QString::number(x)).append(' ')
						   .append(QString::number(y)).append("\n"));
		} else {
			qDebug() << "KEY UP" << keyCode;
			m_monkey.write(QByteArray("key up ").append(QString::number(keyToAndroidCode[keyCode])).append("\n"));
		}
		return true;

	case QEvent::MouseMove:
		if(m_inputMouseDown) {
			const int x = mev->x() * aDev->screenWidth() / widget->width();
			const int y = mev->y() * aDev->screenHeight() / widget->height();
			qDebug() << "MOUSE MOVE" << x << "," << y;
			m_monkey.write(QByteArray("touch move ")
						   .append(QString::number(x)).append(' ')
						   .append(QString::number(y)).append("\n"));
			return true;
		}
		break;

	case QEvent::KeyPress: {
		const auto key = qtToAndroidCode.find(Qt::Key(kev->key()));
		if(key == qtToAndroidCode.cend())
			return false;
		m_monkey.write(QByteArray("key down ").append(QString::number(key.value())).append("\n"));
		return true;
	}
	case QEvent::KeyRelease: {
		const auto key = qtToAndroidCode.find(Qt::Key(kev->key()));
		if(key != qtToAndroidCode.cend())
			m_monkey.write(QByteArray("key up ").append(QString::number(key.value())).append("\n"));
		else
			m_monkey.write(QByteArray("type ").append(kev->text().toLatin1()).append("\n"));
		return true;
	}
	default:
		return false;
	}


	return false;
}
