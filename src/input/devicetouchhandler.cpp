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
#include "devicetouchhandler.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QWidget>
#include <QDebug>

#include "device/deviceinfo.h"

DeviceTouchHandler::DeviceTouchHandler(QObject *parent)
	: InputHandler(parent),
	  m_inputMouseDown(false),
	  m_lastTouchId(33)
{
	connect(&m_wheelTimer, &QTimer::timeout, this, &DeviceTouchHandler::sendWheelEvents);
	m_wheelTimer.setSingleShot(true);
}

DeviceTouchHandler::~DeviceTouchHandler()
{

}

bool
DeviceTouchHandler::init()
{
	if(!InputHandler::init())
		return false;

	m_adb.connectToDevice();
	if(!m_adb.send(QByteArray("dev:").append(INPUT_DEV_PATH).append(QString::number(aDev->inputTouch())))) {
		qDebug() << __FUNCTION__ << "failed opening device" << aDev->inputTouch() << "will send touch events using fallback";
		return false;
	}

	return true;
}

void
DeviceTouchHandler::sendWheelEvents()
{
	qDebug() << "MOUSE SCROLL UP" << m_wheelX << "," << m_wheelY;
	m_adb.sendEvents(AdbEventList()
			<< AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, m_lastTouchId)
			<< AdbEvent(EV_ABS, ABS_MT_POSITION_X, m_wheelX)
			<< AdbEvent(EV_ABS, ABS_MT_POSITION_Y, m_wheelY)
			<< AdbEvent(EV_ABS, ABS_X, m_wheelX)
			<< AdbEvent(EV_ABS, ABS_Y, m_wheelY)
			<< AdbEvent(EV_SYN)
			<< AdbEvent(EV_KEY, BTN_TOUCH, 0)
			<< AdbEvent(EV_SYN));
}

bool
DeviceTouchHandler::eventFilter(QObject *obj, QEvent *ev)
{
	Q_ASSERT(obj->objectName() == QStringLiteral("screen"));

	const QWidget *screen = reinterpret_cast<QWidget *>(obj);
	const QMouseEvent *mev = reinterpret_cast<QMouseEvent *>(ev);

	switch(ev->type()) {
	case QEvent::Wheel: {
		const QWheelEvent *wev = reinterpret_cast<QWheelEvent *>(ev);
		QPoint delta = wev->angleDelta() / 8;
		if(delta.isNull())
				delta = wev->pixelDelta();
		if(delta.isNull())
				break;

		if(!m_wheelTimer.isActive()) {
			m_wheelX = wev->x() * aDev->screenWidth() / screen->width();
			m_wheelY = wev->y() * aDev->screenHeight() / screen->height();
		}

		if(!m_wheelTimer.isActive()) {
			qDebug() << "MOUSE SCROLL DOWN" << m_wheelX << "," << m_wheelY;
			m_adb.sendEvents(AdbEventList()
					<< AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, ++m_lastTouchId)
					<< AdbEvent(EV_ABS, ABS_MT_PRESSURE, 40)
					<< AdbEvent(EV_ABS, ABS_MT_DISTANCE, 0)
					<< AdbEvent(EV_ABS, ABS_MT_TOUCH_MAJOR, 1)
					<< AdbEvent(EV_ABS, ABS_MT_WIDTH_MAJOR, 10)
					<< AdbEvent(EV_ABS, ABS_MT_POSITION_X, m_wheelX)
					<< AdbEvent(EV_ABS, ABS_MT_POSITION_Y, m_wheelY)
					<< AdbEvent(EV_ABS, ABS_X, m_wheelX)
					<< AdbEvent(EV_ABS, ABS_Y, m_wheelY)
					<< AdbEvent(EV_KEY, BTN_TOUCH, 1)
					<< AdbEvent(EV_SYN));
		} else {
			qDebug() << "MOUSE SCROLL MOVE" << m_wheelX << "," << m_wheelY;
			m_adb.sendEvents(AdbEventList()
					<< AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, m_lastTouchId)
					<< AdbEvent(EV_ABS, ABS_MT_POSITION_X, m_wheelX)
					<< AdbEvent(EV_ABS, ABS_MT_POSITION_Y, m_wheelY)
					<< AdbEvent(EV_ABS, ABS_X, m_wheelX)
					<< AdbEvent(EV_ABS, ABS_Y, m_wheelY)
					<< AdbEvent(EV_SYN));
		}

		m_wheelY += 8 * delta.ry();

		m_wheelTimer.start(150);
		return true;
	}
	case QEvent::MouseButtonPress: {
		m_inputMouseDown = true;
		const int x = mev->x() * aDev->screenWidth() / screen->width();
		const int y = mev->y() * aDev->screenHeight() / screen->height();
		qDebug() << "MOUSE DOWN" << x << "," << y;
		m_adb.sendEvents(AdbEventList()
				<< AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, ++m_lastTouchId)
				<< AdbEvent(EV_ABS, ABS_MT_PRESSURE, 40)
				<< AdbEvent(EV_ABS, ABS_MT_DISTANCE, 0)
				<< AdbEvent(EV_ABS, ABS_MT_TOUCH_MAJOR, 1)
				<< AdbEvent(EV_ABS, ABS_MT_WIDTH_MAJOR, 10)
				<< AdbEvent(EV_ABS, ABS_MT_POSITION_X, x)
				<< AdbEvent(EV_ABS, ABS_MT_POSITION_Y, y)
				<< AdbEvent(EV_ABS, ABS_X, x)
				<< AdbEvent(EV_ABS, ABS_Y, y)
				<< AdbEvent(EV_KEY, BTN_TOUCH, 1)
				<< AdbEvent(EV_SYN));
		return true;
	}
	case QEvent::MouseButtonRelease: {
		m_inputMouseDown = false;
		const int x = mev->x() * aDev->screenWidth() / screen->width();
		const int y = mev->y() * aDev->screenHeight() / screen->height();
		qDebug() << "MOUSE UP" << x << "," << y;
		m_adb.sendEvents(AdbEventList()
				<< AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, m_lastTouchId)
				<< AdbEvent(EV_ABS, ABS_MT_POSITION_X, x)
				<< AdbEvent(EV_ABS, ABS_MT_POSITION_Y, y)
				<< AdbEvent(EV_ABS, ABS_X, x)
				<< AdbEvent(EV_ABS, ABS_Y, y)
				<< AdbEvent(EV_SYN)
				<< AdbEvent(EV_KEY, BTN_TOUCH, 0)
				<< AdbEvent(EV_SYN));
		return true;
	}
	case QEvent::MouseMove:
		if(m_inputMouseDown) {
			const int x = mev->x() * aDev->screenWidth() / screen->width();
			const int y = mev->y() * aDev->screenHeight() / screen->height();
			qDebug() << "MOUSE MOVE" << x << "," << y;
			m_adb.sendEvents(AdbEventList()
					<< AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, m_lastTouchId)
					<< AdbEvent(EV_ABS, ABS_MT_POSITION_X, x)
					<< AdbEvent(EV_ABS, ABS_MT_POSITION_Y, y)
					<< AdbEvent(EV_ABS, ABS_X, x)
					<< AdbEvent(EV_ABS, ABS_Y, y)
					<< AdbEvent(EV_SYN));
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}
