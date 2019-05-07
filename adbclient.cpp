#include "adbclient.h"

#include <QElapsedTimer>
#include <QImage>
#include <QDebug>

AdbClient::AdbClient(QObject *parent)
	: QObject(parent)
{
	connect(&m_sock, &QTcpSocket::stateChanged, [&](QTcpSocket::SocketState state){
		qDebug() << "ADB CONN:" << state;
	});
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
    return true;
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

bool
AdbClient::readStatus()
{
	char buf[256];

	if(!read(buf, 4)) {
		qDebug() << __FUNCTION__ << "failed: protocol fault (no status)";
		return false;
	}

	if(memcmp(buf, "OKAY", 4) == 0) {
		qDebug() << "ADB OKAY";
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

	if(read(buf, len)) {
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
	QByteArray res;

	size_t len = 0;
	{
		char tmp[5];
		if(!read(tmp, 4)) {
			qDebug() << __FUNCTION__ << "failed: missing response length";
			return res;
		}
		tmp[4] = 0;
		len = strtoul((char*)tmp, 0, 16);
	}

	if(len) {
		res.reserve(len + 1);
		if(!read(res.data(), len)) {
			qDebug() << __FUNCTION__ << "failed: missing response length";
			return res;
		}
	}

	return res;
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
	qDebug() << "ADB SEND" << command;

	return readStatus();
}

bool
AdbClient::connectToDevice(const char *deviceId)
{
	deviceId; // TODO: connect to specific device
	if(!send("host:transport-any")) {
		qWarning() << "WARNING: unable to connect to android device";
		return false;
	}

	return true;
}

QImage
AdbClient::fetchScreenJpeg()
{
	QElapsedTimer timer;
	timer.start();

	if(!connectToDevice())
		return QImage();

	if(!send("shell:/system/bin/screencap -j")) {
		qWarning() << "WARNING: unable to execute shell command";
		return QImage();
	}

	QByteArray res = readAll();
	qDebug() << "SCREEN frame retrieved in" << timer.elapsed() << "ms";

	return QImage::fromData(res);
}
