#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::AboutDialog)
{
	ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
	delete ui;
}

void
AboutDialog::replaceTag(const QString &tag, const QString &text)
{
	ui->lblAbout->setText(ui->lblAbout->text().replace(tag, text));
	ui->lblVersion->setText(ui->lblVersion->text().replace(tag, text));
}
