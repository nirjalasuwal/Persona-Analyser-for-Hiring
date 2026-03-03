import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {        
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("Personal Analyser For Hiring")

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: mainPage
    }

    // Main Page
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
                    stack.push(adminPage)
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
                    stack.push(candidatePage)
                }
            }
        }
    }

    // Admin Page
    Component {
        id: adminPage

        Item {
            id: adminRoot

            // Properties
            readonly property string adminPassword: "admin123"
            readonly property int maxAttempts: 3
            readonly property int lockoutDuration: 30

            property int attempts: 0
            property bool isLocked: false
            property int remainingLockTime: 0

            Rectangle {
                anchors.fill: parent
                color: "#f0f2f5"

                ColumnLayout {
                    anchors.centerIn: parent
                    width: parent.width * 0.8
                    spacing: 15

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "System Admin"
                        font.pixelSize: 28
                        font.bold: true
                        color: "#333333"
                    }

                    // Password Field
                    TextField {
                        id: passwordInput
                        Layout.fillWidth: true
                        placeholderText: "Enter Admin Password"
                        echoMode: TextField.Password
                        focus: true
                        enabled: !adminRoot.isLocked

                        background: Rectangle {
                            implicitHeight: 45
                            radius: 6
                            border.color: passwordInput.activeFocus ? "#3f51b5" : "#bdc3c7"
                            border.width: passwordInput.activeFocus ? 2 : 1
                        }

                        onAccepted: {
                            if (text.trim().length > 0 && !adminRoot.isLocked) {
                                loginButton.clicked()
                            }
                        }
                    }

                    Button {
                        id: loginButton
                        text: "Login"
                        Layout.fillWidth: true
                        enabled: !adminRoot.isLocked && passwordInput.text.trim().length > 0

                        contentItem: Text {
                            text: loginButton.text
                            font.pixelSize: 16
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            implicitHeight: 45
                            color: loginButton.enabled ?
                                   (loginButton.pressed ? "#303f9f" : "#3f51b5")
                                   : "#95a5a6"
                            radius: 6
                        }

                        onClicked: {
                            if (passwordInput.text === adminPassword) {
                                statusText.text = "Access Granted!"
                                statusText.color = "#27ae60"
                                passwordInput.text = ""
                                attempts = 0
                            } else {
                                attempts++
                                passwordInput.text = ""

                                if (attempts >= maxAttempts) {
                                    startLockout()
                                } else {
                                    statusText.text =
                                        "Wrong password! " +
                                        (maxAttempts - attempts) +
                                        " attempts left."
                                    statusText.color = "#e74c3c"
                                }
                            }
                        }
                    }

                    Text {
                        id: statusText
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: ""
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                    }
                }

                // Lockout Overlay
                Rectangle {
                    anchors.fill: parent
                    color: "#EEFFFFFF"
                    visible: adminRoot.isLocked

                    MouseArea { anchors.fill: parent }

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 10

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Security Lockout"
                            font.pixelSize: 22
                            font.bold: true
                            color: "#c0392b"
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Too many failed attempts."
                            font.pixelSize: 14
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Try again in: " + adminRoot.remainingLockTime + "s"
                            font.pixelSize: 24
                            font.bold: true
                            color: "#2c3e50"
                        }
                    }
                }
            }

            function startLockout() {
                adminRoot.isLocked = true
                adminRoot.remainingLockTime = adminRoot.lockoutDuration
                lockoutTimer.start()
                statusText.text = ""
            }

            Timer {
                id: lockoutTimer
                interval: 1000
                repeat: true
                onTriggered: {
                    adminRoot.remainingLockTime--

                    if (adminRoot.remainingLockTime <= 0) {
                        lockoutTimer.stop()
                        adminRoot.isLocked = false
                        adminRoot.attempts = 0
                        passwordInput.focus = true
                    }
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
