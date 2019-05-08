#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QStringList>
#include <QMap>
#include <QByteArray>

#include "input-event-codes.h"
//#include <linux/input.h>

#define INPUT_SYS_PATH "/sys/class/input/input"
#define INPUT_DEV_PATH "/dev/input/event"
#define HAS_BIT(v, b) (v & (1 << b))

typedef QMap<QByteArray, QByteArray> DeviceList;

class DeviceInfo
{
public:
	inline const QString & deviceId() const { return m_deviceId; }

	inline bool isArch64() const { return m_arch64; }

	bool isScreenAwake() const;
	inline quint32 screenWidth() const { return m_screenWidth; }
	inline quint32 screenHeight() const { return m_screenHeight; }

	inline int inputTouch() const { return m_inputTouch; }
	inline int inputPower() const { return m_inputPower; }
	inline int inputHome() const { return m_inputHome; }
	inline int inputVolume() const { return m_inputVolume; }

	static DeviceList deviceList();
	static void connect(const char *deviceId = nullptr);
	static bool waitForDevice();
protected:
	DeviceInfo(const char *deviceId = nullptr);

private:
	QString m_deviceId;
	bool m_arch64;
	int m_inputTouch;
	int m_inputPower;
	int m_inputHome;
	int m_inputVolume;
	quint32 m_screenWidth;
	quint32 m_screenHeight;
};

extern DeviceInfo *aDev;

#endif // DEVICEINFO_H
