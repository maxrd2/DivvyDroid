#include "devicebuttonhandler.h"

#include "device/deviceinfo.h"
#include "device/adbclient.h"
#include "input/input-event-codes.h"

#include <QMouseEvent>

DeviceButtonHandler::DeviceButtonHandler(QObject *parent)
	: InputHandler(parent)
{
}

DeviceButtonHandler::~DeviceButtonHandler()
{

}

bool
DeviceButtonHandler::init(int deviceId, const WidgetKeyMap &keyMap)
{
	if(deviceId == -1) {
		qDebug() << __FUNCTION__ << "failed opening device" << deviceId;
		return false;
	}

	if(!InputHandler::init())
		return false;

	m_adb.connectToDevice();
	if(!m_adb.send(QByteArray("dev:").append(INPUT_DEV_PATH).append(QString::number(deviceId)))) {
		qDebug() << __FUNCTION__ << "failed opening device" << deviceId;
		return false;
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
		m_adb.sendEvents(AdbEventList()
				<< AdbEvent(EV_KEY, keyCode, 1)
				<< AdbEvent(EV_SYN));
		return true;
	}
	case QEvent::MouseButtonRelease: {
		qDebug() << "KEY UP" << keyCode;
		m_adb.sendEvents(AdbEventList()
				<< AdbEvent(EV_KEY, keyCode, 0)
				<< AdbEvent(EV_SYN));
		return true;
	}

	default:
		return false;
	}

	return false;
}
