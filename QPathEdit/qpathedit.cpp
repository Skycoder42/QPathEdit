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
#include <QKeyEvent>

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
	dialogAction(new QAction(getDefaultIcon(), tr("Open File-Dialog"), this)),
	hasCustomIcon(false)
{
	//setup dialog
	dialog->setOptions(0);
	masterDialog(dialog);
	setPathMode(pathMode);
	connect(dialog, &QFileDialog::fileSelected, this, &QPathEdit::dialogFileSelected);

	//setup completer
	completerModel->setRootPath("");
	completerModel->setNameFilterDisables(false);
	connect(completerModel, &QFileSystemModel::directoryLoaded, pathCompleter, [this](QString){
		pathCompleter->complete();
	});
	pathCompleter->setModel(completerModel);

	//setup this
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(QMargins());
	layout->setSpacing(0);
	layout->addWidget(edit);
	layout->addWidget(toolButton);
	setLayout(layout);
	//setup lineedit
	edit->installEventFilter(this);
	edit->setCompleter(pathCompleter);
	edit->setValidator(pathValidator);
	edit->setDragEnabled(true);
	edit->setReadOnly(true);
	connect(edit, &QLineEdit::editingFinished, this, &QPathEdit::editTextUpdate);
	connect(edit, &QLineEdit::textChanged, this, &QPathEdit::updateValidInfo);
	//setup "button"
	connect(dialogAction, &QAction::triggered, this, &QPathEdit::showDialog);
	toolButton->setDefaultAction(dialogAction);
	int height = edit->sizeHint().height();
#ifdef Q_OS_WIN
	height += 2;
#endif
	toolButton->setFixedSize(height, height);
	switch(style) {
	case JoinedButton:
		edit->addAction(dialogAction, QLineEdit::TrailingPosition);
	case NoButton:
		toolButton->setVisible(false);
		break;
	default:
		break;
	}

	QWidget::setTabOrder(edit, toolButton);
	setFocusPolicy(edit->focusPolicy());
	setFocusProxy(edit);
}

QPathEdit::QPathEdit(QPathEdit::PathMode pathMode, QString defaultDirectory, QWidget *parent, QPathEdit::Style style) :
	QPathEdit(pathMode, parent, style)
{
	setDefaultDirectory(defaultDirectory);
}

QPathEdit::PathMode QPathEdit::pathMode() const
{
	return mode;
}

void QPathEdit::setPathMode(PathMode pathMode)
{
	mode = pathMode;
	pathValidator->setMode(pathMode);
	currentValidPath.clear();
	emit pathChanged(QString());
	edit->clear();
	switch(pathMode) {
	case ExistingFile:
		dialog->setAcceptMode(QFileDialog::AcceptOpen);
		dialog->setFileMode(QFileDialog::ExistingFile);
		completerModel->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);
		break;
	case ExistingFolder:
		dialog->setAcceptMode(QFileDialog::AcceptOpen);
		dialog->setFileMode(QFileDialog::Directory);
		completerModel->setFilter(QDir::Drives | QDir::Dirs | QDir::NoDotAndDotDot);
		break;
	case AnyFile:
		dialog->setAcceptMode(QFileDialog::AcceptSave);
		dialog->setFileMode(QFileDialog::AnyFile);
		completerModel->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);
		break;
	default:
		Q_UNREACHABLE();
	}
}

QFileDialog::Options QPathEdit::dialogOptions() const
{
	return dialog->options();
}

void QPathEdit::setDialogOptions(QFileDialog::Options dialogOptions)
{
	dialog->setOptions(dialogOptions);
}

bool QPathEdit::isEmptyPathAllowed() const
{
	return allowEmpty;
}

void QPathEdit::setAllowEmptyPath(bool allowEmptyPath)
{
	allowEmpty = allowEmptyPath;
	pathValidator->setAllowEmpty(allowEmptyPath);
}

QString QPathEdit::defaultDirectory() const
{
	return defaultDir;
}

void QPathEdit::setDefaultDirectory(QString defaultDirectory)
{
	defaultDir = defaultDirectory;
}

QString QPathEdit::path() const
{
	return currentValidPath;
}

QUrl QPathEdit::pathUrl() const
{
	return QUrl::fromLocalFile(currentValidPath);
}

bool QPathEdit::setPath(QString path, bool allowInvalid)
{
	if (edit->text() == path)
		return true;

	if(allowInvalid)
		edit->setText(path);

	int pseudo = 0;
	if(pathValidator->validate(path, pseudo) == QValidator::Acceptable) {
		currentValidPath = path.replace(QStringLiteral("\\"), QStringLiteral("/"));
		if(!allowInvalid)
			edit->setText(path);
		emit pathChanged(path);
		return true;
	} else
		return false;
}

void QPathEdit::clear()
{
	edit->clear();
	currentValidPath.clear();
	emit pathChanged(QString());
}

QString QPathEdit::placeholder() const
{
	return edit->placeholderText();
}

void QPathEdit::setPlaceholder(QString placeholder)
{
	edit->setPlaceholderText(placeholder);
}

QStringList QPathEdit::nameFilters() const
{
	return dialog->nameFilters();
}

void QPathEdit::setNameFilters(QStringList nameFilters)
{
	dialog->setNameFilters(nameFilters);
	QStringList tmp = modelFilters(nameFilters);
	completerModel->setNameFilters(tmp);
}

QStringList QPathEdit::mimeTypeFilters() const
{
	return dialog->mimeTypeFilters();
}

void QPathEdit::setMimeTypeFilters(QStringList mimeFilters)
{
	dialog->setMimeTypeFilters(mimeFilters);
	QStringList tmp = modelFilters(dialog->nameFilters());
	completerModel->setNameFilters(tmp);
}

bool QPathEdit::isEditable() const
{
	return !edit->isReadOnly();
}

void QPathEdit::setEditable(bool editable)
{
	edit->setReadOnly(!editable);
}

bool QPathEdit::useCompleter() const
{
	return edit->completer();
}

void QPathEdit::setUseCompleter(bool useCompleter)
{
	edit->setCompleter(useCompleter ? pathCompleter : nullptr);
}

QPathEdit::Style QPathEdit::style() const
{
	return uiStyle;
}

void QPathEdit::setStyle(QPathEdit::Style style, QLineEdit::ActionPosition position)
{
	if (uiStyle == style)
		return;

	switch(style) {
	case SeperatedButton:
		edit->removeAction(dialogAction);
		toolButton->setVisible(true);
		break;
	case JoinedButton:
		edit->addAction(dialogAction, position);
		toolButton->setVisible(false);
		break;
	case NoButton:
		edit->removeAction(dialogAction);
		toolButton->setVisible(false);
		break;
	default:
		Q_UNREACHABLE();
		break;
	}

	uiStyle = style;
	if(!hasCustomIcon)
		dialogAction->setIcon(getDefaultIcon());
}

QIcon QPathEdit::dialogButtonIcon() const
{
	return dialogAction->icon();
}

void QPathEdit::setDialogButtonIcon(const QIcon &icon)
{
	dialogAction->setIcon(icon);
	hasCustomIcon = true;
}

void QPathEdit::resetDialogButtonIcon()
{
	dialogAction->setIcon(QPathEdit::getDefaultIcon());
	hasCustomIcon = false;
}

void QPathEdit::showDialog()
{
	if(dialog->isVisible()) {
		dialog->raise();
		dialog->activateWindow();
		return;
	}

	QString oldPath = edit->text();
	if(oldPath.isEmpty())
		dialog->setDirectory(defaultDir);
	else {
		if(mode == ExistingFolder)
			dialog->setDirectory(oldPath);
		else {
			QFileInfo info(oldPath);
			if(info.isDir())
				dialog->setDirectory(oldPath);
			else {
				dialog->setDirectory(info.dir());
				dialog->selectFile(info.fileName());
			}
		}
	}

	dialog->open();
}

void QPathEdit::updateValidInfo(const QString &path)
{
	completerModel->index(QFileInfo(path).dir().absolutePath());//enforce "directory loading"
	if(edit->hasAcceptableInput()) {
		if(!wasPathValid) {
			wasPathValid = true;
			edit->setPalette(palette());
		}
	} else {
		if(wasPathValid) {
			wasPathValid = false;
			QPalette pal = palette();
			pal.setColor(QPalette::Text, QColor(QStringLiteral("#B40404")));
			edit->setPalette(pal);
		}
	}
}

void QPathEdit::editTextUpdate()
{
	if(edit->hasAcceptableInput()) {
		QString newPath = edit->text().replace(QStringLiteral("\\"), QStringLiteral("/"));
		if(currentValidPath != newPath) {
			currentValidPath = newPath;
			emit pathChanged(currentValidPath);
		}
	}
}

void QPathEdit::dialogFileSelected(const QString &file)
{
	if(!file.isEmpty()) {
		edit->setText(dialog->selectedFiles()
							.first()
							.replace(QStringLiteral("\\"), QStringLiteral("/")));
		editTextUpdate();
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
	switch(uiStyle) {
	case SeperatedButton: {
		QImage image(16, 16, QImage::Format_ARGB32);
		image.fill(Qt::transparent);
		QPainter painter(&image);
		painter.setFont(font());
		painter.setPen(palette().color(QPalette::ButtonText));
		painter.drawText(QRect(0, 0, 16, 16), Qt::AlignCenter, "…");
		return QPixmap::fromImage(image);
	}
	case JoinedButton:
		return QIcon(QStringLiteral(":/qpathedit/icons/dialog.ico"));
	case NoButton:
		return QIcon();
	default:
		Q_UNREACHABLE();
	}
}

bool QPathEdit::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::KeyPress){
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if(keyEvent->key() == Qt::Key_Space &&
				keyEvent->modifiers() == Qt::ControlModifier){
			pathCompleter->complete();
			return true;
		} else
			return QObject::eventFilter(watched, event);
	} else
		return QObject::eventFilter(watched, event);
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
	allowEmpty = allow;
}

QValidator::State PathValidator::validate(QString &text, int &) const
{
	//check if empty is accepted
	if(text.isEmpty())
		return allowEmpty ? QValidator::Acceptable : QValidator::Intermediate;

	//nonexisting parent dir is not possible
	QFileInfo pathInfo(text);
	if(!pathInfo.dir().exists())
		return QValidator::Invalid;

	switch(mode) {
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

