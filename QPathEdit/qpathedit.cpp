#include "qpathedit.h"
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QCompleter>
#include <QValidator>
#include <QStandardPaths>
#include <QFileSystemModel>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QEvent>
#include <QTimer>
#include <QAction>
#include <QPainter>
#include <functional>

//HELPER CLASSES

void masterDialog(QDialog *dialog);

class PathValidator : public QValidator
{
public:
	PathValidator(QObject *parent);
	void setMode(QPathEdit::PathMode mode);
	void setAllowEmpty(bool allow);
	State validate(QString &text, int &) const Q_DECL_OVERRIDE;
private:
	QPathEdit::PathMode mode;
	bool allowEmpty;
};

//QPATHEDIT IMPLEMENTATION

QPathEdit::QPathEdit(QWidget *parent, QPathEdit::Style style) :
	QPathEdit(ExistingFile, parent, style)
{}

QPathEdit::QPathEdit(QPathEdit::PathMode pathMode, QWidget *parent, QPathEdit::Style style) :
	QWidget(parent),
	edit(new QLineEdit(this)),
	pathCompleter(new QCompleter(this)),
	completerModel(new QFileSystemModel(this)),
	pathValidator(new PathValidator(this)),
	dialog(new QFileDialog(this)),
	currentValidPath(),
	wasPathValid(true),
	uiStyle(style),
	mode(ExistingFile),
	defaultDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)),
	allowEmpty(true),
	toolButton(new QToolButton(this)),
	dialogAction(new QAction(this->getDefaultIcon(), tr("Open File-Dialog"), this)),
	hasCustomIcon(false)
{
	//setup dialog
	this->dialog->setOptions(0);
	masterDialog(this->dialog);
	this->setPathMode(pathMode);
	connect(this->dialog, &QFileDialog::fileSelected, this, &QPathEdit::dialogFileSelected);

	//setup completer
	this->completerModel->setRootPath("");
	this->completerModel->setNameFilterDisables(false);
	connect(this->completerModel, &QFileSystemModel::directoryLoaded, this->pathCompleter, [this](QString){
		this->pathCompleter->complete();
	});
	this->pathCompleter->setModel(this->completerModel);

	//setup this
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(QMargins());
	layout->setSpacing(0);
	layout->addWidget(this->edit);
	layout->addWidget(this->toolButton);
	this->setLayout(layout);
	//setup lineedit
	this->edit->setCompleter(this->pathCompleter);
	this->edit->setValidator(this->pathValidator);
	this->edit->setDragEnabled(true);
	this->edit->setReadOnly(true);
	connect(this->edit, &QLineEdit::editingFinished, this, &QPathEdit::editTextUpdate);
	connect(this->edit, &QLineEdit::textChanged, this, &QPathEdit::updateValidInfo);
	//setup "button"
	connect(this->dialogAction, &QAction::triggered, this, &QPathEdit::showDialog);
	this->toolButton->setDefaultAction(this->dialogAction);
	int height = this->edit->sizeHint().height();
	this->toolButton->setFixedSize(height, height);
	switch(style) {
	case JoinedButton:
		this->edit->addAction(this->dialogAction, QLineEdit::TrailingPosition);
	case NoButton:
		this->toolButton->setVisible(false);
		break;
	default:
		break;
	}
}

QPathEdit::QPathEdit(QPathEdit::PathMode pathMode, QString defaultDirectory, QWidget *parent, QPathEdit::Style style) :
	QPathEdit(pathMode, parent, style)
{
	this->setDefaultDirectory(defaultDirectory);
}

QPathEdit::PathMode QPathEdit::pathMode() const
{
	return this->mode;
}

void QPathEdit::setPathMode(PathMode pathMode)
{
	this->mode = pathMode;
	this->pathValidator->setMode(pathMode);
	this->currentValidPath.clear();
	emit pathChanged(QString());
	this->edit->clear();
	switch(pathMode) {
	case ExistingFile:
		this->dialog->setAcceptMode(QFileDialog::AcceptOpen);
		this->dialog->setFileMode(QFileDialog::ExistingFile);
		this->completerModel->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);
		break;
	case ExistingFolder:
		this->dialog->setAcceptMode(QFileDialog::AcceptOpen);
		this->dialog->setFileMode(QFileDialog::Directory);
		this->completerModel->setFilter(QDir::Drives | QDir::Dirs | QDir::NoDotAndDotDot);
		break;
	case AnyFile:
		this->dialog->setAcceptMode(QFileDialog::AcceptSave);
		this->dialog->setFileMode(QFileDialog::AnyFile);
		this->completerModel->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);
		break;
	default:
		Q_UNREACHABLE();
	}
}

QFileDialog::Options QPathEdit::dialogOptions() const
{
	return this->dialog->options();
}

void QPathEdit::setDialogOptions(QFileDialog::Options dialogOptions)
{
	this->dialog->setOptions(dialogOptions);
}

bool QPathEdit::isEmptyPathAllowed() const
{
	return this->allowEmpty;
}

void QPathEdit::setAllowEmptyPath(bool allowEmptyPath)
{
	this->allowEmpty = allowEmptyPath;
	this->pathValidator->setAllowEmpty(allowEmptyPath);
}

QString QPathEdit::defaultDirectory() const
{
	return this->defaultDir;
}

void QPathEdit::setDefaultDirectory(QString defaultDirectory)
{
	this->defaultDir = defaultDirectory;
}

QString QPathEdit::path() const
{
	return this->currentValidPath;
}

QUrl QPathEdit::pathUrl() const
{
	return QUrl::fromLocalFile(this->currentValidPath);
}

bool QPathEdit::setPath(QString path, bool allowInvalid)
{
	if (this->edit->text() == path)
		return true;

	if(allowInvalid)
		this->edit->setText(path);

	int pseudo = 0;
	if(this->pathValidator->validate(path, pseudo) == QValidator::Acceptable) {
		this->currentValidPath = path.replace(QStringLiteral("\\"), QStringLiteral("/"));
		if(!allowInvalid)
			this->edit->setText(path);
		emit pathChanged(path);
		return true;
	} else
		return false;
}

void QPathEdit::clear()
{
	this->edit->clear();
	this->currentValidPath.clear();
	emit pathChanged(QString());
}

QString QPathEdit::placeholder() const
{
	return this->edit->placeholderText();
}

void QPathEdit::setPlaceholder(QString placeholder)
{
	this->edit->setPlaceholderText(placeholder);
}

QStringList QPathEdit::nameFilters() const
{
	return this->dialog->nameFilters();
}

void QPathEdit::setNameFilters(QStringList nameFilters)
{
	this->dialog->setNameFilters(nameFilters);
	QStringList tmp = this->modelFilters(nameFilters);
	this->completerModel->setNameFilters(tmp);
}

QStringList QPathEdit::mimeTypeFilters() const
{
	return this->dialog->mimeTypeFilters();
}

void QPathEdit::setMimeTypeFilters(QStringList mimeFilters)
{
	this->dialog->setMimeTypeFilters(mimeFilters);
	QStringList tmp = this->modelFilters(this->dialog->nameFilters());
	this->completerModel->setNameFilters(tmp);
}

bool QPathEdit::isEditable() const
{
	return !this->edit->isReadOnly();
}

void QPathEdit::setEditable(bool editable)
{
	this->edit->setReadOnly(!editable);
}

bool QPathEdit::useCompleter() const
{
	return this->edit->completer();
}

void QPathEdit::setUseCompleter(bool useCompleter)
{
	this->edit->setCompleter(useCompleter ? this->pathCompleter : NULL);
}

QPathEdit::Style QPathEdit::style() const
{
	return this->uiStyle;
}

void QPathEdit::setStyle(QPathEdit::Style style)
{
	if (this->uiStyle == style)
		return;

	switch(style) {
	case SeperatedButton:
		this->edit->removeAction(this->dialogAction);
		this->toolButton->setVisible(true);
		break;
	case JoinedButton:
		this->edit->addAction(this->dialogAction, QLineEdit::TrailingPosition);
		this->toolButton->setVisible(false);
		break;
	case NoButton:
		this->edit->removeAction(this->dialogAction);
		this->toolButton->setVisible(false);
	default:
		break;
	}

	this->uiStyle = style;
	if(!this->hasCustomIcon)
		this->dialogAction->setIcon(this->getDefaultIcon());
}

QIcon QPathEdit::dialogButtonIcon()
{
	return this->dialogAction->icon();
}

void QPathEdit::setDialogButtonIcon(const QIcon &icon)
{
	this->dialogAction->setIcon(icon);
	this->hasCustomIcon = true;
}

void QPathEdit::resetDialogButtonIcon()
{
	this->dialogAction->setIcon(QPathEdit::getDefaultIcon());
	this->hasCustomIcon = false;
}

void QPathEdit::showDialog()
{
	if(this->dialog->isVisible()) {
		this->dialog->raise();
		this->dialog->activateWindow();
		return;
	}

	QString oldPath = this->edit->text();
	if(oldPath.isEmpty())
		this->dialog->setDirectory(this->defaultDir);
	else {
		if(this->mode == ExistingFolder)
			this->dialog->setDirectory(oldPath);
		else {
			QFileInfo info(oldPath);
			if(info.isDir())
				this->dialog->setDirectory(oldPath);
			else {
				this->dialog->setDirectory(info.dir());
				this->dialog->selectFile(info.fileName());
			}
		}
	}

	this->dialog->open();
}

void QPathEdit::updateValidInfo(const QString &path)
{
	this->completerModel->index(QFileInfo(path).dir().absolutePath());//DEBUG enforce "directory loading"
	if(this->edit->hasAcceptableInput()) {
		if(!this->wasPathValid) {
			this->wasPathValid = true;
			this->edit->setPalette(this->palette());
		}
	} else {
		if(this->wasPathValid) {
			this->wasPathValid = false;
			QPalette pal = this->palette();
			pal.setColor(QPalette::Text, QColor(QStringLiteral("#B40404")));
			this->edit->setPalette(pal);
		}
	}
}

void QPathEdit::editTextUpdate()
{
	if(this->edit->hasAcceptableInput()) {
		QString newPath = this->edit->text().replace(QStringLiteral("\\"), QStringLiteral("/"));
		if(this->currentValidPath != newPath) {
			this->currentValidPath = newPath;
			emit pathChanged(this->currentValidPath);
		}
	}
}

void QPathEdit::dialogFileSelected(const QString &file)
{
	if(!file.isEmpty()) {
		this->edit->setText(this->dialog->selectedFiles()
							.first()
							.replace(QStringLiteral("\\"), QStringLiteral("/")));
		this->editTextUpdate();
	}
}

QStringList QPathEdit::modelFilters(const QStringList &normalFilters)
{
	QStringList res;
	foreach(QString filter, normalFilters) {
		QRegularExpressionMatch match = QRegularExpression(QStringLiteral("^.*\\((.*)\\)$"))
										.match(filter);
		if(match.hasMatch())
			res.append(match.captured(1).split(QRegularExpression(QStringLiteral("\\s"))));
	}
	return res;
}

QIcon QPathEdit::getDefaultIcon()
{
	switch(this->uiStyle) {
	case SeperatedButton: {
		QImage image(16, 16, QImage::Format_ARGB32);
		image.fill(Qt::transparent);
		QPainter painter(&image);
		painter.setFont(this->font());
		painter.setPen(this->palette().color(QPalette::ButtonText));
		painter.drawText(QRect(0, 0, 16, 16), Qt::AlignCenter, "…");
		return QPixmap::fromImage(image);
	}
	case JoinedButton:
		return QIcon(QStringLiteral(":/icons/qpathedit/dialog.ico"));
	case NoButton:
		return QIcon();
	default:
		Q_UNREACHABLE();
	}
}

//HELPER CLASSES IMPLEMENTATION

PathValidator::PathValidator(QObject *parent) :
	QValidator(parent),
	mode(QPathEdit::ExistingFile),
	allowEmpty(true)
{}

void PathValidator::setMode(QPathEdit::PathMode mode)
{
	this->mode = mode;
}

void PathValidator::setAllowEmpty(bool allow)
{
	this->allowEmpty = allow;
}

QValidator::State PathValidator::validate(QString &text, int &) const
{
	//check if empty is accepted
	if(text.isEmpty())
		return this->allowEmpty ? QValidator::Acceptable : QValidator::Intermediate;

	//nonexisting parent dir is not possible
	QFileInfo pathInfo(text);
	if(!pathInfo.dir().exists())
		return QValidator::Invalid;

	switch(this->mode) {
	case QPathEdit::AnyFile://acceptable, as long as it's not an directoy
		if(pathInfo.isDir())
			return QValidator::Intermediate;
		else
			return QValidator::Acceptable;
	case QPathEdit::ExistingFile://must be an existing file
		if(pathInfo.exists() && pathInfo.isFile())
			return QValidator::Acceptable;
		else
			return QValidator::Intermediate;
	case QPathEdit::ExistingFolder://must be an existing folder
		if(pathInfo.exists() && pathInfo.isDir())
			return QValidator::Acceptable;
		else
			return QValidator::Intermediate;
	default:
		Q_UNREACHABLE();
	}

	return QValidator::Invalid;
}

void masterDialog(QDialog *dialog)
{
	Qt::WindowFlags flags = Qt::WindowTitleHint |
							Qt::WindowSystemMenuHint |
							Qt::WindowCloseButtonHint;

	dialog->setSizeGripEnabled(true);
	if(dialog->parentWidget()) {
		dialog->setWindowModality(Qt::WindowModal);
		flags |= Qt::Sheet;
	} else {
		dialog->setWindowModality(Qt::ApplicationModal);
		flags |= Qt::Dialog;
	}
	dialog->setWindowFlags(flags);
}

