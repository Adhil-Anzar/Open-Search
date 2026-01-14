import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts 1.15
import QtQuick.Effects
import Results 1.0

Window {
    id: root
    width: 500
    height: 67
    visible: true
    flags: Qt.Window | Qt.FramelessWindowHint

    // Locking it in place ( Disabling user based actions )
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width
    color: "#11000000"
    onActiveChanged: {
        if (!active) {
            //root.visibility = false;
            //root.close();
        }
    }
    Behavior on height{
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    FocusScope {
        anchors.fill: parent
        focus: true

        Component.onCompleted: searchBox.forceActiveFocus()
        Keys.onPressed:
        event => {
            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter){
                if (results.currentIndex === -1)
                    executeItem(0)
                else
                    executeItem(results.currentIndex);
            }
            else if (event.key === Qt.Key_Escape) {
                root.close();
                event.accepted = true;
            }
            else if (event.key === Qt.Key_Up) {
                if (results.currentIndex > 0) {
                    results.currentIndex -= 1;
                } else {
                    results.currentIndex = 0;
                }
            }
            else if (event.key === Qt.Key_Down) {
                if (results.currentIndex < results.count - 1) {
                    results.currentIndex += 1;
                } else {
                    results.currentIndex = results.count - 1;
                }
            }
        }
        function executeItem(index){
            var origin = resultsModel.getOrigin(index)
            if (origin === 2){
                searchBox.text = resultsModel.getName(index);
            }
            else if (origin === -1) return;
            resultsModel.executeItem(index);
        }

        Column{
            anchors.fill: parent

            Item{
                id: searchBoxContainer
                width: parent.width
                height: 50

                Rectangle {
                    anchors.fill: parent
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.margins: 4
                    radius: 10

                    color: "transparent"
                    border.color: "#55FFFFFF"
                    border.width: 1
                }
                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 4
                    height: parent.height - 4
                    Item { width: 10; height: 10; }
                    Image {
                        id: searchBoxImage
                        fillMode: Image.PreserveAspectFit
                        width: parent.height - 20
                        height: width
                        anchors.verticalCenter: parent.verticalCenter

                        source: "/assets/open-search.svg"
                    }
                    Item { width: 10; height: 10; }
                    Item{
                        width: parent.width - searchBoxImage.width - 30
                        height: parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        TextField {
                            id: searchBox
                            anchors.fill: parent
                            font.pixelSize: 24
                            placeholderText: qsTr("Type to Explore!")
                            // REMOVE ALL STYLING
                            background: null
                            padding: 0
                            leftPadding: 0
                            rightPadding: 0
                            topPadding: 0
                            bottomPadding: 0

                            onDisplayTextChanged: {
                                results.currentIndex = -1;
                                if (!resultsModel.searchResults(searchBox.text)){
                                    resultsModel.clearItems();
                                }
                            }
                        }
                    }

                }
            }
            Item{
                id: resultsListContainer
                width: parent.width
                height: 0

                ListView {
                    id: results
                    width: parent.width
                    height: parent.height
                    clip: true
                    spacing: 4
                    focus: true
                    currentIndex: -1

                    model: resultsModel

                    //Component.onCompleted: updateHeight()
                    onCountChanged: updateHeight()
                    function updateHeight() {
                        if (resultsModel === null) return;
                        var h = resultsModel.rowCount() > 5 ? 5 * 60 : resultsModel.rowCount() * 60;
                        root.height = h + searchBoxContainer.height;
                        resultsListContainer.height = h;
                    }

                    delegate: Item {
                        width: results.width
                        height: 56
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                results.currentIndex = results.currentIndex === index ? -1 : index;
                            }
                        }
                        Rectangle {
                            anchors.fill: parent
                            radius: 12
                            color: index === results.currentIndex ? "#22FFFFFF" : "transparent"
                        }
                        Row {
                            anchors.fill: parent
                            Item { width: 12; height: 12}
                            Image {
                                width: 30
                                height: 30
                                anchors.verticalCenter: parent.verticalCenter
                                source: "file://" + ItemUtils.resolveIconPath(icon)
                                fillMode: Image.PreserveAspectFit
                            }
                            Item { width: 12; height: 12}
                            Column {
                                anchors.margins: 12
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 2
                                Text {
                                    text: `<b>${name}</b>`
                                    color: "#FFFFFF"
                                    font.pixelSize: 16
                                }
                                Text {
                                    text: {
                                        if (origin === 0){
                                            if (genericName === "" || genericName === null)
                                                return `<b>${type}</b>`;
                                            else
                                                return `<b>${type}</b> - ${genericName}`;
                                        }
                                        else if (origin === 1 || origin === 2 || origin === 3){
                                            return comment;
                                        }
                                        else{
                                            return "Not Implemented";
                                        }
                                    }
                                    color: "#AAAAAA"
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.Auto
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                    }
                }
            }
        }

    }
}
