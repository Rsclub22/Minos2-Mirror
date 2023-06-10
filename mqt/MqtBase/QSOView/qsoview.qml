import QtQuick 2.15
import QtQuick.Controls 2.15
import QtPositioning 5.15
import QtLocation 5.15

Frame {

    anchors.fill: parent

    visible: true
    signal qmlSignal(variant msg)

    property bool showGrid: false
    property bool showLines: false

    property string homeLat: "0.0"
    property string homeLon: "0.0"

    Map {
        id: mapOfEurope
        anchors.centerIn: parent;
        anchors.fill: parent
        plugin: Plugin {
            name: "osm"
        }

        ToolTip.text: calcToolTip()
        ToolTip.visible: hovered && mapMouse.containsMouse
        ToolTip.delay: 1000

        MouseArea {
            id: mapMouse
            anchors.fill: parent
            hoverEnabled: true

            onPressed: mouse => {
                let hcoord = QtPositioning.coordinate(homeLat, homeLon)
                let cc = mapOfEurope.toCoordinate(Qt.point(mouse.x,mouse.y));
                let b = hcoord.azimuthTo(cc)
                let gc = ["Pressed", cc.latitude, cc.longitude, b];
                qmlSignal(gc);
            }
            onPositionChanged: mapMouse => {
                // doesn't work!
                ToolTip.y = mapMouse.mouseY
                ToolTip.x = mapMouse.mouseX

            }
        }

        onCenterChanged: centre => {
            let gc = ["CentreChanged", center.latitude, center.longitude];
            qmlSignal(gc)
        }

        onZoomLevelChanged: zoomLevel => {
            let gc = ["ZoomChanged", zoomLevel]
            qmlSignal(gc)
        }

    } // end map

    function setColorAlpha(color, alpha) {
        return Qt.hsla(color.hslHue, color.hslSaturation, color.hslLightness, alpha)
    }
    function drawGrid()
    {
        if (showGrid)
        {
            // If we don't do it in chunks, the whole thing
            // can get cropped

            drawLatitude(-85, 85, -180, -90)
            drawLatitude(-85, 85, -90, /*-0.000001*/0)
            drawLatitude(-85, 85, /*0.000001*/0, 90)
            drawLatitude(-85, 85, 90, 180)

            drawLongitude(-85, 85, -180, -90)
            drawLongitude(-85, 85, -90, 0)
            drawLongitude(-85, 85, 0, 90)
            drawLongitude(-85, 85, 90, 180)
        }
    }

    function drawLatitude(minlat, maxlat, minlong,maxlong)
    {
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

    for (var latpos = minlat; latpos <= maxlat; latpos += 2)
    {
        cmlat.addCoordinate(QtPositioning.coordinate(latpos, minlong))
        cmlat.addCoordinate(QtPositioning.coordinate(latpos, maxlong))

        if (latpos !== maxlat)
        {
            cmlat.addCoordinate(QtPositioning.coordinate(latpos + 1, maxlong))
            cmlat.addCoordinate(QtPositioning.coordinate(latpos + 1, minlong))
        }
    }
    mapOfEurope.addMapItem(cmlat)
    }
    function drawLongitude(minlat, maxlat, minlong, maxlong)
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
        for (var longpos = minlong; longpos < maxlong ; longpos += 4)
        {
            cmlong.addCoordinate(QtPositioning.coordinate(maxlat, longpos))
            //cmlong.addCoordinate(QtPositioning.coordinate(0, longpos))
            cmlong.addCoordinate(QtPositioning.coordinate(minlat, longpos))

            cmlong.addCoordinate(QtPositioning.coordinate(minlat, longpos + 2 ))
            //cmlong.addCoordinate(QtPositioning.coordinate(0, longpos + 2))
            cmlong.addCoordinate(QtPositioning.coordinate(maxlat, longpos + 2))
        }
        mapOfEurope.addMapItem(cmlong)

    }
    function calcToolTip()
    {
        let hcoord = QtPositioning.coordinate(homeLat, homeLon)
        let cc = mapOfEurope.toCoordinate(Qt.point(mapMouse.mouseX, mapMouse.mouseY));
        if (cc.isValid)
        {

            let b = hcoord.azimuthTo(cc)
            let d = hcoord.distanceTo(cc)/1000

            let r = d.toFixed(1) + " Km " + b.toFixed() + " deg "
                + QmlCppLink.locator(cc.latitude, cc.longitude)

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


        //Can we determine if this will overlap anything, and then offset it?

        mapOfEurope.addMapItem(cm)

    }
    function addSpot(coord, callsign, loc)
    // Add the new callsign at coord to the map
    {
        var cm = Qt.createQmlObject('SpotMarker {}', mapOfEurope)
        cm.callsign = callsign
        cm.coordinate = coord
        cm.locator = loc;


        //Can we determine if this will overlap anything, and then offset it?

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
    // callInfo is a QStringList ["callsign", "latitude", "longitude", "loc"]
    {
        mapOfEurope.clearMapItems();

        var monitor = callInfo[0]

        var zoom = callInfo[1]
        var clat = callInfo[2]
        var clon = callInfo[3]

        mapOfEurope.center = QtPositioning.coordinate(clat, clon)
        mapOfEurope.zoomLevel = zoom

        var call = callInfo[4]

        homeLat = callInfo[5];
        homeLon = callInfo[6];
        var hcoord = QtPositioning.coordinate(homeLat, homeLon)
        var loc = callInfo[7]
        addHome(hcoord, call, loc)
    }
    function newCall(callInfo)
    // slot to receive signal callSig(QVariant)
    // callInfo is a QStringList ["callsign", "latitude", "longitude", "loc"]
    {
        //console.log(callInfo)
        var call = callInfo[0]
        var coord = QtPositioning.coordinate(callInfo[1], callInfo[2])
        var loc = callInfo[3]
        var doDrawLine = callInfo[4] === String("true")
        addCall(coord, call, loc)
        if (showLines && doDrawLine)
        {
            drawLine(QtPositioning.coordinate(homeLat, homeLon), coord)
        }
    }

    function newSpot(callInfo)
    // slot to receive signal spotSig(QVariant)
    // callInfo is a QStringList ["callsign", "latitude", "longitude", "loc"]
    {
        //console.log(callInfo)
        var call = callInfo[0]
        var coord = QtPositioning.coordinate(callInfo[1], callInfo[2])
        var loc = callInfo[3]
        var doDrawLine = callInfo[4] === String("true")
        addSpot(coord, call, loc)
        if (showLines && doDrawLine)
        {
            drawSpotLine(QtPositioning.coordinate(homeLat, homeLon), coord)
        }
    }

    function drawLine(hcoord, ccoord)
    {
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
    function drawSpotLine(hcoord, ccoord)
    {
        var line = Qt.createQmlObject(
'import QtQuick 2.15
import QtLocation 5.15

MapPolyline
{
    id: mline
    line.color: "blue"
    line.width: 1
}', mapOfEurope)
        line.addCoordinate(hcoord);
        line.addCoordinate(ccoord);

        mapOfEurope.addMapItem(line)
    }
    function setDrawLines(dl)
    {
        showLines = dl
    }
    function setDrawGrid(dg)
    {
        showGrid = dg
    }
    function clearAll()
    {
        mapOfEurope.clearMapItems();
    }
}

