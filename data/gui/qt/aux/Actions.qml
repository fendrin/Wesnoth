/*
QtQuick Controls supports actions in Button, ToolButton, and MenuItem.
*/

import QtQuick 2.3;
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2;

QtObject {

    property Action cancel: Action {
        text: "&Cancel"
    }
    property Action selecthex: Action {
        text: "&Select Hex"
    }
    property Action deselecthex: Action {
        text: "&Deselect Hex"
    }
    property Action moveaction: Action {
        text: "&Move/Attack"
    }
    property Action cycleback: Action {
        text: "&Previous Unit"
    }
    property Action holdposition: Action {
        text: "&Hold Position"
    }
    property Action endunitturn: Action {
        text: "&End Unit Turn"
    }
    property Action leader: Action {
        text: "&Leader"
    }
    property Action animatemap: Action {
        text: "&Animate Map"
    }

    property Action zoomin: Action {
        text: "&Zoom In"
    }
    property Action zoomout: Action {
        text: "&Zoom Out"
    }
    property Action zoomdefault: Action {
        text: "&Default Zoom"
    }
    property Action fullscreen: Action {
        text: "&Toggle Full Screen"
    }
    property Action accelerated: Action {
        text: "&Accelerated"
    }

    property Action objectives: Action {
        text: "&Show Objectives"
        shortcut: "Ctrl+j"
        // iconSource: "images/document-open.png"
        // onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action statistics: Action {
        text: "&Statistics"
        shortcut: "s"
        // iconSource: "images/document-open.png"
        // onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action unitlist: Action {
        text: "&Unit List"
        shortcut: "Alt+u"
        // iconSource: "images/document-open.png"
        // onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action statustable: Action {
        text: "&Status Table"
        shortcut: "Alt+s"
        // iconSource: "images/document-open.png"
        // onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action save: Action {
        text: "&Save Game"
        shortcut: "Ctrl+s"
        // iconSource: "images/document-open.png"
        // onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action savereplay: Action {
        text: "&Save Replay"
        // shortcut: None
        // iconSource: "images/document-open.png"
        // onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action savemap: Action {
        text: "&Save Map"
        //shortcut: None
        // iconSource: "images/document-open.png"
        // onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action load: Action {
        text: "&Load Game"
        shortcut: "Ctrl+o"
        // iconSource: "images/document-open.png"
        // onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action menu_autosaves: Action {
        text: "&Submenu Autosaves"
        //         shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action preferences: Action {
        text: "&Preferences"
        shortcut: "Ctrl+p"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action chatlog: Action {
        text: "&View Chat Log"
        shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action help: Action {
        text: "&Help"
        shortcut: "F1"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action stopnetwork: Action {
        text: "&Stop Network"
        //         shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action startnetwork: Action {
        text: "&Start Network"
        //         shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action quit: Action {
        text: "&Quit Game"
        shortcut: "Ctrl+q"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action undo: Action {
        text: "&Undo"
        shortcut: "u"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action redo: Action {
        text: "&Redo"
        shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action wbexecuteallactions: Action {
        text: "&Status Table"
        shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action wbexecuteaction: Action {
        text: "&Status Table"
        shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action wbdeleteaction: Action {
        text: "&Status Table"
        shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action cycle: Action {
        text: "&Status Table"
        shortcut: "n"
        iconSource: "images/document-open.png"
        onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action speak: Action {
        text: "&Status Table"
        shortcut: "m"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        //        tooltip: "Open an image"
    }

    property Action recruit: Action {
        text: "&Recruit"
        shortcut: "Ctrl+r"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action recall: Action {
        text: "&Recall"
        shortcut: "Alt+r"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action showenemymoves: Action {
        text: "&Show Enemy Moves"
        shortcut: "Ctrl+v"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action bestenemymoves: Action {
        text: "&Best Possible Enemy Moves"
        shortcut: "Ctrl+b"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action wbtoggle: Action {
        text: "&Planning Mode"
        shortcut: "p"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action delayshroud: Action {
        text: "&Delay Shroud Updates"
        shortcut: "Alt+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action updateshroud: Action {
        text: "&Update Shroud Now"
        shortcut: "Shift+s"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    property Action endturn: Action {
        text: "&End Turn"
        shortcut: "Ctrl+Space"
        //         iconSource: "images/document-open.png"
        //         onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

}
