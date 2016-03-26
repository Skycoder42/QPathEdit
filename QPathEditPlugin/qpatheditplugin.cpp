#include "qpatheditplugin.h"
#include <qpathedit.h>

QPathEditPlugin::QPathEditPlugin(QObject *parent) :
	QObject(parent),
	QDesignerCustomWidgetInterface(),
	core(NULL)
{}

QString QPathEditPlugin::name() const
{
	return QStringLiteral("QPathEdit");
}

QString QPathEditPlugin::group() const
{
	return tr("Input Widgets");
}

QString QPathEditPlugin::toolTip() const
{
	return tr("A lineedit, that can be used to enter paths to files in a user-friendly way");
}

QString QPathEditPlugin::whatsThis() const
{
	return tr("A lineedit, that can be used to enter paths to files.<br>"
			  "It allows the user to enter filepaths, and provides a button "
			  "that will show a QFileDialog to the user, to simplify path editing.<br>"
			  "In addition to that, a completer and validator are in use to make sure, "
			  "that valid paths will be entered");
}

QString QPathEditPlugin::includeFile() const
{
	return QStringLiteral("qpathedit.h");
}

QIcon QPathEditPlugin::icon() const
{
	return QIcon(QStringLiteral(":/qpathedit/icons/lineedit.png"));
}

bool QPathEditPlugin::isContainer() const
{
	return false;
}

QWidget *QPathEditPlugin::createWidget(QWidget *parent)
{
	return new QPathEdit(parent);
}

bool QPathEditPlugin::isInitialized() const
{
	return this->core;
}

void QPathEditPlugin::initialize(QDesignerFormEditorInterface *core)
{
	this->core = core;
}

QString QPathEditPlugin::domXml() const
{
	return QLatin1String("<ui language=\"c++\">\n"
		   " <widget class=\"QPathEdit\" name=\"pathedit\" />\n"
		   " <customwidgets>\n"
		   "  <customwidget>\n"
		   "   <class>QPathEdit</class>\n"
		   "   <extends>QWidget</extends>\n"
		   "   <header>qpathedit.h</header>\n"
		   "  </customwidget>\n"
		   " </customwidgets>\n"
		   "</ui>\n");
}
