#include "gui_qt/utils.hpp"

#include "widget.hpp"

#include <QtQuick/QQuickView>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtCore/QString>

namespace gui_qt {

int show_application(const std::string& qml_file_path,
                     int argc, char** argv) {

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine(QString(qml_file_path.c_str()));

//    qmlRegisterType<gui_qt::widget>("com.myself", 1, 0, "MyWidget");

    return app.exec();
}

void show_quick(const std::string& qml_file_path) {

    int argc = 0;
    QGuiApplication app(argc, NULL);
    QQuickView view;
    view.setSource(QUrl::fromLocalFile(
                       QString(qml_file_path.c_str())));
    view.show();
    app.exec();
}


} // End namespace qt

