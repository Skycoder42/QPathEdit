#include "form.h"
#include "ui_form.h"
#include <QDebug>

Form::Form(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Form)
{
	ui->setupUi(this);
}

Form::~Form()
{
	delete ui;
}

void Form::on_styleComboBox_activated(int index)
{
	switch(index) {
	case 0:
		this->ui->pathedit->setStyle(QPathEdit::SeperatedButton);
		break;
	case 1:
		this->ui->pathedit->setStyle(QPathEdit::JoinedButton);
		break;
	case 2:
		this->ui->pathedit->setStyle(QPathEdit::NoButton);
		break;
	default:
		break;
	}
}

void Form::on_dialogButtonIconPathEdit_pathChanged(const QString &path)
{
	if(!path.isEmpty())
		this->ui->pathedit->setDialogButtonIcon(QIcon(path));
}

void Form::on_pathModeComboBox_activated(int index)
{
	switch(index) {
	case 0:
		this->ui->pathedit->setPathMode(QPathEdit::ExistingFile);
		break;
	case 1:
		this->ui->pathedit->setPathMode(QPathEdit::ExistingFolder);
		break;
	case 2:
		this->ui->pathedit->setPathMode(QPathEdit::AnyFile);
		break;
	default:
		break;
	}
}

void Form::on_pathedit_pathChanged(const QString &path)
{
	qDebug() << "\npath changed:" << path;
}
