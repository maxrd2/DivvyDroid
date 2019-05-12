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
#ifndef DEVICEBUTTONHANDLER_H
#define DEVICEBUTTONHANDLER_H

#include "input/inputhandler.h"

#include <QMap>
#include <QElapsedTimer>

class DeviceButtonHandler : public InputHandler
{
public:
	explicit DeviceButtonHandler(QObject *parent = nullptr);
	virtual ~DeviceButtonHandler();

	bool init(int deviceNr, const WidgetKeyMap &keyMap);

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;

private:
	bool init() override { return false; }

	WidgetKeyMap m_keyMap;
	bool m_useDevice;
	QElapsedTimer m_pressTime;
};

#endif // DEVICEBUTTONHANDLER_H
