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
#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <QObject>

#include "device/adbclient.h"

typedef QMap<QObject *, quint16> WidgetKeyMap;

class InputHandler : public QObject
{
	Q_OBJECT

public:
	explicit InputHandler(QObject *parent = nullptr);
	virtual ~InputHandler();

	virtual bool init();

protected:
	virtual bool eventFilter(QObject *obj, QEvent *ev) override = 0;

	AdbClient m_adb;
};

#endif // INPUTHANDLER_H
