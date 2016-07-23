import QtQuick 2.4
import Ubuntu.Components 1.3
import avahi 0.1
import cast 0.1

// A simple example talking to the demo "Hello" receiver application
// found here:
//     https://github.com/googlecast/CastHelloText-chrome

MainView {
  id: root
  width: units.gu(40)
  height: units.gu(30)

  Column {
    anchors.fill: parent

    OptionSelector {
      id: selector
      anchors {
        left: parent.left
        right: parent.right
      }
      model: Browser {
        serviceType: "_googlecast._tcp"
      }
      delegate: OptionSelectorDelegate {
        text: model.serviceName
        subText: model.hostName

        readonly property string address: model.address
        readonly property int port: model.port
        readonly property bool current: ListView.isCurrentItem
        onCurrentChanged: {
          if (current) {
            selector.address = address
            selector.port = port
          }
        }
      }

      property string address: ""
      property int port: 0
    }

    Button {
      text: "Connect"
      onClicked: {
        cast.connectToHost(selector.address, selector.port);
      }
    }

    Row {
      TextField {
        id: textField
      }
      Button {
        text: "Send"
        onClicked: {
          if (root.helloIface !== null) {
             root.helloIface.send(textField.text);
          }
        }
      }
    }

    TextArea {
      id: textArea
    }
  }

  Caster {
    id: cast

    onConnected: {
      cast.receiver.launch("794B7BBF")
    }
  }

  property var helloChannel: null
  property var helloIface: null

  Connections {
    target: cast.receiver

    onStatusChanged: {
      var APP_ID = "794B7BBF";
      var r = cast.receiver;
      var app = null;
      for (var i = 0; i < r.applications.length; ++i) {
        if (r.applications[i].appId == APP_ID) {
          app = r.applications[i];
        } else if (!r.applications[i].isIdleScreen) {
          r.stop(r.applications[i].sessionId);
        }
      }
      if (app) {
        root.helloChannel = cast.createChannel("client-0", app.transportId)
        root.helloIface = root.helloChannel.addInterface("urn:x-cast:com.google.cast.sample.helloworld");
      } else {
        r.launch(APP_ID);
      }
    }
  }

  Connections {
    target: helloChannel

    onClosed: {
      console.log("Connection to receiver app closed");
    }
  }

  Connections {
    target: helloIface

    onMessageReceived: {
      if (textArea.text.length != 0) {
        textArea.text += "\n";
      }
      textArea.text += data;
    }
  }
}

