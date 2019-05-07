#ifndef ADBCLIENT_H
#define ADBCLIENT_H

#include <QTcpSocket>

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

	QImage fetchScreenJpeg();

signals:

public slots:

private:
	QTcpSocket m_sock;
};

#endif // ADBCLIENT_H
