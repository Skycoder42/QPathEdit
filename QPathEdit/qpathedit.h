#ifndef QPATHEDIT_H
#define QPATHEDIT_H

#include <QWidget>
#include <QFileDialog>
#include <QIcon>
#include <QString>
#include <QPointer>

#ifdef DESIGNER_PLUGIN
#include <QDesignerExportWidget>
#define DESIGNER_PLUGIN_EXPORT QDESIGNER_WIDGET_EXPORT
#else
#define DESIGNER_PLUGIN_EXPORT
#endif

class QLineEdit;
class QCompleter;
class PathValidator;
class QFileSystemModel;
class QToolButton;

//! The QPathEdit provides a simple way to get a path from the user as comfortable as possible
class DESIGNER_PLUGIN_EXPORT QPathEdit : public QWidget
{
	Q_OBJECT

	//! Defines the Widgets appereance
	Q_PROPERTY(Style style READ style WRITE setStyle)
	//! Holds the icon to be used for the edits button
	Q_PROPERTY(QIcon dialogButtonIcon READ dialogButtonIcon WRITE setDialogButtonIcon)
	//! Specifies the kind of path to be entered
	Q_PROPERTY(PathMode pathMode READ pathMode WRITE setPathMode RESET clear)
	//! Options for the QFileDialog
	Q_PROPERTY(QFileDialog::Options dialogOptions READ dialogOptions WRITE setDialogOptions)
	//! Specifies whether the path can be manually entered or not
	Q_PROPERTY(bool editable READ isEditable WRITE setEditable)
	//! Specifies whether an empty path is allowed or not
	Q_PROPERTY(bool allowEmptyPath READ isEmptyPathAllowed WRITE setAllowEmptyPath)
	//! Turns the auto-completer for manual editing on and off
	Q_PROPERTY(bool useCompleter READ useCompleter WRITE setUseCompleter)
	//! Holds the default directory for the QFileDialog
	Q_PROPERTY(QString defaultDirectory READ defaultDirectory WRITE setDefaultDirectory)
	//! Holds the currently entered, valid path
	Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
	//! Specifiy a placeholder to be shown if no path is entered
	Q_PROPERTY(QString placeholder READ placeholder WRITE setPlaceholder)
	//! Holds name filters for the dialog and the completer
	Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)

public:
	//! Descibes various styles that the edit can take
	enum Style {
		SeperatedButton,//!< The button to open the dialog will be place next to the edit
		JoinedButton,//!< The button to open the dialog will be placed inside the edit
		NoButton,//!< No button will be displayed. However, if the user double-clicks the edit, the dialog will show
		NoDialog//!< The QFileDialog is completly disabled and will never show. Thus, there is no button either
	};
	Q_ENUM(Style)

	//! Describes modes for the kind of path
	enum PathMode {
		ExistingFile,//!< A single, existings file. This is basically "Open file"
		ExistingFolder,//!< A single, existing directory. This is basically "Open Folder"
		AnyFile//!< A single, valid file, no matter if exisiting or not (the directory, however, must exist). This is basically "Save File"
	};
	Q_ENUM(PathMode)

	//! Constructs a new QPathEdit widget. The mode will be QPathEdit::ExistingFile
	explicit QPathEdit(QWidget *parent = NULL, Style style = SeperatedButton);
	//! Constructs a new QPathEdit widget
	explicit QPathEdit(PathMode pathMode, QWidget *parent = NULL, Style style = SeperatedButton);
	//! Constructs a new QPathEdit widget with the given default directory
	explicit QPathEdit(PathMode pathMode, QString defaultDirectory, QWidget *parent = NULL, Style style = SeperatedButton);

	//! READ-ACCESSOR for pathMode
	PathMode pathMode() const;
	//! READ-ACCESSOR for dialogOptions
	QFileDialog::Options dialogOptions() const;
	//! READ-ACCESSOR for allowEmptyPath
	bool isEmptyPathAllowed() const;
	//! READ-ACCESSOR for defaultDirectory
	QString defaultDirectory() const;
	//! READ-ACCESSOR for path
	QString path() const;
	//! Returns the entered path as an QUrl
	QUrl pathUrl() const;
	//! READ-ACCESSOR for placeholder
	QString placeholder() const;
	//! READ-ACCESSOR for nameFilters
	QStringList nameFilters() const;
	//! READ-ACCESSOR for editable
	bool isEditable() const;
	//! READ-ACCESSOR for useCompleter
	bool useCompleter() const;
	//! READ-ACCESSOR for style
	Style style() const;
	//! READ-ACCESSOR for dialogButtonIcon
	QIcon dialogButtonIcon();

	//! WRITE-ACCESSOR for pathMode
	void setPathMode(PathMode pathMode);
	//! WRITE-ACCESSOR for dialogOptions
	void setDialogOptions(QFileDialog::Options dialogOptions);
	//! WRITE-ACCESSOR for allowEmptyPath
	void setAllowEmptyPath(bool allowEmptyPath);
	//! WRITE-ACCESSOR for defaultDirectory
	void setDefaultDirectory(QString defaultDirectory);
	//! WRITE-ACCESSOR for path
	bool setPath(QString path, bool allowInvalid = false);
	//! RESET-ACCESSOR for path
	void clear();
	//! WRITE-ACCESSOR for placeholder
	void setPlaceholder(QString placeholder);
	//! WRITE-ACCESSOR for nameFilters
	void setNameFilters(QStringList nameFilters);
	//! WRITE-ACCESSOR for editable
	void setEditable(bool editable);
	//! WRITE-ACCESSOR for useCompleter
	void setUseCompleter(bool useCompleter);
	//! WRITE-ACCESSOR for style
	void setStyle(Style style);
	//! WRITE-ACCESSOR for dialogButtonIcon
	void setDialogButtonIcon(const QIcon &icon);

public slots:
	//! Shows the QFileDialog so the user can select a path
	void showDialog();

signals:
	//! NOTIFY-ACCESSOR for pathMode
	void pathChanged(QString path);

private slots:
	void editTextUpdate(const QString &path);

private:
	QLineEdit *edit;
	QCompleter *pathCompleter;
	QFileSystemModel *completerModel;
	PathValidator *pathValidator;
	QFileDialog *dialog;

	QString currentValidPath;
	bool wasPathValid;

	Style uiStyle;
	QIcon diagIcon;
	PathMode mode;
	QString defaultDir;
	bool allowEmpty;

	QPointer<QToolButton> currentToolButton;
	QObject *setupResetObject;

	void setupUiSeperate();
	void setupUiJoined();
	void setupUiWithout();
	void setupReset();

	QStringList modelFilters(const QStringList &normalFilters);
};

#endif // QPATHEDIT_H
