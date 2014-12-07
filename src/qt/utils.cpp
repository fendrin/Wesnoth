#include "qt/utils.hpp"

#include <qt5/QtQuick/QQuickView>
#include <qt5/QtGui/QGuiApplication>
#include <qt5/QtQml/QQmlApplicationEngine>
#include <qt5/QtCore/QString>

namespace qt {

int show_application(const std::string& qml_file_path, int argc, char** argv) {

	QGuiApplication app(argc, argv);
	QQmlApplicationEngine engine(QString(qml_file_path.c_str()));

	return app.exec();
}

void show_quick(const std::string& qml_file_path) {

	int argc = 0;
	QGuiApplication app(argc, NULL);
	QQuickView view;
	view.setSource(QUrl::fromLocalFile(QString(qml_file_path.c_str())));
	view.show();
	app.exec();
}


} // End namespace qt

