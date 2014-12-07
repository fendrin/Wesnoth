/*
Lifted from the qt documentation.
I consider it to be too trivial to cause copyright issues.

This gui element can be drawn with the help of the QQuickView class.
*/

import QtQuick 2.3;

Rectangle {
    id: page
    width: 320; height: 480
    color: "lightgray"

    Text {
        id: helloText
        text: "Hello world!"
        y: 30
        anchors.horizontalCenter: page.horizontalCenter
        font.pointSize: 24; font.bold: true
    }
}

