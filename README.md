# QPathEdit
A Qt-Widget to get local file and folder-paths in an optimized and simple way.

## Introduction
The QPathEdit class was designed to provide a widget that allows the user to easily enter or select a local file or directory path. The widget comes with a plugin for QtCreator.

## Details
### The QPathEdit Class
The QPathEdit class provides the following features:
 - **Appereance:** The Edit is composed of a QLineEdit and QToolButton. It uses natives styles. It should look "normal" on any plattform. It provides 3 different styles to allows customisation of the way these components are displayed.
 - **Dialog:** The user will be able to select his path using QFileDialog. The dialog can be customized to a limited amount.
 - **Validation:** The edit automatically validates entered and selected paths and makes shure, that it always returns a valid path to your program.
 - **Editable:** The edit can be editable or readonly. If editable, the validator will mark invalid paths and an auto completer will simplify path entering.
 - **Completer:** The edit uses an auto completer, that uses the underlying file-system to provide paths as autocompletion to make entering paths even easiert. It can be turned on and off.
 - **QtCreator Plugin:** The edit widget comes with an designer-plugin for Qt-Creator, to make it possible to user the pathEdit insider .ui files.
 
You can check out the documentation by clicking on QPathEdit, or download the files below.

### Usage - A simple Example
Since the QPathEdit is just a simple widget, you can use it like any other widget.
The following example will create a simple window with the QPathEdit inside. If the user enters a path, the new path will be logged on the console:

```cpp
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPathEdit>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   
   QWidget window;
   QVBoxLayout *layout = new QVBoxLayout(&window);
   QPathEdit *edit = new QPathEdit(&window);
   layout->addWidget(edit);
   window.setLayout(layout);
   
   QObject::connect(edit, &QPathEdit::pathChanged, [](QString path){
       qDebug() << path;
   });
   
   window.show();
   return a.exec();
}
```

### Installing the Plugin
To install the plugin, you need to copy the right file from the zip-package to the QtCreators designer plugin path. Inside the `designerplugin` folder, there are a number of subfolders for operating systems I've created the plugin for. If yours is not present, you need to compile the plugin yourself. Copy file (for example `qpatheditplugin.dll`) into QtCreators path. The default path would be:
- Windows: `<path_to_qt>/Tools/QtCreator/bin/plugins/designer`
- OsX: `<qt_creator_bundle>/Contents/PlugIns/designer`
- Linux: `<path_to_qt>/Tools/QtCreator/lib/Qt/plugins/designer` 

After restarting the creator, navigate to the designer and to "Tools > Form Editor > About Qt Designer Plugins". The plugin should appear there. You can find it inside the "Input Widgets" Section.

For more details, check [Adding Qt Designer Plugins](http://doc.qt.io/qtcreator/adding-plugins.html).

## Documentation
The documentation is available within the releases and on [github pages](https://skycoder42.github.io/QPathEdit/).

The documentation was created using [doxygen](www.doxygen.org/). It includes an HTML-documentation and Qt-Help files that can be included into QtCreator (QtAssistant) to show F1-Help (See [Adding External Documentation](https://doc.qt.io/qtcreator/creator-help.html#adding-external-documentation) for more details).

## Download
All downloads are located at [github release](https://github.com/Skycoder42/QPathEdit/releases). The downloads include:
- A .pri file for easy project includes (Just add `include(<path_to>/qpathedit.pri)` to your project and all the includes/libray imports/... will be done by that file!)
- The QPathEdit-class and a resource file with the icon (automatically included by the pri-file)
- The HTML and QtHelp documentation
- QtCreator-Plugins, all build with Qt 5.6 and working with at least the current version (3.6.1), for: (optional)
  - Windows
  - OsX
  - X11 (x64 only)
- Readme and license

## Icons
The edit uses a default icon for joined dialog buttons. The icon was downloaded from http://www.fatcow.com/free-icons.
