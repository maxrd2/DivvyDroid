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

AdbClient MonkeyHandler::m_daemon;
bool MonkeyHandler::m_daemonReady = false;

MonkeyHandler::MonkeyHandler(QObject *parent)
	: InputHandler(parent),
	  m_inputMouseDown(false)
{
	connect(&m_wheelTimer, &QTimer::timeout, this, &MonkeyHandler::sendWheelEvents);
	m_wheelTimer.setSingleShot(true);
}

MonkeyHandler::~MonkeyHandler()
{
	m_monkey.write("quit\n");
	m_monkey.waitForDisconnected();
}

/*static*/ void
MonkeyHandler::initDaemon()
{
	m_daemonReady = false;

	// kill existing monkey daemons
	AdbClient::shell("kill $(pidof com.android.commands.monkey)");

	if(!m_daemon.connectToDevice())
		return;
	if(!m_daemon.send(QByteArray("shell:monkey --port 33333")))
		return;

	disconnect(&m_daemon, &AdbClient::readyRead, nullptr, nullptr);
	connect(&m_daemon, &AdbClient::readyRead, [&](){
		qDebug() << "MONKEYHANDLER daemon:" << m_daemon.readLine().trimmed();
	});

	m_daemonReady = true;
}

bool
MonkeyHandler::init(const WidgetKeyMap &keyMap)
{
	m_keyMap = keyMap;

	if(!m_daemonReady)
		return false;

	// wait a bit so monkey daemon has time to start
	int timeout = 3000;
	while(timeout > 0) {
		QThread::msleep(300);
		QCoreApplication::processEvents();
		timeout -= 300;

		// connect to device's tcp
		if(!m_monkey.connectToDevice())
			return false;
		if(m_monkey.send(QByteArray("tcp:33333")))
			break;
		m_monkey.close();

		// connect using IPv6
		if(!m_monkey.connectToDevice())
			return false;
		if(m_monkey.send(QByteArray("shell:telnet ::1 33333 ; echo OK"))) {
			if(!m_monkey.waitForReadyRead(200)) {
				// read timed out - telnet is connected :)
				break;
			}
		}
		m_monkey.close();
	}
	if(!m_monkey.isConnected())
		return false;

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

void
MonkeyHandler::sendWheelEvents()
{
	qDebug() << "MOUSE SCROLL UP" << m_wheelX << "," << m_wheelY;
	m_monkey.write(QByteArray("touch up ")
				   .append(QString::number(m_wheelX)).append(' ')
				   .append(QString::number(m_wheelY)).append("\n"));
}

bool
MonkeyHandler::eventFilter(QObject *obj, QEvent *ev)
{
	const quint16 keyCode = m_keyMap[obj];

	QWidget *widget = reinterpret_cast<QWidget *>(obj);

	QMouseEvent *mev = reinterpret_cast<QMouseEvent *>(ev);
	QKeyEvent *kev = reinterpret_cast<QKeyEvent *>(ev);

	switch(ev->type()) {
	case QEvent::Wheel:
		if(keyCode == BTN_TOUCH) {
			const QWheelEvent *wev = reinterpret_cast<QWheelEvent *>(ev);
			QPoint delta = wev->angleDelta() / 8;
			if(delta.isNull())
					delta = wev->pixelDelta();
			if(delta.isNull())
					break;

			if(!m_wheelTimer.isActive()) {
				m_wheelX = wev->x() * aDev->screenWidth() / widget->width();
				m_wheelY = wev->y() * aDev->screenHeight() / widget->height();
			}

			if(!m_wheelTimer.isActive()) {
				qDebug() << "MOUSE SCROLL DOWN" << m_wheelX << "," << m_wheelY;
				m_monkey.write(QByteArray("touch down ")
							   .append(QString::number(m_wheelX)).append(' ')
							   .append(QString::number(m_wheelY)).append("\n"));
			} else {
				qDebug() << "MOUSE SCROLL MOVE" << m_wheelX << "," << m_wheelY;
				m_monkey.write(QByteArray("touch move ")
							   .append(QString::number(m_wheelX)).append(' ')
							   .append(QString::number(m_wheelY)).append("\n"));
			}

			m_wheelY += 8 * delta.ry();

			m_wheelTimer.start(150);
			return true;
		}
		break;
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
