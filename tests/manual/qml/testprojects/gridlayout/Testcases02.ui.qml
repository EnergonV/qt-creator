// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

Rectangle {
    width: 768
    height: 640

    RadioButton {
        id: radioButton1
        x: 56
        y: 23
        text: qsTr("Radio Button")
    }

    Slider {
        id: sliderVertical1
        x: 19
        y: 23
        orientation: Qt.Vertical
    }

    ComboBox {
        id: comboBox1
        x: 56
        y: 51
    }

    TextArea {
        id: textArea1
        x: 56
        y: 77
    }

    Button {
        id: button1
        x: 221
        y: 50
        text: qsTr("Button")
    }

    Slider {
        id: sliderHorizontal1
        x: 353
        y: 122
        width: 230
        height: 22
    }

    CheckBox {
        id: checkBox1
        x: 353
        y: 90
        text: qsTr("Check Box")
    }

    CheckBox {
        id: checkBox2
        x: 435
        y: 90
        text: qsTr("Check Box")
    }

    CheckBox {
        id: checkBox3
        x: 516
        y: 90
        text: qsTr("Check Box")
    }

    ComboBox {
        id: comboBox2
        x: 435
        y: 56
        width: 148
        height: 20
    }

    Label {
        id: label1
        x: 358
        y: 60
        text: qsTr("Label")
    }

    TextField {
        id: textField1
        x: 435
        y: 24
        width: 148
        height: 20
        placeholderText: qsTr("Text Field")
    }

    Label {
        id: label2
        x: 358
        y: 28
        text: qsTr("Label")
    }

    Rectangle {
        id: rectangle1
        x: 19
        y: 313
        width: 200
        height: 87
        color: "#f05454"
    }

    Rectangle {
        id: rectangle2
        x: 225
        y: 313
        width: 290
        height: 87
        color: "#dbdbdb"
    }

    Rectangle {
        id: rectangle3
        x: 19
        y: 412
        width: 94
        height: 87
        color: "#afc8e7"
    }

    Rectangle {
        id: rectangle4
        x: 125
        y: 406
        width: 94
        height: 87
        color: "#e5bc2c"
    }

    Rectangle {
        id: rectangle5
        x: 225
        y: 412
        width: 94
        height: 87
        color: "#000000"
    }

    Rectangle {
        id: rectangle6
        x: 330
        y: 421
        width: 94
        height: 87
        color: "#b31198"
    }

    Rectangle {
        id: rectangle7
        x: 430
        y: 412
        width: 94
        height: 181
        color: "#344ed5"
    }

    Rectangle {
        id: rectangle8
        x: 19
        y: 506
        width: 290
        height: 87
        color: "#bedd55"
    }

    Rectangle {
        id: rectangle9
        x: 323
        y: 514
        width: 94
        height: 87
        color: "#8d8d8d"
    }
}
