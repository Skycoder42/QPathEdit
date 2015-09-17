#include "form.h"
#include <QApplication>
#include "qpathedit.h"

#define I "C:/Users/Felix/Pictures/imageres.dll/ico_imageres_dll0020.ico"
#define U "C:/Users/Felix/Pictures/open_icon_library-standard-0.11/icons/qt_qrc.ico"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	//QPathEdit::setDialogButtonIcon(QIcon(U));

	Form w;
	w.show();

	return a.exec();
}
