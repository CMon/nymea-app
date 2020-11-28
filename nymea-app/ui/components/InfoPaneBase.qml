import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.2

Item {
    id: root
    implicitHeight: d.shownHeight
    visible: d.shownHeight > 0

    property alias color: background.color
    property bool shown: false

    property alias contentItem: content.data

    function show() {
        shown = true;
    }
    function hide() {
        shown = false;
    }

    signal clicked();

    QtObject {
        id: d
        property int shownHeight: shown ? content.implicitHeight : 0
        Behavior on shownHeight { NumberAnimation { easing.type: Easing.InOutQuad; duration: 150 } }
    }

    Pane {
        anchors { left: parent.left; bottom: parent.bottom; right: parent.right }
        Material.elevation: 2
        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0
        topPadding: 0
        height: content.implicitHeight

        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked()
        }

        Rectangle {
            id: background
            color: app.accentColor
            anchors.fill: parent
        }

        Item {
            id: content
            anchors { left: parent.left; top: parent.top; right: parent.right; leftMargin: app.margins; rightMargin: app.margins; topMargin: app.margins / 2 }
            implicitHeight: childrenRect.height + app.margins
        }
    }
}


