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
#include "adbclient.h"

#include "deviceinfo.h"

#include <QElapsedTimer>
#include <QPixmap>
#include <QDebug>

//#define ADB_DEBUG_CONN
//#define ADB_DEBUG_SHELL

AdbClient::AdbClient(QObject *parent)
	: QObject(parent)
{
	connect(&m_sock, &QTcpSocket::stateChanged, this, &AdbClient::stateChanged);
	connect(&m_sock, QOverload<QTcpSocket::SocketError>::of(&QTcpSocket::error), this, &AdbClient::onError);
	connect(&m_sock, &QTcpSocket::readyRead, this, &AdbClient::readyRead);
	connect(&m_sock, &QTcpSocket::bytesWritten, this, &AdbClient::bytesWritten);
#ifdef ADB_DEBUG_CONN
	connect(&m_sock, &QTcpSocket::stateChanged, [&](QTcpSocket::SocketState state){
		qDebug() << "ADB CONN:" << state;
	});
#endif
}

AdbClient::~AdbClient()
{
	m_sock.close();
}

bool
AdbClient::write(const void *data, qint64 max)
{
    int done = 0;
    while(max > done) {
        int n = m_sock.write((char*)data + done, max - done);
		if(n <= 0) {
			qDebug() << __FUNCTION__ << "failed";
            return false;
		}
        done += n;
    }
	return m_sock.waitForBytesWritten();
}

bool
AdbClient::read(void *data, qint64 max)
{
    int done = 0;
    while(max > done) {
        const int n = m_sock.read((char*)data + done, max - done);
		if(n < 0) {
			qDebug() << __FUNCTION__ << "failed";
            return false;
		}
        if(n == 0) {
            if(!m_sock.waitForReadyRead())
                return false;
        }
        done += n;
    }
    return true;
}

QByteArray
AdbClient::readAll()
{
	QByteArray buf;

	while(m_sock.waitForReadyRead())
		buf.append(m_sock.readAll());
	Q_ASSERT(m_sock.state() == QAbstractSocket::UnconnectedState);
	return buf;
}

QByteArray
AdbClient::readLine()
{
	while(!m_sock.canReadLine() && m_sock.waitForReadyRead());
	return m_sock.readLine();
}

QByteArray
AdbClient::readAvailable()
{
	m_sock.waitForReadyRead();
	return m_sock.readAll();
}

bool
AdbClient::readStatus()
{
	char buf[256];

	if(!read(buf, 4)) {
		qDebug() << __FUNCTION__ << "failed: protocol fault (no status)";
		return false;
	}

	if(memcmp(buf, "OKAY", 4) == 0) {
#ifdef ADB_DEBUG_CONN
		qDebug() << "ADB OKAY";
#endif
		return true;
	}

	if(memcmp(buf, "FAIL", 4)) {
		qDebug() << __FUNCTION__ << "failed: invalid status response";
		return false;
	}

	if(!read(buf, 4)) {
		qDebug() << __FUNCTION__ << "failed: missing error message length";
		return false;
	}

	buf[4] = 0;
	unsigned int len = strtoul((char*)buf, 0, 16);
	if(len > 255) len = 255;

	if(!read(buf, len)) {
		qDebug() << __FUNCTION__ << "failed: missing error message";
		return false;
	}
	buf[len] = 0;
	qDebug() << "ADB FAIL:" << buf;
	return false;
}

QByteArray
AdbClient::readResponse()
{
	int len = 0;
	{
		char tmp[5];
		if(!read(tmp, 4)) {
			qDebug() << __FUNCTION__ << "failed: missing response length";
			return QByteArray();
		}
		tmp[4] = 0;
		len = strtoul((char*)tmp, 0, 16);
	}

	if(len) {
		QByteArray res;
		res.reserve(len + 1);
		res[0] = res[len] = 0;
		if(!read(res.data(), len))
			qDebug() << __FUNCTION__ << "failed: missing response data";
		return res;
	}

	return QByteArray();
}

bool
AdbClient::send(QByteArray command)
{
	if(m_sock.state() != QTcpSocket::ConnectedState) {
		connectToHost();
		if(!m_sock.waitForConnected()) {
			qWarning() << __FUNCTION__ << "failed: error connecting to adb server.";
			return false;
		}
	}

	write(QString("%1").arg(command.size(), 4, 16, QChar('0')).toLatin1());
	write(command);
#ifdef ADB_DEBUG_CONN
	qDebug() << "ADB SEND" << command;
#endif

	return readStatus();
}

void
AdbClient::connectToHost()
{
	//m_sock.setSocketOption(QTcpSocket::LowDelayOption, 1); // TCP_NODELAY
	m_sock.setSocketOption(QTcpSocket::KeepAliveOption, 1); // SO_KEEPALIVE
	m_sock.connectToHost(QStringLiteral("localhost"), 5037, QIODevice::ReadWrite);
}


bool
AdbClient::connectToDevice()
{
	QByteArray cmd("host:transport");
	if(aDev->deviceId().isEmpty())
		cmd.append("-any");
	else
		cmd.append(":").append(aDev->deviceId());

	if(!send(cmd)) {
		qWarning() << "WARNING: unable to connect to android device";
		return false;
	}

	return true;
}

bool
AdbClient::forwardTcpPort(int local, int remote)
{
	QByteArray cmd;
	if(aDev->deviceId().isEmpty())
		cmd = QByteArray("host:");
	else
		cmd = QByteArray("host-serial:").append(aDev->deviceId());

	cmd.append("forward:tcp:").append(QString::number(local))
		.append(";tcp:").append(QString::number(remote));

	if(!send(cmd)) {
		qWarning() << "WARNING: unable to forward port to android device";
		return false;
	}

	return true;
}

bool
AdbClient::fetchScreenRawInit()
{
	if(!connectToDevice())
		return false;

	if(!send("framebuffer:")) {
		qWarning() << "FRAMEBUFFER unable to connect to framebuffer";
		return false;
	}
	if(!read(&m_fbInfo.version, sizeof(m_fbInfo.version))) {
		qDebug() << "FRAMEBUFFER error reading framebuffer version";
		return false;
	}

	bool res = false;
	switch(m_fbInfo.version) {
	case 16: // version 0
		qDebug() << "FRAMEBUFFER v0";
		res = read(&m_fbInfo.v0, sizeof(m_fbInfo.v0));
		if(!write("0", 1)) {
			qDebug() << "FRAMEBUFFER error writing v0 request";
			return false;
		}
		break;
	case 1:
		qDebug() << "FRAMEBUFFER v1";
		res = read(&m_fbInfo.v1, sizeof(m_fbInfo.v1));
		Q_ASSERT(m_fbInfo.v1.size == m_fbInfo.v1.width * m_fbInfo.v1.height * m_fbInfo.v1.bpp / 8);
		break;
	case 2:
		qDebug() << "FRAMEBUFFER v2";
		res = read(&m_fbInfo.v2, sizeof(m_fbInfo.v2));
		break;
	default:
		qDebug() << "FRAMEBUFFER version unsupported";
		res = false;
		break;
	}
	if(!res) {
		qDebug() << "FRAMEBUFFER error reading framebuffer info";
		return false;
	}

	qDebug() << "FRAMEBUFFER " << m_fbInfo.width() << 'x' << m_fbInfo.height() << 'x' << m_fbInfo.bpp() << ' ' << m_fbInfo.size() << "bytes";

	return true;
}

QImage
AdbClient::fetchScreenRaw()
{
	QElapsedTimer timer;
	timer.start();

	if(!fetchScreenRawInit())
		return QImage();

	const int bytesPerLine = m_fbInfo.width() * m_fbInfo.bpp() / 8;
	QImage img(m_fbInfo.width(), m_fbInfo.height(), m_fbInfo.format());
	for(int y = 0, h = img.height(); y < h; y++) {
		if(!read(img.scanLine(y), bytesPerLine)) {
			qDebug() << "FRAMEBUFFER error reading framebuffer frame";
			return img;
		}
	}
	if(m_fbInfo.format() == QImage::Format_RGB32) {
		// swap R and B
		for(int y = 0, h = img.height(); y < h; y++) {
			uchar *s = img.scanLine(y);
			for(int x = 0, w = img.width(); x < w; x++) {
				s += 4;
				const int t = s[0];
				s[0] = s[2];
				s[2] = t;
			}
		}
	}

	m_sock.readAll();
	m_sock.waitForDisconnected();

	qDebug() << "FRAMEBUFFER RAW frame retrieved in" << timer.elapsed() << "ms";

	return img;
}

QImage
AdbClient::fetchScreenPng()
{
	QElapsedTimer timer;
	timer.start();

	if(!connectToDevice())
		return QImage();

	if(!send("shell:stty raw; screencap -p")) {
		qWarning() << "FRAMEBUFFER error executing PNG screencap";
		return QImage();
	}

	QByteArray res = readAll();
	qDebug() << "FRAMEBUFFER PNG frame retrieved in" << timer.elapsed() << "ms";

	return QImage::fromData(res);
}

QImage
AdbClient::fetchScreenJpeg()
{
	QElapsedTimer timer;
	timer.start();

	if(!connectToDevice())
		return QImage();

	if(!send("shell:stty raw; screencap -j")) {
		qWarning() << "FRAMEBUFFER error executing JPEG screencap";
		return QImage();
	}

	QByteArray res = readAll();
	qDebug() << "SCREEN JPEG frame retrieved in" << timer.elapsed() << "ms";

	return QImage::fromData(res);
}

/*static*/ QByteArray
AdbClient::shell(const char *cmd)
{
	AdbClient adb;

	if(!adb.connectToDevice())
		return QByteArray();

	if(!adb.send(QByteArray("shell:").append(cmd))) {
		qWarning() << "WARNING: unable to execute shell command:" << cmd;
		return QByteArray();
	}

#ifdef ADB_DEBUG_SHELL
	qWarning() << "SHELL" << cmd;
#endif

	return adb.readAll();
}

bool
AdbClient::sendEvents(AdbEventList events)
{
	for(const AdbEvent &evt : events) {
		if(!write(&evt, sizeof(AdbEvent))) {
			qDebug() << __FUNCTION__ << "failed sending event";
			return false;
		}
	}

	return true;
}

/*static*/ bool
AdbClient::sendEvents(int deviceIndex, AdbEventList events)
{
	AdbClient adb;
	if(!adb.connectToDevice())
		return false;
	if(!adb.send(QByteArray("dev:").append(INPUT_DEV_PATH).append(QString::number(deviceIndex)))) {
		qDebug() << __FUNCTION__ << "failed opening device" << deviceIndex;
		return false;
	}

	return adb.sendEvents(events);
}
