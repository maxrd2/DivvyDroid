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
#include "shellkeyboardhandler.h"

#include <QKeyEvent>
#include <QDebug>

#include "device/deviceinfo.h"
#include "input/input_to_adroid_keys.h"
#include "input/android_keycodes.h"

#define TIMEOUT_MS 150

ShellKeyboardHandler::ShellKeyboardHandler(QObject *parent)
	: InputHandler(parent)
{
	connect(&m_timer, &QTimer::timeout, this, &ShellKeyboardHandler::sendEvents);
	m_timer.setSingleShot(true);
}

ShellKeyboardHandler::~ShellKeyboardHandler()
{

}

void
ShellKeyboardHandler::sendEvents()
{
	if(!m_bufferedKeys.isEmpty()) {
		qDebug() << "sending key events";
		AdbClient::shell(QByteArray("input keyevent").append(m_bufferedKeys));
		m_bufferedKeys.clear();
	}
	if(!m_bufferedText.isEmpty()) {
		qDebug() << "sending text events";
		AdbClient::shell(QByteArray("input text \"").append(m_bufferedText).append('"'));
		m_bufferedText.clear();
	}
}

bool
ShellKeyboardHandler::eventFilter(QObject */*obj*/, QEvent *ev)
{
	if(ev->type() == QEvent::KeyRelease) {
		QKeyEvent *kev = reinterpret_cast<QKeyEvent *>(ev);
		const QString text = kev->text();
		if(text.isEmpty() || !text.at(0).isPrint()) {
			const auto key = qtToAndroidCode.find(Qt::Key(kev->key()));
			if(key != qtToAndroidCode.cend()) {
				if(!m_bufferedText.isEmpty())
					sendEvents();
				m_bufferedKeys.append(' ').append(QString::number(key.value()));
				m_timer.start(TIMEOUT_MS);
				return true;
			}
		} else {
			if(!m_bufferedKeys.isEmpty())
				sendEvents();
			for(int i = 0; i < text.length(); i++) {
				const QChar ch = text.at(i);
				if(ch == '"')
					m_bufferedText.append("\"'\"'\"");
				else if(ch.isPrint())
					m_bufferedText.append(ch);
			}
			m_timer.start(TIMEOUT_MS);
			return true;
		}
	}

	return false;
}
