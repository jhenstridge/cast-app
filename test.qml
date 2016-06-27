import QtQuick 2.4
import Ubuntu.Components 1.3
import cast 0.1

MainView {
  width: units.gu(40)
  height: units.gu(71)

  ListView {
    anchors.fill: parent
    model: Browser {
      serviceType: "_workstation._tcp"
    }
    delegate: ListItem {
      Label {
        text: serviceName + " " + hostName + " (" + address + " : " + port + ")"
      }
    }
  }
}
