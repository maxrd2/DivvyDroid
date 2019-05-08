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
#include "videothread.h"
#include "adbclient.h"
#include "deviceinfo.h"

#include <QImage>

VideoThread::VideoThread(QObject *parent)
	: QThread(parent)
{
}

VideoThread::~VideoThread()
{
	requestInterruption();
	wait();
}

void
VideoThread::run()
{
	bool canJpeg = true;
	bool canPng = true;
	bool canRaw = true;
	AdbClient adb;

	while(!isInterruptionRequested()) {
		QImage img;
		if(aDev->isScreenAwake()) {
			if(canJpeg) {
				img = adb.fetchScreenJpeg();
				canJpeg = !img.isNull();
			} else if(canRaw) {
				img = adb.fetchScreenRaw();
				canRaw = !img.isNull();
			} else if(canPng) {
				img = adb.fetchScreenPng();
				canPng = !img.isNull();
			} else {
				return;
			}
		} else {
			img = QImage(IMAGE_WIDTH, IMAGE_WIDTH * aDev->screenHeight() / aDev->screenWidth(), QImage::Format_RGB888);
			img.fill(Qt::black);
		}
		emit imageReady(img.scaledToWidth(IMAGE_WIDTH, Qt::SmoothTransformation));
		msleep(10);
	}
}
