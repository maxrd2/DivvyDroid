#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <QObject>

#include "adbclient.h"

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
