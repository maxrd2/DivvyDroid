#include "inputhandler.h"

InputHandler::InputHandler(QObject *parent)
	: QObject(parent)
{
}

InputHandler::~InputHandler()
{
}

bool
InputHandler::init()
{
	return m_adb.connectToDevice();
}
