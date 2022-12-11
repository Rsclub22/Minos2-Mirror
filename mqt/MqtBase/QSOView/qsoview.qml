import QtQuick 2.15
import QtQuick.Controls 2.15
import QtPositioning 5.15
import QtLocation 5.15

Frame {

    anchors.fill: parent

    visible: true
    signal qmlSignal(variant msg)

    property string homeLat: "0.0"
    property string homeLon: "0.0"

    // Define Visible Region of map
    property variant topLeftEurope: QtPositioning.coordinate(60.5, -11.0)
    property variant bottomRightEurope: QtPositioning.coordinate(45.0, 12.0)
    property variant viewOfEurope:
        QtPositioning.rectangle(topLeftEurope, bottomRightEurope)

    Map {
        id: mapOfEurope
        anchors.centerIn: parent;
        anchors.fill: parent
        plugin: Plugin {
            name: "osm"
        }

        // Set (Initial) Visible Region of map
        visibleRegion: viewOfEurope

        ToolTip.text: calcToolTip()
        ToolTip.visible: hovered && mapMouse.containsMouse
        ToolTip.delay: 1000

        MouseArea {
            id: mapMouse
            anchors.fill: parent
            hoverEnabled: true
            //onEntered: showToolTip = true
            //onExited: showToolTip = false
            onPressed: {
                let hcoord = QtPositioning.coordinate(homeLat, homeLon)
                let cc = mapOfEurope.toCoordinate(Qt.point(mouse.x,mouse.y));
                let b = hcoord.azimuthTo(cc)
                let gc = [cc.latitude, cc.longitude, b];
                qmlSignal(gc);
            }
            onPositionChanged: {
                // doesn't work!
                ToolTip.y = mapMouse.mouseY
                ToolTip.x = mapMouse.mouseX

            }
            }
    } // end map

    Button {
        text: "Clear"
        hoverEnabled: true
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: "Clear all the calls from the map."

        anchors.top: parent.top
        anchors.right: parent.right
        enabled: true
        onClicked: mapOfEurope.clearMapItems()
    }

    function calcToolTip()
    {
        let hcoord = QtPositioning.coordinate(homeLat, homeLon)
        let cc = mapOfEurope.toCoordinate(Qt.point(mapMouse.mouseX, mapMouse.mouseY));
        let b = hcoord.azimuthTo(cc)
        let d = hcoord.distanceTo(cc)/1000

        let r = d.toFixed(1) + " Km " + b.toFixed() + " deg"

        return r
    }

    function addCircle(coord)
    // Add a circle, such as to mark the home QTH
    {
        var circle = Qt.createQmlObject('import QtLocation 5.15; MapCircle {}', mapOfEurope)
        circle.center = coord
        circle.radius = 1000.0 // km
        circle.color = 'blue'
        circle.opacity = 0.5
        circle.border.width = 3
        mapOfEurope.addMapItem(circle)
    }

    function addCall(coord, callsign, loc)
    // Add the new callsign at coord to the map
    {
        var cm = Qt.createQmlObject('CallMarker {}', mapOfEurope)
        cm.callsign = callsign
        cm.coordinate = coord
        cm.locator = loc;

        mapOfEurope.addMapItem(cm)
    }

    function addHome(coord, callsign, loc)
    // Add the new callsign at coord to the map
    {
        var cm = Qt.createQmlObject('HomeMarker {}', mapOfEurope)
        cm.callsign = callsign
        cm.coordinate = coord
        cm.locator = loc;

        mapOfEurope.addMapItem(cm)
    }

    function newHome(callInfo)
    // slot to receive signal callSig(QVariant)
    // callInfo is a QStringList ["callsign", "latitude", "longitude"]
    {
        mapOfEurope.clearMapItems();
        var call = callInfo[0]

        homeLat = callInfo[1];
        homeLon = callInfo[2];
        var hcoord = QtPositioning.coordinate(homeLat, homeLon)
        var loc = callInfo[3]
        addHome(hcoord, call, loc)
    }
    function newCall(callInfo)
    // slot to receive signal callSig(QVariant)
    // callInfo is a QStringList ["callsign", "latitude", "longitude"]
    {
        //console.log(callInfo)
        var call = callInfo[0]
        var coord = QtPositioning.coordinate(callInfo[1], callInfo[2])
        var loc = callInfo[3]
        addCall(coord, call, loc)
        drawLine(QtPositioning.coordinate(homeLat, homeLon), coord)
    }


    function drawLine(hcoord, ccoord)
    {
        var uline = Qt.createComponent("qrc:/mapline.qml", Component.PreferSynchronous, mapOfEurope);
        var line = uline.createObject(mapOfEurope);
        line.addCoordinate(hcoord);
        line.addCoordinate(ccoord);

        mapOfEurope.addMapItem(line)
    }

}
