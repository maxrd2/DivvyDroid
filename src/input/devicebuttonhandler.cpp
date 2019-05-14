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
#include "devicebuttonhandler.h"

#include "device/deviceinfo.h"
#include "device/adbclient.h"
#include "input/input_event_codes.h"
#include "input/input_to_adroid_keys.h"

#include <QMouseEvent>

DeviceButtonHandler::DeviceButtonHandler(QObject *parent)
	: InputHandler(parent),
	  m_useDevice(true)
{
}

DeviceButtonHandler::~DeviceButtonHandler()
{

}

bool
DeviceButtonHandler::init(int deviceNr, const WidgetKeyMap &keyMap)
{
	if(deviceNr == -1 || !InputHandler::init() || !m_adb.send(QByteArray("dev:").append(INPUT_DEV_PATH).append(QString::number(deviceNr)))) {
		qDebug() << __FUNCTION__ << "failed opening device" << deviceNr << "will send button events using fallback";
		m_useDevice = false;
	}

	m_keyMap = keyMap;
	for(QObject *obj : m_keyMap.keys())
		obj->installEventFilter(this);

	return true;
}

bool
DeviceButtonHandler::eventFilter(QObject *obj, QEvent *ev)
{
	if(!m_keyMap.contains(obj))
		return false;

	const quint16 keyCode = m_keyMap[obj];

	switch(ev->type()) {
	case QEvent::MouseButtonPress: {
		qDebug() << "KEY DOWN" << keyCode;
		if(m_useDevice) {
			m_adb.sendEvents(AdbEventList()
					<< AdbEvent(EV_KEY, keyCode, 1)
					<< AdbEvent(EV_SYN));
		} else {
			m_pressTime.start();
		}
		return true;
	}
	case QEvent::MouseButtonRelease: {
		qDebug() << "KEY UP" << keyCode;
		if(m_useDevice) {
			m_adb.sendEvents(AdbEventList()
					<< AdbEvent(EV_KEY, keyCode, 0)
					<< AdbEvent(EV_SYN));
		} else {
			QByteArray cmd("input keyevent ");
			if(m_pressTime.elapsed() > 600)
				cmd.append("--longpress ");
			cmd.append(QString::number(keyToAndroidCode[keyCode]));
			AdbClient::shell(cmd);
		}
		return true;
	}

	default:
		return false;
	}

	return false;
}
