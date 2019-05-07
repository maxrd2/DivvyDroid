#ifndef ADBCLIENT_H
#define ADBCLIENT_H

#include <QTcpSocket>
#include <QImage>

#include "input-event-codes.h"
//#include <linux/input.h>

#define INPUT_SYS_PATH "/sys/class/input/input"
#define INPUT_DEV_PATH "/dev/input/event"
#define HAS_BIT(v, b) (v & (1 << b))

struct AdbEvent {
	AdbEvent(quint16 t, quint16 c = 0, qint32 v = 0)
		: time(0),
		  type(t),
		  code(c),
		  value(v)
	{}
	quint64 time;
	quint16 type;
	quint16 code;
	qint32 value;
};
typedef QList<AdbEvent> AdbEventList;

class AdbClient : public QObject
{
public:
	explicit AdbClient(QObject *parent = nullptr);

	void connectToHost() { m_sock.connectToHost(QStringLiteral("localhost"), 5037, QIODevice::ReadWrite); }
	bool connectToDevice(const char *deviceId = nullptr);

	bool read(void *data, qint64 max);
	bool write(const void *data, qint64 max);
	bool write(const QByteArray &data) { return write(data.constData(), data.size()); }

	bool send(QByteArray command);

	bool readStatus();
	QByteArray readResponse();
	QByteArray readAll() { m_sock.waitForDisconnected(); return m_sock.readAll(); }

	QImage fetchScreenRaw();
	QImage fetchScreenPng();
	QImage fetchScreenJpeg();
	void fetchScreenX264();

	static QByteArray shell(const char *cmd);
	bool sendEvents(AdbEventList events);
	static bool sendEvents(int deviceIndex, AdbEventList events);

protected:
	bool fetchScreenRawInit();

signals:

public slots:

private:
	QTcpSocket m_sock;

#define FB_VAR(VAR) (version == 16 ? v0.VAR : version == 1 ? v1.VAR : version == 2 ? v2.VAR : 0)
#define FB_VAR_1(VAR, DEF) (version == 16 ? DEF : version == 1 ? v1.VAR : version == 2 ? v2.VAR : 0)
	struct FBInfo {
		quint32 version;
		union {
			struct {
				quint32 size;
				quint32 width;
				quint32 height;
			} v0; // version=16 here which is actually 16bpp
			struct {
				quint32 bpp;
				quint32 size;
				quint32 width;
				quint32 height;
				quint32 red_offset;
				quint32 red_length;
				quint32 blue_offset;
				quint32 blue_length;
				quint32 green_offset;
				quint32 green_length;
				quint32 alpha_offset;
				quint32 alpha_length;
			} v1;
			struct {
				quint32 bpp;
				quint32 colorSpace;
				quint32 size;
				quint32 width;
				quint32 height;
				quint32 red_offset;
				quint32 red_length;
				quint32 blue_offset;
				quint32 blue_length;
				quint32 green_offset;
				quint32 green_length;
				quint32 alpha_offset;
				quint32 alpha_length;
			} v2;
		};
		quint32 width() { return FB_VAR(width); }
		quint32 height() { return FB_VAR(height); }
		quint32 size() { return FB_VAR(size); }
		quint32 bpp() { return FB_VAR_1(bpp, 16); }
		QImage::Format format() {
			if(version == 16)
				return QImage::Format_RGB16;
//			if(version == 1 && v1.bpp == 24)
//				return QImage::Format_RGB888;
//			if(version <= 2 && v1.bpp == 32) // TODO: consider offsets and lengths to figure the format
				return QImage::Format_RGB32;
		}
	} m_fbInfo;
};

#endif // ADBCLIENT_H
