#include <Arduino.h>
#include <IO7F32.h>

String user_html = "" ;

char* ssid_pfix = (char*)"IOTLamp";
unsigned long lastPublishMillis = -pubInterval;
const int RELAY = 15;

void publishData() {
    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");

    data["lamp"] = digitalRead(RELAY) == 1 ? "on" : "off";

    serializeJson(root, msgBuffer);
    client.publish(evtTopic, msgBuffer);
}

void handleUserCommand(char* topic, JsonDocument* root) {
    JsonObject d = (*root)["d"];

    Serial.println(topic);
    if (d.containsKey("lamp")) {
        if (strstr(d["lamp"], "on")) {
            digitalWrite(RELAY, HIGH);
        } else if (strstr(d["lamp"], "off")) {
            digitalWrite(RELAY, LOW);
        } else if (strstr(d["lamp"], "toggle")) {
            digitalWrite(RELAY, !digitalRead(RELAY));
        }
        lastPublishMillis = -pubInterval;
    }
    // USER CODE EXAMPLE
}

void setup() {
    Serial.begin(115200);
    pinMode(RELAY, OUTPUT);

    initDevice();
    JsonObject meta = cfg["meta"];
    pubInterval = meta.containsKey("pubInterval") ? meta["pubInterval"] : 0;
    lastPublishMillis = -pubInterval;

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    // main setup
    Serial.printf("\nIP address : ");
    Serial.println(WiFi.localIP());

    userCommand = handleUserCommand;
    set_iot_server();
    iot_connect();
}

void loop() {
    if (!client.connected()) {
        iot_connect();
    }
    client.loop();
    if ((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval)) {
        publishData();
        lastPublishMillis = millis();
    }
}