#ifndef FORM_H
#define FORM_H

#include <QWidget>

namespace Ui {
	class Form;
}

class Form : public QWidget
{
	Q_OBJECT

public:
	explicit Form(QWidget *parent = 0);
	~Form();

private slots:
	void on_styleComboBox_activated(int index);

	void on_dialogButtonIconPathEdit_pathChanged(const QString &path);

	void on_pathModeComboBox_activated(int index);

	void on_pathedit_pathChanged(const QString &path);

private:
	Ui::Form *ui;
};

#endif // FORM_H
