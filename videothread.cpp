#include "videothread.h"
#include "adbclient.h"

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

		if(canJpeg) {
			img = adb.fetchScreenJpeg();
//			m_canJpeg = !img.isNull();
		} else if(canRaw) {
			img = adb.fetchScreenRaw();
//			canRaw = !img.isNull();
		} else if(canPng) {
			img = adb.fetchScreenPng();
//			canPng = !img.isNull();
		} else {
			return;
		}
		emit imageReady(img.scaledToWidth(360, Qt::SmoothTransformation), img.width(), img.height());
		msleep(10);
	}
}
