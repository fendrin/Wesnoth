import QtQuick 2.3;
import QtQuick.Dialogs 1.2;
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

// import "../widgets"
import "../aux"

ApplicationWindow {
    visible: true
    title: "The Battle for Wesnoth"

    width: 1024
    height: 768

    minimumWidth: 800
    minimumHeight: 480

    property Actions actions: Actions {}

    menuBar: MenuBar {
        Menu {
            id: main_menu
            title: "&Menu"

            MenuItem { action: actions.objectives }
            MenuItem { action: actions.statistics }
            MenuItem { action: actions.unitlist }
            MenuItem { action: actions.statustable }
            MenuSeparator {}
            MenuItem { action: actions.save }
            MenuItem { action: actions.savereplay }
            MenuItem { action: actions.savemap }
            MenuItem { action: actions.load }
            MenuItem { action: actions.menu_autosaves }
            MenuSeparator {}
            MenuItem { action: actions.preferences }
            MenuItem { action: actions.chatlog }
            MenuItem { action: actions.help }
            MenuSeparator {}
            MenuItem { action: actions.stopnetwork }
            MenuItem { action: actions.startnetwork }
            MenuItem { action: actions.quit }
        }

        Menu {
            id: action_menu
            title: "&Actions"
            MenuItem { action: actions.undo }
            MenuItem { action: actions.redo }
            MenuItem { action: actions.wbexecuteallactions }
            MenuItem { action: actions.wbexecuteaction }
            MenuItem { action: actions.wbdeleteaction }
            MenuItem { action: actions.cycle }
            MenuItem { action: actions.speak }
            MenuItem { action: actions.recruit }
            MenuItem { action: actions.recall }
            MenuItem { action: actions.showenemymoves }
            MenuItem { action: actions.bestenemymoves }
            MenuItem { action: actions.wbtoggle }
            MenuItem { action: actions.delayshroud }
            MenuItem { action: actions.updateshroud }
            MenuSeparator {}
            MenuItem { action: actions.endturn }
        }
    }

    toolBar:ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                action: actions.load
            }
            ToolButton {
                action: actions.quit
            }
            ToolButton {
                action: actions.endturn
            }
            Item { Layout.fillWidth: true }
            CheckBox {
                text: "Enabled"
                checked: true
                Layout.alignment: Qt.AlignRight
            }
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

