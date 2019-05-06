#include "adbshell.h"

#include <QDebug>

#define SHELL_RC_SEPARATOR "-!-!SHELL-RC!-!-"

AdbShell::AdbShell(QObject *parent)
	: QProcess(parent)
{
	connect(this, &AdbShell::stateChanged, [](ProcessState state){
		qDebug() << "ADB SHELL state:" << state;
	});

	start(QStringLiteral("adb"), QStringList() << QStringLiteral("shell") << QStringLiteral("-T"));
}

AdbShell::~AdbShell()
{
	waitForStarted();
	write("exit\n");
	waitForFinished();
}

AdbShell::Res
AdbShell::execute(QByteArray command)
{
	waitForStarted();
	write(command);
	write("\necho \":$?" SHELL_RC_SEPARATOR "\"\n");
	waitForBytesWritten();

	AdbShell::Res res;
	do {
		waitForReadyRead();
		res.out.append(readAllStandardOutput());
		res.err.append(readAllStandardError());
	} while(!res.out.endsWith(SHELL_RC_SEPARATOR "\n"));

	int i = res.out.size() - sizeof(SHELL_RC_SEPARATOR);
	while(i > 0 && res.out.at(i) != ':')
		i--;
	res.rc = res.out.mid(i + 1).toInt();
	res.out.truncate(i);
	qDebug() << "SHELL " << command << "; rc:" << res.rc;

	return res;
}
