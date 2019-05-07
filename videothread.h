#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QThread>

class VideoThread : public QThread
{
	Q_OBJECT
public:
	VideoThread(QObject *parent = nullptr);
	~VideoThread();

	void setWidth(int width) { m_imageWidth = width; }

signals:
    void imageReady(const QImage &image, int width, int height);

protected:
    void run();

private:
	int m_imageWidth;
};

#endif // VIDEOTHREAD_H
