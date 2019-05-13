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
#ifndef MONKEYHANDLER_H
#define MONKEYHANDLER_H

#include "device/adbclient.h"
#include "input/inputhandler.h"

#include <QTcpSocket>
#include <QTimer>

class MonkeyHandler : public InputHandler
{
public:
	explicit MonkeyHandler(QObject *parent = nullptr);
	virtual ~MonkeyHandler();

	bool init(const WidgetKeyMap &keyMap);
	static void initDaemon();

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
	void sendWheelEvents();

private:
	bool init() override { return false; }

	static AdbClient m_daemon;
	static bool m_daemonReady;
	AdbClient m_monkey;

	WidgetKeyMap m_keyMap;
	QTimer m_wheelTimer;
	int m_wheelX;
	int m_wheelY;
	bool m_inputMouseDown;
};

#endif // MONKEYHANDLER_H
