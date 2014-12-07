/*
qml example dialog
Lifted from the qt documentation wiki.

I suggest it to be too trivial to cause any copyright problems.
*/
import QtQuick 2.3;
import QtQuick.Dialogs 1.2;
import QtQuick.Controls 1.2

FileDialog {
    id: fileChooser
    title: "Load Savegame"
    onAccepted: {
        console.log("fileChooser: Game file: " + fileChooser.fileUrls + " selected.")
        Qt.quit()
    }
    onRejected: {
        console.log("fileChooser: Dialog canceled")
        Qt.quit()
    }
    Component.onCompleted: visible = true
}

