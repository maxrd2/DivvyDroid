#include "devicetouchhandler.h"

#include <QMouseEvent>
#include <QWidget>
#include <QDebug>

#include "device/deviceinfo.h"

DeviceTouchHandler::DeviceTouchHandler(QObject *parent)
	: InputHandler(parent),
	  m_inputMouseDown(false),
	  m_lastTouchId(33)
{
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
		qDebug() << __FUNCTION__ << "failed opening device" << aDev->inputTouch();
		return false;
	}

	return true;
}

bool
DeviceTouchHandler::eventFilter(QObject *obj, QEvent *ev)
{
	Q_ASSERT(obj->objectName() == QStringLiteral("screen"));

	QWidget *screen = reinterpret_cast<QWidget *>(obj);
	QMouseEvent *mev = reinterpret_cast<QMouseEvent *>(ev);

	switch(ev->type()) {
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
		return false;
	}

	return false;
}
