
import QtQuick 2.4
import QtLocation 5.6

// Marker.qml
MapQuickItem {
    id: marker
    property string callsign;
    property string imageName;

    anchorPoint.x: image.width/2
    anchorPoint.y: image.height // bottom of the image

    sourceItem: Grid {
        columns: 1
        Grid {
            horizontalItemAlignment: Grid.AlignHCenter
            Image {
                id: image
                rotation: 0
                source: imageName
            }
            Rectangle {
                id: bubble
                color: "lightblue"
                border.width: 1
                width: text.width * 1.3
                height: text.height * 1.3
                radius: 5
                opacity: 0.5
                Text {
                    id: text
                    anchors.centerIn: parent
                    text: callsign
                    opacity: 1.0
                }
            }
        }

        Rectangle {
            id: message
            color: "red"
            border.width: 1
            width: banner.width * 1.3
            height: banner.height * 1.3
            radius: 5
            opacity: 0
            Text {
                id: banner
                anchors.centerIn: parent
            }
            SequentialAnimation {
                id: playMessage
                running: false
                NumberAnimation { target: message;
                    property: "opacity";
                    to: 1.0;
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                PauseAnimation  { duration: 1000 }
                NumberAnimation { target: message;
                    property: "opacity";
                    to: 0.0;
                    duration: 200}
            }
        }
    }
    function showMessage(message) {
        banner.text = message
        playMessage.start()
    }
}
