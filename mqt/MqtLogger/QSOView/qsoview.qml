/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtPositioning 5.15
import QtLocation 5.15

Frame {

    // Initial App Size
//    width: parent.width
//    height: parent.height

    anchors.fill: parent

    visible: true

    property string homeLat: "0.0"
    property string homeLon: "0.0"

    // Define Visible Region of map
    property variant topLeftEurope: QtPositioning.coordinate(60.5, 0.0)
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

        // Set Visible Region of map
        visibleRegion: viewOfEurope
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
        cm.showMessage('New')
    }

    function addHome(coord, callsign, loc)
    // Add the new callsign at coord to the map
    {
        var cm = Qt.createQmlObject('HomeMarker {}', mapOfEurope)
        cm.callsign = callsign
        cm.coordinate = coord
        cm.locator = loc;

        mapOfEurope.addMapItem(cm)
        cm.showMessage('Home')
    }

    function newHome(callInfo)
    // slot to receive signal callSig(QVariant)
    // callInfo is a QStringList ["callsign", "latitude", "longitude"]
    {
        mapOfEurope.clearMapItems();
        //console.log(callInfo)
        var call = callInfo[0]
        var coord = QtPositioning.coordinate(callInfo[1], callInfo[2])

        homeLat = callInfo[1];
        homeLon = callInfo[2];
        var loc = callInfo[3]
        addHome(coord, call, loc)
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
