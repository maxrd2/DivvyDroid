#include "videothread.h"
#include "adbclient.h"
#include "deviceinfo.h"

#include <QImage>

#define IMAGE_WIDTH 360

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
