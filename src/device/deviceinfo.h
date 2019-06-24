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
#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QStringList>
#include <QMap>
#include <QByteArray>

#include "input/input_event_codes.h"
//#include <linux/input.h>

#define INPUT_SYS_PATH "/sys/class/input/input"
#define INPUT_DEV_PATH "/dev/input/event"
#define HAS_BIT(v, b) (v & (1 << b))

typedef QMap<QByteArray, QByteArray> DeviceList;

class DeviceInfo
{
public:
	inline const QString & deviceId() const { return m_deviceId; }

	inline const QString androidVersion() const { return m_androidVer; }
	inline bool isArch64() const { return m_arch64; }

	bool isScreenAwake() const;
	inline quint32 screenWidth() const { return m_screenWidth; }
	inline quint32 screenHeight() const { return m_screenHeight; }
	inline quint32 screenRotation() const { return m_screenRotation; }

	inline int inputTouch() const { return m_inputTouch; }
	inline int inputPower() const { return m_inputPower; }
	inline int inputHome() const { return m_inputHome; }
	inline int inputBack() const { return m_inputBack; }
	inline int inputVolume() const { return m_inputVolume; }

	static DeviceList deviceList();
	static void connect(const char *deviceId = nullptr);
	static void initInput();
	static bool waitForDevice();
protected:
	DeviceInfo(const char *deviceId = nullptr);

private:
	QString m_deviceId;
	QString m_androidVer;
	bool m_arch64;
	int m_inputTouch;
	int m_inputPower;
	int m_inputHome;
	int m_inputBack;
	int m_inputVolume;
	quint32 m_screenRotation;
	quint32 m_screenWidth;
	quint32 m_screenHeight;
};

extern DeviceInfo *aDev;

#endif // DEVICEINFO_H
