#ifndef QPATHEDITPLUGIN_H
#define QPATHEDITPLUGIN_H

#include <QDesignerCustomWidgetInterface>
#include <QObject>
#include <QPointer>

class QPathEditPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "org.SkycoderSoft.widgets.QPathEdit")
#endif // QT_VERSION >= 0x050000
	Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
	QPathEditPlugin(QObject *parent = 0);

	// QDesignerCustomWidgetInterface interface
	QString name() const Q_DECL_OVERRIDE;
	QString group() const Q_DECL_OVERRIDE;
	QString toolTip() const Q_DECL_OVERRIDE;
	QString whatsThis() const Q_DECL_OVERRIDE;
	QString includeFile() const Q_DECL_OVERRIDE;
	QIcon icon() const Q_DECL_OVERRIDE;
	bool isContainer() const Q_DECL_OVERRIDE;
	QWidget *createWidget(QWidget *parent) Q_DECL_OVERRIDE;
	bool isInitialized() const Q_DECL_OVERRIDE;
	void initialize(QDesignerFormEditorInterface *core) Q_DECL_OVERRIDE;
	QString domXml() const Q_DECL_OVERRIDE;

private:
	QDesignerFormEditorInterface *core;
};

#endif // QPATHEDITPLUGIN_H
