import QtQuick 2.3;
import QtQuick.Dialogs 1.2;
import QtQuick.Controls 1.2

ApplicationWindow {
    visible: true
    title: "The Battle for Wesnoth"

    width: 1024
    height: 768

    minimumWidth: 800
    minimumHeight: 480

    menuBar: MenuBar {
        Menu {
            id: menu
            title: "&Menu"
            MenuItem { action: objectives }
            MenuItem { action: statistics }
            MenuItem { action: unitlist }
            MenuItem { action: statustable }
            MenuSeparator {}
            MenuItem { action: save }
            MenuItem { action: savereplay }
            MenuItem { action: savemap }
            MenuItem { action: load }
            MenuItem { action: menu_autosaves }
            MenuSeparator {}
            MenuItem { action: preferences }
            MenuItem { action: chatlog }
            MenuItem { action: help }
            MenuSeparator {}
            MenuItem { action: stopnetwork }
            MenuItem { action: startnetwork }
            MenuItem { action: quit }
        }
        Menu {
            id: actions
            title: "&Actions"
            MenuItem { action: undo }
            MenuItem { action: redo }
            MenuItem { action: wbexecuteallactions }
            MenuItem { action: wbexecuteaction }
            MenuItem { action: wbdeleteaction }
            MenuItem { action: cycle }
            MenuItem { action: speak }
            MenuItem { action: recruit }
            MenuItem { action: recall }
            MenuItem { action: showenemymoves }
            MenuItem { action: bestenemymoves }
            MenuItem { action: wbtoggle }
            MenuItem { action: delayshroud }
            MenuItem { action: updateshroud }
            MenuItem { action: endturn }
        }
    }

    Label {
        text: "Wesnoth is Great"
        font.pixelSize: 22
        font.italic: true
        color: "green"
        objectName: "wesnoth_great_label"
        id: label1
    }

}

