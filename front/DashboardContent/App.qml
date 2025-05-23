import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtCharts

Window {
    id: mainWindow
    width: 1280
    height: 720
    visible: true
    title: "Dashboard"

    // Componente da janela de detalhe do processo
    Component {
        id: processDetailWindowComponent
        Window {
            width: 300
            height: 200
            visible: true
            title: "Process Detail - " + processName

            property string processId: ""
            property string processName: ""

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Text { text: "Process ID: " + processId; font.pointSize: 14 }
                Text { text: "Process Name: " + processName; font.pointSize: 14 }
                // Adicione mais informações do processo aqui se quiser
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // Linha no topo com info de processos e threads
        RowLayout {
            Layout.fillWidth: true
            spacing: 20
            Rectangle {
                color: "#e0e0e0"
                radius: 6
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                Text {
                    anchors.centerIn: parent
                    text: "Number of Processes: 150"
                    font.pointSize: 16
                    font.bold: true
                }
            }
            Rectangle {
                color: "#e0e0e0"
                radius: 6
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                Text {
                    anchors.centerIn: parent
                    text: "Number of Threads: 1240"
                    font.pointSize: 16
                    font.bold: true
                }
            }
        }

        // Conteúdo principal (tabela e gráficos)
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            // Lado esquerdo: tabela com ScrollView
            ScrollView {
                Layout.preferredWidth: 400
                Layout.fillHeight: true

                TableView {
                    id: tableView
                    anchors.fill: parent
                    clip: true
                    rowHeightProvider: function(row) { return 40 }

                    model: ListModel {
                        ListElement { idText: "001"; valueText: "Alpha" }
                        ListElement { idText: "002"; valueText: "Beta" }
                        ListElement { idText: "003"; valueText: "Gamma" }
                        ListElement { idText: "004"; valueText: "Delta" }
                        ListElement { idText: "005"; valueText: "Epsilon" }
                        ListElement { idText: "006"; valueText: "Zeta" }
                        ListElement { idText: "007"; valueText: "Eta" }
                        ListElement { idText: "008"; valueText: "Theta" }
                        ListElement { idText: "009"; valueText: "Iota" }
                        ListElement { idText: "010"; valueText: "Kappa" }
                        ListElement { idText: "011"; valueText: "Lambda" }
                        ListElement { idText: "012"; valueText: "Mu" }
                        ListElement { idText: "013"; valueText: "Nu" }
                        ListElement { idText: "014"; valueText: "Xi" }
                        ListElement { idText: "015"; valueText: "Omicron" }
                        ListElement { idText: "016"; valueText: "Pi" }
                        ListElement { idText: "017"; valueText: "Rho" }
                        ListElement { idText: "018"; valueText: "Sigma" }
                        ListElement { idText: "019"; valueText: "Tau" }
                        ListElement { idText: "020"; valueText: "Upsilon" }
                        ListElement { idText: "021"; valueText: "Phi" }
                        ListElement { idText: "022"; valueText: "Chi" }
                        ListElement { idText: "023"; valueText: "Psi" }
                        ListElement { idText: "024"; valueText: "Omega" }
                    }

                    delegate: RowLayout {
                        width: parent.width
                        height: 40

                        Rectangle {
                            width: 100; height: 40
                            color: "lightgray"
                            Text { anchors.centerIn: parent; text: model.idText }
                        }

                        Rectangle {
                            width: 280; height: 40
                            color: "white"
                            Text { anchors.centerIn: parent; text: model.valueText }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                let win = processDetailWindowComponent.createObject(mainWindow, {
                                    processId: model.idText,
                                    processName: model.valueText
                                });
                                win.show();
                            }
                        }
                    }
                }
            }

            // Lado direito: gráficos ocupando toda a altura
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 20

                ChartView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    title: "CPU Usage"
                    antialiasing: true

                    LineSeries {
                        name: "CPU"
                        XYPoint { x: 0; y: 1.2 }
                        XYPoint { x: 1; y: 2.5 }
                        XYPoint { x: 2; y: 3.1 }
                        XYPoint { x: 3; y: 2.0 }
                        XYPoint { x: 4; y: 4.3 }
                    }
                }

                ChartView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    title: "Memory Usage"
                    antialiasing: true

                    LineSeries {
                        name: "Memory"
                        XYPoint { x: 0; y: 0.9 }
                        XYPoint { x: 1; y: 1.7 }
                        XYPoint { x: 2; y: 2.5 }
                        XYPoint { x: 3; y: 2.9 }
                        XYPoint { x: 4; y: 3.4 }
                    }
                }
            }
        }
    }
}
