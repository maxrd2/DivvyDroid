#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "adbshell.h"
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
	bool sendInput(int deviceIndex, quint16 eventType, quint16 eventCode, qint32 eventValue);
	bool sendReport(int deviceIndex) { return sendInput(deviceIndex, 0, 0, 0); }

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;

private:
	bool inputHasKey(const QVector<quint64> keyBits, quint64 keyCode);

	Ui::MainWindow *ui;

	QTimer m_screenTimer;
	quint32 m_screenWidth;
	quint32 m_screenHeight;

	AdbClient m_adbScreen;

//	AdbShell m_shellScreen;
//	AdbShell m_shellInput;

	int m_inputTouch;
	bool m_inputMouseDown;
	bool m_arch64;
	qint32 m_keyDevice[256];
};

#endif // MAINWINDOW_H
