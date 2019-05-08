#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "adbclient.h"
#include "videothread.h"

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

protected:
	void sendMouseDown();

public slots:
	void updateScreen(const QImage &image);

private:
	Ui::MainWindow *ui;

	VideoThread *m_videoThread;

	AdbClient m_adbTouch;
};

#endif // MAINWINDOW_H
