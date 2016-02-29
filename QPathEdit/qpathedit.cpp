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
#include <functional>

//HELPER CLASSES

#ifdef Q_OS_WIN32
#define SEP_OFFSET 2
#else
#define SEP_OFFSET 0
#endif

#ifdef Q_OS_WIN32
#define JOIN_OFFSET 1
#else
#define JOIN_OFFSET 2
#endif

class JoinedToolButton : public QToolButton
{
public:
	JoinedToolButton(QWidget *parent);
	void fitPos();
protected:
	bool eventFilter(QObject *, QEvent *event) Q_DECL_OVERRIDE;
};

class ClickEventListener : public QObject
{
public:
	ClickEventListener(std::function<void()> func, QObject *parent);
protected:
	bool eventFilter(QObject *, QEvent *event) Q_DECL_OVERRIDE;
private:
	std::function<void()> func;
};

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
	diagIcon(),
	mode(ExistingFile),
	defaultDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)),
	allowEmpty(true),
	currentToolButton(NULL),
	setupResetObject(NULL)
{
	//setup dialog
	this->dialog->setOptions(0);
	this->dialog->setWindowModality(Qt::WindowModal);
	this->dialog->setWindowFlags(this->dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	this->setPathMode(pathMode);
	connect(this->dialog, &QFileDialog::fileSelected, this, &QPathEdit::dialogFileSelected);

	//setup completer
	this->completerModel->setRootPath("");
	connect(this->completerModel, &QFileSystemModel::directoryLoaded, this->pathCompleter, [this](QString){
		this->pathCompleter->complete();
	});
	this->pathCompleter->setModel(this->completerModel);

	//setup lineedit
	this->edit->setCompleter(this->pathCompleter);
	this->edit->setValidator(this->pathValidator);
	this->edit->setDragEnabled(true);
	this->edit->setReadOnly(true);
	connect(this->edit, &QLineEdit::editingFinished, this, &QPathEdit::editTextUpdate);
	connect(this->edit, &QLineEdit::textChanged, this, &QPathEdit::updateValidInfo);
	//setup this
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(QMargins());
	layout->setSpacing(0);
	layout->addWidget(this->edit);
	this->setLayout(layout);

	//setup "button"
	switch(style) {
	case SeperatedButton:
		this->setupUiSeperate();
		break;
	case JoinedButton:
		this->setupUiJoined();
		break;
	case NoButton:
		this->setupUiWithout();
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

	this->setupReset();
	switch(style) {
	case SeperatedButton:
		this->setupUiSeperate();
		break;
	case JoinedButton:
		this->setupUiJoined();
		break;
	case NoButton:
		this->setupUiWithout();
		break;
	default:
		break;
	}
	this->uiStyle = style;
}

QIcon QPathEdit::dialogButtonIcon()
{
	return this->diagIcon;
}

void QPathEdit::setDialogButtonIcon(const QIcon &icon)
{
	this->diagIcon = icon;
	if(!this->currentToolButton.isNull())
		this->currentToolButton->setIcon(icon);
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
		QFileInfo info(oldPath);
		if(this->mode == ExistingFolder)
			this->dialog->setDirectory(info.dir());
		else {
			if(info.isDir())
				this->dialog->setDirectory(oldPath);
			else
				this->dialog->setDirectory(info.dir());
		}
	}

	this->dialog->open();
}

void QPathEdit::updateValidInfo(const QString &)
{
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

void QPathEdit::setupUiSeperate()
{
	QToolButton *tbtn = new QToolButton(this);
	this->setupResetObject = tbtn;
	this->currentToolButton = tbtn;

	tbtn->setText("…");
	tbtn->setIcon(this->diagIcon);
    tbtn->setFixedHeight(this->edit->sizeHint().height() + SEP_OFFSET);
	connect(tbtn, &QToolButton::clicked, this, &QPathEdit::showDialog);

	//setup this
	this->layout()->addWidget(tbtn);
}

void QPathEdit::setupUiJoined()
{
	JoinedToolButton *tbtn = new JoinedToolButton(this->edit);
	this->setupResetObject = tbtn;
	this->currentToolButton = tbtn;

	//create button
	tbtn->setText("…");
	tbtn->setIcon(this->diagIcon);
	tbtn->setAutoRaise(true);
    tbtn->setFixedHeight(this->edit->sizeHint().height() - JOIN_OFFSET);
	tbtn->setMouseTracking(true);
	QCursor cursor = tbtn->cursor();
	cursor.setShape(Qt::ArrowCursor);
	tbtn->setCursor(cursor);
	connect(tbtn, &QToolButton::clicked, this, &QPathEdit::showDialog);

	//update lineedit to contain the button
	this->edit->setTextMargins(0, 0, tbtn->sizeHint().width(), 0);
	this->edit->installEventFilter(tbtn);

	//start timer to move button
    QTimer::singleShot(0, this, [tbtn, this](){
		if(this->uiStyle == JoinedButton){
            tbtn->show();
            tbtn->fitPos();
		}
	});
}

void QPathEdit::setupUiWithout()
{
	//install click listener
	ClickEventListener *clicker = new ClickEventListener([this](){
		this->showDialog();
	}, this);
	this->edit->installEventFilter(clicker);
	this->setupResetObject = clicker;
}

void QPathEdit::setupReset()
{
	delete this->setupResetObject;
	this->setupResetObject = NULL;
	this->edit->setTextMargins(QMargins());
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

//HELPER CLASSES IMPLEMENTATION

JoinedToolButton::JoinedToolButton(QWidget *parent) :
	QToolButton(parent)
{}

void JoinedToolButton::fitPos() {
	this->move(this->parentWidget()->width() - this->width() - 1, 1);
}

bool JoinedToolButton::eventFilter(QObject *, QEvent *event)
{
	if(event->type() == QEvent::Move || event->type() == QEvent::Resize)
		this->fitPos();
	return false;
}

ClickEventListener::ClickEventListener(std::function<void ()> func, QObject *parent) :
	QObject(parent),
	func(func)
{}

bool ClickEventListener::eventFilter(QObject *, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonDblClick)
		this->func();
	return false;
}

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
