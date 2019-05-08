#include "deviceinfo.h"

#include "adbclient.h"

#include <QProcess>
#include <QDebug>

DeviceInfo *aDev = nullptr;

DeviceInfo::DeviceInfo(const char *deviceId)
	: m_deviceId(deviceId),
	  m_inputTouch(-1),
	  m_inputPower(-1),
	  m_inputHome(-1),
	  m_inputVolume(-1)
{
}

bool
DeviceInfo::waitForDevice()
{
	QProcess shell;
	shell.start(QStringLiteral("adb"), QStringList() << QStringLiteral("wait-for-device"));
	shell.waitForFinished();
	if(shell.exitCode()) {
		qWarning() << "Failed to start adb server.";
		return false;
	}

	return true;
}

DeviceList
DeviceInfo::deviceList()
{
	AdbClient adb;
	if(!adb.send("host:devices-l"))
		return DeviceList();

	DeviceList devList;

	QList<QByteArray> devices = adb.readResponse().split('\n');
	for(const QByteArray &dev : devices) {
		QByteArray info = dev.simplified();
		if(info.isEmpty())
			continue;

		const int i = info.indexOf(' ');
		devList.insert(info.left(i), info.mid(i + 1));
	}

	return devList;
}

static bool
inputHasKey(const QVector<quint64> keyBits, quint64 keyCode)
{
	const int byte = keyCode / (aDev->isArch64() ? 64 : 32);
	const int bit = keyCode % (aDev->isArch64() ? 64 : 32);

	if(byte >= keyBits.size())
		return false;

	return (keyBits.at(byte) & (1ull << bit)) != 0;
}


void
DeviceInfo::connect(const char *deviceId)
{
	delete aDev;
	aDev = new DeviceInfo(deviceId);

	// device architecture
	aDev->m_arch64 = AdbClient::shell("file -L /system/bin/cat | grep 32-bit").size() == 0;

	// screen resolution
	const QByteArray res = AdbClient::shell("dumpsys display | grep -E 'mDisplayWidth|mDisplayHeight'").replace('\n', '\0');
	int i = res.indexOf("mDisplayWidth");
	aDev->m_screenWidth = i != -1 ? res.mid(res.indexOf('=', i) + 1).toInt() : 0;
	i = res.indexOf("DisplayHeight");
	aDev->m_screenHeight = i != -1 ? res.mid(res.indexOf('=', i) + 1).toInt() : 0;

	// enumerate input devices
	QList<QByteArray> inputs = AdbClient::shell("ls -d " INPUT_SYS_PATH "*").simplified().split(' ');

	for(const QByteArray &input : inputs) {
		int devIndex = input.mid(sizeof(INPUT_SYS_PATH) - 1).toInt();

		QList<QByteArray> res = AdbClient::shell(QByteArray("cat ")
							   .append(input).append("/name ")
							   .append(input).append("/capabilities/ev ")
							   .append(input).append("/capabilities/key ")
							   .append("2>/dev/null")).split('\n');

		QString name = res.at(0).trimmed();
		int evBits = res.at(1).trimmed().toInt(nullptr, 16);
		QVector<quint64> keyBits;
		const QList<QByteArray> keyList = res.at(2).simplified().split(' ');
		for(auto it = keyList.crbegin(); it != keyList.crend(); ++it)
			keyBits.push_back(it->toULongLong(nullptr, 16));

		if(HAS_BIT(evBits, EV_SYN) && HAS_BIT(evBits, EV_ABS) && HAS_BIT(evBits, EV_KEY) && inputHasKey(keyBits, BTN_TOUCH)) {
			qDebug() << "INPUT device" << devIndex << name << "is touch screen";
			aDev->m_inputTouch = devIndex;
		} else if(HAS_BIT(evBits, EV_SYN) && HAS_BIT(evBits, EV_KEY)) {
			if(inputHasKey(keyBits, KEY_HOME) && inputHasKey(keyBits, KEY_BACK)) {
				qDebug() << "INPUT device" << devIndex << name << "has some home/back key";
				aDev->m_inputHome = devIndex;
			} else if(inputHasKey(keyBits, KEY_POWER)) {
				qDebug() << "INPUT device" << devIndex << name << "has some power key";
				aDev->m_inputPower = devIndex;
			} else if(inputHasKey(keyBits, KEY_VOLUMEUP) && inputHasKey(keyBits, KEY_VOLUMEDOWN)) {
				qDebug() << "INPUT device" << devIndex << name << "has some volume keys";
				aDev->m_inputVolume = devIndex;
			} else {
				qDebug() << "INPUT device" << devIndex << name << "has some keys";
			}
		} else {
			qDebug() << "INPUT device" << devIndex << name << "is not supported";
		}
	}
}

bool
DeviceInfo::isScreenAwake() const
{
	return AdbClient::shell("service call power 12").at(31) == '1';
}
