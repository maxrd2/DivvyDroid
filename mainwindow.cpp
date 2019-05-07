#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <cstdarg>

#include <QMouseEvent>
#include <QProcess>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow),
	  m_videoThread(nullptr),
	  m_screenTimer(this),
	  m_lastTouchId(33),
	  m_inputTouch(-1),
	  m_inputMouseDown(false),
	  m_arch64(false)
{
	for(unsigned int i = 0; i < sizeof(m_keyDevice) / sizeof(*m_keyDevice); i++)
		m_keyDevice[i] = -1;

	ui->setupUi(this);
}

bool
MainWindow::inputHasKey(const QVector<quint64> keyBits, quint64 keyCode)
{
	const int byte = keyCode / (m_arch64 ? 64 : 32);
	const int bit = keyCode % (m_arch64 ? 64 : 32);

	if(byte >= keyBits.size())
		return false;

	return (keyBits.at(byte) & (1ull << bit)) != 0;
}

void
MainWindow::init()
{
	QProcess shell;
	shell.start(QStringLiteral("adb"), QStringList() << QStringLiteral("wait-for-device"));
	shell.waitForFinished();
	if(shell.exitCode()) {
		qCritical() << "Failed to start adb server.";
		QApplication::quit();
		return;
	}

	// device architecture
	m_arch64 = AdbClient::shell("file -L /system/bin/cat | grep 32-bit").size() == 0;

	// video thread
	m_videoThread = new VideoThread();
	connect(m_videoThread, &VideoThread::imageReady, this, &MainWindow::updateScreen);
	m_videoThread->start();

	// device input
	initInput();

	// wake up
	AdbClient::sendEvents(m_keyDevice[KEY_POWER], AdbEventList()
						  << AdbEvent(EV_KEY, KEY_POWER, 1)
						  << AdbEvent(EV_SYN)
						  << AdbEvent(EV_KEY, KEY_POWER, 0)
						  << AdbEvent(EV_SYN));
}

void
MainWindow::updateScreen(const QImage &image, int width, int height)
{
	m_screenWidth = width;
	m_screenHeight = height;
	ui->screen->setPixmap(QPixmap::fromImage(image));
}

void
MainWindow::initInput()
{
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
			m_inputTouch = devIndex;
//		} else if(evBits && (keyBits.first() & 0xffffffff) == 0xfffffffe) {
//			qDebug() << "INPUT device" << devIndex << name << " is keyboard";
		} else if(HAS_BIT(evBits, EV_SYN) && HAS_BIT(evBits, EV_KEY)) {
			qDebug() << "INPUT device" << devIndex << name << "has some keys";
			for(int i = 0; i < 256; i++) {
				if(inputHasKey(keyBits, i)) {
					if(m_keyDevice[i] != -1)
						qDebug() << "INPUT device" << devIndex << "will handle key" << i << "instead of" << m_keyDevice[i];
					else
						qDebug() << "INPUT device" << devIndex << "will handle key" << i;
					m_keyDevice[i] = devIndex;
				}
			}
		} else {
			qDebug() << "INPUT device" << devIndex << name << "is not supported";
		}
	}

	if(m_inputTouch != -1) {
		m_adbTouch.connectToDevice();
		if(!m_adbTouch.send(QByteArray("dev:").append(INPUT_DEV_PATH).append(QString::number(m_inputTouch)))) {
			qDebug() << __FUNCTION__ << "failed opening device" << m_inputTouch;
			return;
		}
	}

	ui->screen->installEventFilter(this);
}

bool
MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
	if(obj == ui->screen) {
		QMouseEvent *mev = reinterpret_cast<QMouseEvent *>(ev);
		switch(ev->type()) {
		case QEvent::MouseButtonPress: {
			m_inputMouseDown = true;
			const int x = mev->x() * m_screenWidth / ui->screen->width();
			const int y = mev->y() * m_screenHeight / ui->screen->height();
			m_adbTouch.sendEvents(AdbEventList()
								  << AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, ++m_lastTouchId)
								  << AdbEvent(EV_ABS, ABS_MT_PRESSURE, 40)
								  << AdbEvent(EV_ABS, ABS_MT_DISTANCE, 0)
								  << AdbEvent(EV_ABS, ABS_MT_TOUCH_MAJOR, 1)
								  << AdbEvent(EV_ABS, ABS_MT_WIDTH_MAJOR, 10)
								  << AdbEvent(EV_ABS, ABS_MT_POSITION_X, x)
								  << AdbEvent(EV_ABS, ABS_MT_POSITION_Y, y)
								  << AdbEvent(EV_ABS, ABS_X, x)
								  << AdbEvent(EV_ABS, ABS_Y, y)
								  << AdbEvent(EV_KEY, BTN_TOUCH, 1)
								  << AdbEvent(EV_SYN));
			qDebug() << "MOUSE DOWN" << x << "," << y;
			return true;
		}
		case QEvent::MouseButtonRelease: {
			m_inputMouseDown = false;
			const int x = mev->x() * m_screenWidth / ui->screen->width();
			const int y = mev->y() * m_screenHeight / ui->screen->height();
			qDebug() << "MOUSE UP" << x << "," << y;
			m_adbTouch.sendEvents(AdbEventList()
								  << AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, m_lastTouchId)
								  << AdbEvent(EV_ABS, ABS_MT_POSITION_X, x)
								  << AdbEvent(EV_ABS, ABS_MT_POSITION_Y, y)
								  << AdbEvent(EV_ABS, ABS_X, x)
								  << AdbEvent(EV_ABS, ABS_Y, y)
								  << AdbEvent(EV_SYN)
								  << AdbEvent(EV_KEY, BTN_TOUCH, 0)
								  << AdbEvent(EV_SYN));
			return true;
		}
		case QEvent::MouseMove:
			if(m_inputMouseDown) {
				const int x = mev->x() * m_screenWidth / ui->screen->width();
				const int y = mev->y() * m_screenHeight / ui->screen->height();
				qDebug() << "MOUSE MOVE" << x << "," << y;
				m_adbTouch.sendEvents(AdbEventList()
									  << AdbEvent(EV_ABS, ABS_MT_TRACKING_ID, m_lastTouchId)
									  << AdbEvent(EV_ABS, ABS_MT_POSITION_X, x)
									  << AdbEvent(EV_ABS, ABS_MT_POSITION_Y, y)
									  << AdbEvent(EV_ABS, ABS_X, x)
									  << AdbEvent(EV_ABS, ABS_Y, y)
									  << AdbEvent(EV_SYN));
			}
			break;

		default:
			return false;
		}

	}
	return false;
}

MainWindow::~MainWindow()
{
	delete ui;
}
