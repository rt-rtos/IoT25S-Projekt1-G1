#include <HiTECH_R4_Wifi_Manager.h>

WiFiManager wifiManager;

const char *myAPName = "HiTECH_Config_AP";

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
    }

    wifiManager.addParameter("mqtt_server", "MQTT Broker IP", "192.168.1.100");
    wifiManager.addParameter("device_id", "Device ID", "HTC_NODE_01");

    wifiManager.setAPCallback([]()
                              { Serial.println("[Event] Switched to AP Mode for configuration."); });

    wifiManager.setSaveCallback([]()
                                { Serial.println("[Event] Configuration saved successfully."); });

    wifiManager.setConnectCallback([]()
                                   { Serial.println("[Event] Network connected."); });

    bool isConnected = wifiManager.autoConnect(myAPName);

    Serial.println("\r\n=================================");
    Serial.println("       Wifi Manager       ");
    Serial.println("=================================");

    if (isConnected)
    {
        Serial.println("Status  : CONNECTED");
        Serial.print("IP Addr : ");
        Serial.println(WiFi.localIP());
        Serial.print("MQTT IP : ");
        Serial.println(wifiManager.getParameter("mqtt_server"));
        Serial.print("Dev ID  : ");
        Serial.println(wifiManager.getParameter("device_id"));
    }
    else
    {
        Serial.println("Status  : AP MODE ACTIVATED");
        Serial.print("AP Name : ");
        Serial.println(myAPName);
        Serial.println("URL     : http://192.168.4.1");
    }

    Serial.println("=================================\r\n");
}

void loop()
{
    wifiManager.process();

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 10000)
    {
        lastPrint = millis();
        Serial.print("[System Monitor] Current Network Status: ");

        switch (wifiManager.getStatus())
        {
        case WiFiManager::STATUS_IDLE:
            Serial.println("IDLE");
            break;
        case WiFiManager::STATUS_AP_MODE:
            Serial.println("AP_MODE");
            break;
        case WiFiManager::STATUS_CONNECTING:
            Serial.println("CONNECTING");
            break;
        case WiFiManager::STATUS_CONNECTED:
            Serial.println("CONNECTED");
            break;
        case WiFiManager::STATUS_RECONNECTING:
            Serial.println("RECONNECTING");
            break;
        }
    }
}