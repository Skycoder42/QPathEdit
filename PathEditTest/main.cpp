#include <QApplication>
#include <qpathedit.h>
#include "form.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	Form w;
	w.show();

	return a.exec();
}
