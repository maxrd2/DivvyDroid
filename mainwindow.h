#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "adbclient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void init();
	void initInput();
//	bool sendInput(int deviceIndex, quint16 eventType, quint16 eventCode, qint32 eventValue, ...);
//	bool sendReport(int deviceIndex) { return sendInput(deviceIndex, 0/*EV_SYN*/, 0, 0); }

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;
	void sendMouseDown();

private:
	bool inputHasKey(const QVector<quint64> keyBits, quint64 keyCode);

	Ui::MainWindow *ui;

	QTimer m_screenTimer;
	quint32 m_screenWidth;
	quint32 m_screenHeight;
	AdbClient m_adbScreen;
	bool m_canJpeg;
	bool m_canPng;
	bool m_canRaw;

	QPoint m_lastMouseDown;
	QTimer m_mouseDownTimer;
	qint32 m_lastTouchId;
	AdbClient m_adbTouch;
//	AdbShell m_shellScreen;
//	AdbShell m_shellInput;

	int m_inputTouch;
	bool m_inputMouseDown;
	bool m_arch64;
	qint32 m_keyDevice[256];
};

#endif // MAINWINDOW_H
