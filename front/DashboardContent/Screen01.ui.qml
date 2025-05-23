

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import Dashboard

Rectangle {
    id: rectangle
    width: Constants.width
    height: Constants.height

    color: Constants.backgroundColor

    Row {
        id: row
        x: -221
        y: -282
        width: 153
        height: 626
    }

    Chart {
        id: rectangle1
        x: 0
        y: 0
    }
    states: [
        State {
            name: "clicked"
        }
    ]
}
