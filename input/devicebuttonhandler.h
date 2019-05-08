#ifndef DEVICEBUTTONHANDLER_H
#define DEVICEBUTTONHANDLER_H

#include "inputhandler.h"

#include <QMap>

typedef QMap<QObject *, quint16> WidgetKeyMap;

class DeviceButtonHandler : public InputHandler
{
public:
	explicit DeviceButtonHandler(QObject *parent = nullptr);
	virtual ~DeviceButtonHandler();

	bool init(int deviceId, const WidgetKeyMap &keyMap);

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;

private:
	bool init() override { return false; }

	WidgetKeyMap m_keyMap;
};

#endif // DEVICEBUTTONHANDLER_H
