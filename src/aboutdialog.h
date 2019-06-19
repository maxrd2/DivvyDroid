#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AboutDialog(QWidget *parent = nullptr);
	~AboutDialog();

	void replaceTag(const QString &tag, const QString &text);

private:
	Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
