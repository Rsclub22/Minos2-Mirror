
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15

// Marker.qml
MapQuickItem {
    id: marker
    property string callsign;
    property string imageName;
    property  string locator;
    property bool exactCentre:false;

    anchorPoint.x: image.width/2
    anchorPoint.y: exactCentre?image.height/2:image.height // bottom of the image

    sourceItem: Image {
        id: image
        rotation: 0
        source: imageName
        fillMode: Image.PreserveAspectFit

        width: sourceSize.width / 2
        height: sourceSize.height / 2

        ToolTip.text: callsign + ' ' + locator
        ToolTip.visible: locator ? ma.containsMouse : false

        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true

            // this seems to be needed to pass the press on
            onPressed: mouse.accepted = false;
        }
    }
}
