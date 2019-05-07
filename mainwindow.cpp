#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QMouseEvent>
#include <QDebug>

//#include "input-event-codes.h"
#include <linux/input.h>

#define INPUT_SYS_PATH "/sys/class/input/input"
#define INPUT_DEV_PATH "/dev/input/event"
#define HAS_BIT(v, b) (v & (1 << b))

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::MainWindow),
	  m_screenTimer(this),
//	  m_shellScreen(this),
//	  m_shellInput(this),
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

	connect(&m_screenTimer, &QTimer::timeout, [&](){
		QImage img = m_adbScreen.fetchScreenJpeg();
		m_screenWidth = img.width();
		m_screenHeight = img.height();
		ui->screen->setPixmap(QPixmap::fromImage(img).scaledToWidth(360, Qt::SmoothTransformation));
	});
	m_screenTimer.start(40);

//	m_adb.send("host:version");
//	m_adb.send("host:devices");
//	m_adb.send("host:devices-l");

//	if(!m_adb.send("host:transport-any"))
//		return;
//	if(m_adb.send("shell:ls -l /sdcard"))
//		qDebug() << m_adb.readAll();
}

void
MainWindow::initInput()
{
	/*
	AdbShell::Res res;

	res = m_shellInput.execute("file /system/bin/cat | grep 32-bit");
	m_arch64 = res.rc;

	res = m_shellInput.execute("ls -d " INPUT_SYS_PATH "*");
	const QStringList inputs(QString(res.out.trimmed()).split('\n'));

	for(const QString input : inputs) {
		int devIndex = input.midRef(sizeof(INPUT_SYS_PATH) - 1).toInt();

		res = m_shellInput.execute(QByteArray("cat ").append(input).append("/name"));
		QString name = res.out.trimmed();

		res = m_shellInput.execute(QByteArray("cat ").append(input).append("/capabilities/ev"));
		int evBits = res.out.trimmed().toInt(nullptr, 16);

		res = m_shellInput.execute(QByteArray("cat ").append(input).append("/capabilities/key"));
		QVector<quint64> keyBits;
		{
			const QList<QByteArray> keyList = res.out.simplified().split(' ');
			for(auto it = keyList.crbegin(); it != keyList.crend(); ++it)
				keyBits.push_back(it->toULongLong(nullptr, 16));
		}

		if(HAS_BIT(evBits, EV_SYN) && HAS_BIT(evBits, EV_ABS) && HAS_BIT(evBits, EV_KEY) && inputHasKey(keyBits, BTN_TOUCH)) {
			qDebug() << "INPUT device" << devIndex << ":" << name << "is touch screen";
			m_inputTouch = devIndex;
//		} else if(evBits && (keyBits.first() & 0xffffffff) == 0xfffffffe) {
//			qDebug() << "INPUT device" << devIndex << ":" << name << " is keyboard";
		} else if(HAS_BIT(evBits, EV_SYN) && HAS_BIT(evBits, EV_KEY)) {
			qDebug() << "INPUT device" << devIndex << ":" << name << "has some keys";
			for(int i = 0; i < 256; i++) {
				if(inputHasKey(keyBits, i)) {
					if(m_keyDevice[i] != -1)
						qDebug() << "INPUT device" << devIndex << "will handle key" << i << "instead of" << m_keyDevice[i];
					m_keyDevice[i] = devIndex;
				}
			}
        } else {
			qDebug() << "INPUT device" << devIndex << ":" << name << "is not supported";
		}
	}

	ui->screen->installEventFilter(this);
	*/
}

bool
MainWindow::sendInput(int deviceIndex, quint16 eventType, quint16 eventCode, qint32 eventValue)
{
	/*
//	AdbShell::Res res = m_shellInput.execute(QString("sendevent %1%2 %3 %4 %5")
//		.arg(INPUT_DEV_PATH).arg(deviceIndex)
//		.arg(eventType)
//		.arg(eventCode)
//		.arg(eventValue)
//		.toUtf8());

	auto hex = [](unsigned char n)->const char *{
		static unsigned char s[3];
		s[0] = n >> 4;
		s[1] = n & 0x0f;
		s[0] = s[0] < 10 ? s[0] + '0' : s[0] - 10 + 'a';
		s[1] = s[1] < 10 ? s[1] + '0' : s[1] - 10 + 'a';
		s[2] = 0;
		return reinterpret_cast<const char *>(s);
	};

	struct {
		union {
			input_event evt;
			unsigned char data[sizeof(input_event)];
		};
	} evt { .evt={ .time={0, 0}, eventType, eventCode, eventValue} };


	QByteArray event("printf '%b' '");
	for(unsigned int i = 0; i < sizeof(input_event); i++) {
		event.append("\\x");
		event.append(hex(evt.data[i]));
	}
	event.append("' > /sdcard/test_event");
//	event.append("' > ");
//	event.append(INPUT_DEV_PATH);
//	event.append(QString::number(deviceIndex));
	AdbShell::Res res = m_shellInput.execute(event);
//	qDebug() << "EVENT sent:" << res.rc << res.out << res.err;
	std::cerr << "EVENT cmd:" << event.constData() << std::endl;
	std::cerr << "EVENT sent:" << res.rc;
	std::cerr << " out: " << res.out.constData();
	std::cerr << " err: " << res.err.constData() << std::endl;
//	AdbShell::Res res = m_shellInput.execute(QString("echo '\\x%3\\x%4\\x%5' \\> %1%2")
//		.arg(INPUT_DEV_PATH).arg(deviceIndex)
//		.arg(eventType, 0, 16)
//		.arg(eventCode, 0, 16)
//		.arg(eventValue, 0, 16)
//		.toUtf8());

	return res.rc == 0;
	*/
	return false;
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
			qDebug() << "MOUSE DOWN" << x << "," << y;
			sendInput(m_inputTouch, EV_ABS, ABS_MT_POSITION_X, x);
			sendInput(m_inputTouch, EV_ABS, ABS_MT_POSITION_Y, y);
			sendInput(m_inputTouch, EV_ABS, ABS_X, x);
			sendInput(m_inputTouch, EV_ABS, ABS_Y, y);
			sendInput(m_inputTouch, EV_KEY, BTN_TOUCH, 1);
			sendReport(m_inputTouch);
			return true;
		}
		case QEvent::MouseButtonRelease: {
			m_inputMouseDown = false;
			const int x = mev->x() * m_screenWidth / ui->screen->width();
			const int y = mev->y() * m_screenHeight / ui->screen->height();
			qDebug() << "MOUSE UP" << x << "," << y;
			sendInput(m_inputTouch, EV_ABS, ABS_MT_POSITION_X, x);
			sendInput(m_inputTouch, EV_ABS, ABS_MT_POSITION_Y, y);
			sendInput(m_inputTouch, EV_ABS, ABS_X, x);
			sendInput(m_inputTouch, EV_ABS, ABS_Y, y);
			sendInput(m_inputTouch, EV_KEY, BTN_TOUCH, 0);
			sendReport(m_inputTouch);
			return true;
		}
		case QEvent::MouseMove:
			if(m_inputMouseDown) {
				const int x = mev->x() * m_screenWidth / ui->screen->width();
				const int y = mev->y() * m_screenHeight / ui->screen->height();
				qDebug() << "MOUSE MOVE" << x << "," << y;
				sendInput(m_inputTouch, EV_ABS, ABS_MT_POSITION_X, x);
				sendInput(m_inputTouch, EV_ABS, ABS_MT_POSITION_Y, y);
				sendInput(m_inputTouch, EV_ABS, ABS_X, x);
				sendInput(m_inputTouch, EV_ABS, ABS_Y, y);
				sendReport(m_inputTouch);
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
