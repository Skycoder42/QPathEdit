#ifndef QPATHEDITPLUGIN_H
#define QPATHEDITPLUGIN_H

#include <QDesignerCustomWidgetInterface>
#include <QObject>
#include <QPointer>

class QPathEditPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QDesignerCustomWidgetInterface_iid FILE "qpathedit.json")
	Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
	QPathEditPlugin(QObject *parent = nullptr);

	// QDesignerCustomWidgetInterface interface
	QString name() const override;
	QString group() const override;
	QString toolTip() const override;
	QString whatsThis() const override;
	QString includeFile() const override;
	QIcon icon() const override;
	bool isContainer() const override;
	QWidget *createWidget(QWidget *parent) override;
	bool isInitialized() const override;
	void initialize(QDesignerFormEditorInterface *core) override;
	QString domXml() const override;

private:
	QDesignerFormEditorInterface *core;
};

#endif // QPATHEDITPLUGIN_H
