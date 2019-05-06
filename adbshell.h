#ifndef ADBSHELL_H
#define ADBSHELL_H

#include <QProcess>

class AdbShell : public QProcess
{
public:
	struct Res {
		int rc;
		QByteArray out;
		QByteArray err;
	};

	AdbShell(QObject *parent = nullptr);
	virtual ~AdbShell();

	Res execute(QByteArray command);
};

#endif // ADBSHELL_H
