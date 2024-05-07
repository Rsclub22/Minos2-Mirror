import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtPositioning 5.15
import QtLocation 5.15

Frame {

    anchors.fill: parent

    visible: true
    signal qmlSignal(variant msg)

    property bool showGrid: false
    property bool showLines: false

    property bool showLocs: true
    property int locTLLat: 0
    property int locTLLon: 0
    property int locBRLat: 0
    property int locBRLon: 0
    property bool showNav: true

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

        WheelHandler {
             id: wheel
             // workaround for QTBUG-87646 / QTBUG-112394 / QTBUG-112432:
             // Magic Mouse pretends to be a trackpad but doesn't work with PinchHandler
             // and we don't yet distinguish mice and trackpads on Wayland either
             acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                              ? PointerDevice.Mouse | PointerDevice.TouchPad
                              : PointerDevice.Mouse
             rotationScale: 1/120
             property: "zoomLevel"
         }
         DragHandler {
             id: drag
             target: null
             onTranslationChanged: (delta) => mapOfEurope.pan(-delta.x, -delta.y)
         }
        MouseArea {
            id: mapMouse
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton/* | Qt.RightButton*/

            onPressed: mouse => {
                {
                    if (pressedButtons && Qt.LeftButton)
                    {
                        let hcoord = QtPositioning.coordinate(homeLat, homeLon)
                        let cc = mapOfEurope.toCoordinate(Qt.point(mouse.x,mouse.y));
                        let b = hcoord.azimuthTo(cc)
                        let gc = ["LeftPressed", cc.latitude, cc.longitude, b];
                        qmlSignal(gc);
                    }
                }
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

    Frame {
        id: navPanel
        anchors{
            top:parent.top
            right:parent.right
        }
        GridLayout {
            id: gridLayout
            columns:2
            anchors.fill: parent

            ToolButton {
                id: upButton
                Layout.row:0
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter
                text:"^"
                enabled:true
                onClicked: {
                    mapOfEurope.pan(0, -10)
                }
            }
            ToolButton {
                id: leftButton
                Layout.row:1
                Layout.column: 0
                text:"<"
                enabled:true
                onClicked: {
                    mapOfEurope.pan(-10, 0)
                }
            }
            ToolButton {
                id: rightButton
                Layout.row:1
                Layout.column: 1
                text:">"
                enabled:true
                onClicked: {
                    mapOfEurope.pan(10, 0)
                }
            }
            ToolButton {
                id: downButton
                Layout.row:2
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter
                text:"V"
                enabled:true
                onClicked: {
                    onClicked:mapOfEurope.pan(0, 10)
                }
            }
            ToolButton {
                id: plusButton
                Layout.row:3
                Layout.column: 0
                text:"+"
                enabled:true
                onClicked: {
                    mapOfEurope.zoomLevel=mapOfEurope.zoomLevel + 0.1
                }
            }
            ToolButton {
                id: minusButton
                Layout.row:3
                Layout.column: 1
                text:"-"
                enabled:true
                onClicked: {
                    mapOfEurope.zoomLevel=mapOfEurope.zoomLevel - 0.1
                }
            }
        }
    }

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

            // for (var latPos = -84; latPos < 85; latPos += 1 )
            // {
            //     for (var lonPos = -180; lonPos < 180; lonPos += 2)
            if (showLocs)
            {
                for (var latPos = locBRLat; latPos < locTLLat; latPos += 1 )
                {
                    for (var lonPos = locTLLon; lonPos < locBRLon; lonPos += 2)
                    {
                        drawQRA(latPos, lonPos);
                    }
                }
            }
//            drawQRA(52, 0);

        }
    }

    function drawQRA(lat, lon)
    {
        var cmqra = Qt.createQmlObject(
'import QtQuick 2.15
import QtLocation 5.15

MapQuickItem {
    id: qraItem
sourceItem: Text {
        id: qraText
        text: ""
        color: "gray"

        font.pointSize: mapOfEurope.zoomLevel * mapOfEurope.zoomLevel  * mapOfEurope.zoomLevel/ 16

}
}', mapOfEurope);
        cmqra.sourceItem.color = setColorAlpha(cmqra.sourceItem.color, 0.5)
        let r = QmlCppLink.locator(lat - 0.05, lon + 0.05);

        cmqra.sourceItem.text = r.slice(0, 4);
        let p = QtPositioning.coordinate(lat - 0.05, lon + 0.05)
        cmqra.coordinate = p;
        mapOfEurope.addMapItem(cmqra);
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

        navPanel.visible = showNav
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
    function setShowLocs(sl)
    {
        showLocs = sl;
    }
    function setShowNav(sn)
    {
        showNav = sn;
    }
    function setShowLocsTL(tl)
    {
        locTLLat = parseInt(tl[0], 10)
        locTLLon = parseInt(tl[1], 10)
    }
    function setShowLocsBR(br)
    {
        locBRLat = parseInt(br[0], 10)
        locBRLon = parseInt(br[1], 10)
    }

    function clearAll()
    {
        mapOfEurope.clearMapItems();
    }
}

