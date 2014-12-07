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

    Action {
         id: objectives
         text: "&Show Objectives"
         shortcut: "Ctrl+j"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: statistics
         text: "&Statistics"
         shortcut: "s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: unitlist
         text: "&Unit List"
         shortcut: "Alt+u"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: statustable
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: save
         text: "&Save Game"
         shortcut: "Ctrl+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: savereplay
         text: "&Save Replay"
//         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: savemap
         text: "&Save Map"
//         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: load
         text: "&Load Game"
         shortcut: "Ctrl+o"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: menu_autosaves
         text: "&Submenu Autosaves"
//         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: preferences
         text: "&Preferences"
         shortcut: "Ctrl+p"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: chatlog
         text: "&View Chat Log"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: help
         text: "&Help"
         shortcut: "F1"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: stopnetwork
         text: "&Stop Network"
//         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: startnetwork
         text: "&Start Network"
//         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: quit
         text: "&Quit Game"
         shortcut: "Ctrl+q"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: undo
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: redo
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: wbexecuteallactions
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: wbexecuteaction
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: wbdeleteaction
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: cycle
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: speak
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: recruit
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: recall
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: showenemymoves
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: bestenemymoves
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: wbtoggle
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: delayshroud
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: updateshroud
         text: "&Status Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }

    Action {
         id: endturn
         text: "&Stawtus Table"
         shortcut: "Alt+s"
//         iconSource: "images/document-open.png"
//         onTriggered: fileDialog.open()
         tooltip: "Open an image"
     }


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

