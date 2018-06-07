import QtQuick 2.0
import QtQuick.Templates 2.2
import QtQuick.Controls.Material 2.2

ApplicationWindow {
    property color guhAccent: "#ffcc00"
    property string systemName: "maveo"
    property string appName: qsTr("Maveo Pro Box Dashboard")

    Material.theme: Material.Light
    Material.accent: guhAccent
    Material.primary: Material.Grey
}
