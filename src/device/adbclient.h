/*
   DivvyDroid - Application to screencast and remote control Android devices.

   Copyright (C) 2019 - Mladen Milinkovic <maxrd2@smoothware.net>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef ADBCLIENT_H
#define ADBCLIENT_H

#include <QTcpSocket>
#include <QImage>

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
	Q_OBJECT
public:
	explicit AdbClient(QObject *parent = nullptr);
	virtual ~AdbClient();

	void connectToHost();
	bool connectToDevice();
	bool forwardTcpPort(int local, int remote);
	inline void close() { m_sock.close(); }
	inline bool waitForDisconnected(int msecs = -1) { return m_sock.waitForDisconnected(msecs); }
	inline bool waitForReadyRead(int msecs = -1) { return m_sock.waitForReadyRead(msecs); }
	inline QTcpSocket::SocketError error() { return m_sock.error(); }
	inline qint64 bytesAvailable() { return m_sock.bytesAvailable(); }
	inline qint64 isConnected() { return m_sock.state() != QTcpSocket::UnconnectedState; }

	bool read(void *data, qint64 max);
	bool write(const void *data, qint64 max);
	bool write(const QByteArray &data) { return write(data.constData(), data.size()); }

	bool send(QByteArray command);

	bool readStatus();
	QByteArray readResponse();
	QByteArray readAll();
	QByteArray readLine();
	QByteArray readAvailable();

	QImage fetchScreenRaw();
	QImage fetchScreenPng();
	QImage fetchScreenJpeg();

	static QByteArray shell(const char *cmd);
	bool sendEvents(AdbEventList events);
	static bool sendEvents(int deviceIndex, AdbEventList events);

protected:
	bool fetchScreenRawInit();

signals:
	void stateChanged(QAbstractSocket::SocketState);
	void onError(QAbstractSocket::SocketError);
	void readyRead();
	void bytesWritten(qint64 bytes);

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
