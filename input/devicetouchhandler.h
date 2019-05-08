#ifndef DEVICETOUCHHANDLER_H
#define DEVICETOUCHHANDLER_H

#include "inputhandler.h"

class DeviceTouchHandler : public InputHandler
{
public:
	explicit DeviceTouchHandler(QObject *parent = nullptr);
	virtual ~DeviceTouchHandler();

	bool init() override;

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;

private:
	bool m_inputMouseDown;
	qint32 m_lastTouchId;
};

#endif // DEVICETOUCHHANDLER_H
