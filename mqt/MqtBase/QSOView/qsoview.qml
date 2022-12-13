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

    // Define Visible Region of map to include Ireland
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

    function setColorAlpha(color, alpha) {
        return Qt.hsla(color.hslHue, color.hslSaturation, color.hslLightness, alpha)
    }
    function drawGrid()
    {

        var cmlong = Qt.createQmlObject(
    'import QtQuick 2.15
    import QtLocation 5.15

    MapPolyline
    {
        id: mline
        line.color: "grey"
        line.width: 1
    }', mapOfEurope)
            cmlong.line.color = setColorAlpha(cmlong.line.color, 0.5)

        // QtPositioning.coordinate(lat, long)
        for (var longpos = -180; longpos < 0 ; longpos += 4)
        {
            cmlong.addCoordinate(QtPositioning.coordinate(80, longpos))
            //cmlong.addCoordinate(QtPositioning.coordinate(80, longpos))
            cmlong.addCoordinate(QtPositioning.coordinate(-80, longpos))

            cmlong.addCoordinate(QtPositioning.coordinate(-80, longpos + 2 ))
            //cmlong.addCoordinate(QtPositioning.coordinate(80, longpos + 2))
            cmlong.addCoordinate(QtPositioning.coordinate(80, longpos + 2))
        }
        mapOfEurope.addMapItem(cmlong)

        var cmlong1 = Qt.createQmlObject(
    'import QtQuick 2.15
    import QtLocation 5.15

    MapPolyline
    {
        id: mline
        line.color: "grey"
        line.width: 1
    }', mapOfEurope)
            cmlong1.line.color = setColorAlpha(cmlong1.line.color, 0.5)

        // QtPositioning.coordinate(lat, long)
        for (var longpos1 = 0; longpos1 < 180; longpos1 += 4)
        {
            cmlong1.addCoordinate(QtPositioning.coordinate(80, longpos1))
            //cmlong1.addCoordinate(QtPositioning.coordinate(80, longpos1))
            cmlong1.addCoordinate(QtPositioning.coordinate(-80, longpos1))

            cmlong1.addCoordinate(QtPositioning.coordinate(-80, longpos1 + 2 ))
            //cmlong1.addCoordinate(QtPositioning.coordinate(80, longpos1 + 2))
            cmlong1.addCoordinate(QtPositioning.coordinate(80, longpos1 + 2))
        }
        mapOfEurope.addMapItem(cmlong1)


            var cmlat = Qt.createQmlObject(
    'import QtQuick 2.15
    import QtLocation 5.15

    MapPolyline
    {
        id: mline
        line.color: "grey"
        line.width: 1
    }', mapOfEurope)
        cmlat.line.color = setColorAlpha(cmlat.line.color, 0.5)

        for (var latpos = -80; latpos <= 80; latpos += 2)
        {
            cmlat.addCoordinate(QtPositioning.coordinate(latpos, -180))
            cmlat.addCoordinate(QtPositioning.coordinate(latpos, -0.00001))

            if (latpos != 80)
            {
                cmlat.addCoordinate(QtPositioning.coordinate(latpos + 1, -0.00001))
                cmlat.addCoordinate(QtPositioning.coordinate(latpos + 1, -180))
            }
        }
        mapOfEurope.addMapItem(cmlat)

        var cmlat1 = Qt.createQmlObject(
'import QtQuick 2.15
import QtLocation 5.15

MapPolyline
{
    id: mline
    line.color: "grey"
    line.width: 1
}', mapOfEurope)
    cmlat1.line.color = setColorAlpha(cmlat1.line.color, 0.5)

    for (var latpos1 = -80; latpos1 <= 80; latpos1 += 2)
    {
        cmlat1.addCoordinate(QtPositioning.coordinate(latpos1, 0.00001))
        cmlat1.addCoordinate(QtPositioning.coordinate(latpos1, 180))

        if (latpos1 != 80)
        {
            cmlat1.addCoordinate(QtPositioning.coordinate(latpos1 + 1, 180))
            cmlat1.addCoordinate(QtPositioning.coordinate(latpos1 + 1, 0.00001))
        }
    }
    mapOfEurope.addMapItem(cmlat1)

        return 'OK'
    }

    function calcToolTip()
    {
        let hcoord = QtPositioning.coordinate(homeLat, homeLon)
        let cc = mapOfEurope.toCoordinate(Qt.point(mapMouse.mouseX, mapMouse.mouseY));
        if (cc.isValid)
        {
            let b = hcoord.azimuthTo(cc)
            let d = hcoord.distanceTo(cc)/1000

            let r = d.toFixed(1) + " Km " + b.toFixed() + " deg"

            return r
        }
        return ""
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
        drawGrid()
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
//        var cm = Qt.createQmlObject('HomeMarker {}', mapOfEurope)
//        var uline = Qt.createComponent("qrc:/mapline.qml", Component.PreferSynchronous, mapOfEurope);
//        var line = uline.createObject(mapOfEurope);

        var line = Qt.createQmlObject(
'import QtQuick 2.15
import QtLocation 5.15

MapPolyline
{
    id: mline
    line.color: "green"
    line.width: 1
}', mapOfEurope)
        line.addCoordinate(hcoord);
        line.addCoordinate(ccoord);

        mapOfEurope.addMapItem(line)
    }

}
