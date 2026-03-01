import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {        // Use ApplicationWindow instead of Window for better controls
    visible: true
    width: 640
    height: 480
    title: qsTr("Personal Analyser For Hiring")

    StackView {
        id: stack
        anchors.fill: parent

        initialItem: mainPage     // start with your main page
    }

    Component {
        id: mainPage

        Page {
            // Title
            Text {
                id: companyName
                text: qsTr("Nepal Army")
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 50
            }

            // Admin Button
            Button {
                text: "Admin"
                width: 200
                height: 50
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 100

                background: Rectangle {
                    color: "#4B5320"
                    radius: 10
                }

                onClicked: {
                    stack.push(adminPage)   // push the Admin page
                }
            }

            // Candidate Button
            Button {
                text: "Candidate"
                width: 200
                height: 50
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 100

                background: Rectangle {
                    color: "#4B5320"
                    radius: 10
                }

                onClicked: {
                    stack.push(candidatePage)   // push the Candidate page
                }
            }
        }
    }

    // Admin Page
    Component {
        id: adminPage

        Page {


                        Column {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 10

                            // Move column slightly above center
                            anchors.verticalCenterOffset: -50

                            // Label
                            Text {
                                text: "Enter Password"
                                font.pixelSize: 24
                                horizontalAlignment: Text.AlignHCenter
                            }

                            // Input field
                            TextField {
                                placeholderText: "Password"
                                width: 200
                                echoMode: TextInput.Password  // hides input
                                font.pixelSize: 16
                                                padding: 5
                                                background: Rectangle {
                                                    color: "#E0E0E0"
                                                    radius: 8
                                                }
                            }

                            // Button
                            Button {
                                text: "Submit"
                                width: 100
                                anchors.horizontalCenter: parent.horizontalCenter
                                background: Rectangle {
                                                    color: "#4B5320"   // army green
                                                    radius: 20
                                }
                                onClicked: {
                                    stack.push(passwordcheck)   // push the Candidate page
                                }
                            }
                            Button {
                                text: "Back"
                                onClicked: stack.pop()
                            }
                        }




        }
    }

    // Candidate Page
    Component {
        id: candidatePage

        Page {

            Button {
                text: "Back"
                onClicked: stack.pop()
            }
        }
    }
    Component {
        id: passwordcheck

        Page {

            Button {
                text: "Back"
                onClicked: stack.pop()
            }
        }
    }
}
