import QtQuick 2.0
import QtQuick.Controls 2.0

Image {
    id: img
    property string unselected_source
    property string selected_source
    property bool selectable: false
    property bool selected: false
    signal clicked()
    source: selected ? selected_source : unselected_source
    cache: false
    Behavior on scale {
        NumberAnimation { duration: 200 }
    }
    MouseArea {
        anchors.fill: img
        hoverEnabled: true
        onEntered: img.scale = 0.92
        onExited: img.scale = 1.0
        onCanceled: img.scale = 1.0
        onPressed: img.scale = 0.8
        onClicked: img.clicked()
        onReleased: {
            img.scale = 0.92
            if(img.selectable) {
                img.selected ^= 1
                img.selectedChanged()
            }
        }
    }
}

