#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>
#include <Libraries/DHT/DHT.h>


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "FRESHTOMATO 2.4GHz" // Put you SSID and Password here
	#define WIFI_PWD "qawsedrf"
#endif

//* For I2C mode:
// Default I2C pins 0 and 2. Pin 4 - optional reset
// I2C mapping : SCL - D3(GPIO0), SDA - D4(GPIO2)
// Default I2C address is 0x3c!! Check Adafruit_SSD1306.h
Adafruit_SSD1306 display(4);
// for DHT22 temp/humi sensor
DHT dht(14);  // GPIO14 (D5)

// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);
void displayTemp(String s_temp);
void displayText(String s_msg);


Timer procTimer;

// MQTT client
// For quickly check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
MqttClient mqtt("tortuga.iptime.org", 1883, onMessageReceived);


// MQTT : Publish our message
void publishMessage()
{
	if (mqtt.getConnectionState() != eTCS_Connected) {
		startMqttClient(); // Auto reconnect
		return;
	}

	// Read DHT22
//	float h = dht.readHumidity();
//	float t = dht.readTemperature();
//	if (isnan(t) || isnan(h)) {
//		Serial.println("Failed to read from DHT");
//		displayText("Failed to read from DHT");
//		return;
//	}
	TempAndHumidity th;
	if(!dht.readTempAndHumidity(th)) {
		return;
	}

	String message = "T=";
	message += th.temp;
	message += ", H=";
	message += th.humid;

	// publish message
	Serial.println("Let's publish message now!");
//	static char szMsg[50];
//	for(int i=0; i<50; i++) szMsg[i] = 0x00;
//	message.toCharArray(szMsg, 50);
	mqtt.publish("messagebox", message); // or publishWithQoS

	displayTemp(message);
}

// MQTT : Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Prettify alignment for printing
	Serial.println(message);

//	static char szBuf[50];
//	for(int i=0; i<50; i++) szBuf[i] = 0x00;
//	message.toCharArray(szBuf, 50);

	if(message.indexOf("T=") != 0)	// do not display my message
		displayText(message);
}

// Run MQTT client
void startMqttClient()
{
	if(!mqtt.setWill("last/will","The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}
	mqtt.connect("ESP8266");
	mqtt.subscribe("messagebox");
	displayText("Start MQTT client...");
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");
	displayText("I'm CONNECTED to AP");

	// Run MQTT client
	startMqttClient();

	// Start publishing loop
	procTimer.initializeMs(30*60*1000, publishMessage).start(); // every 20 seconds
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");
	displayText("I'm NOT CONNECTED. Need help !!");
	// .. some you code for device configuration ..
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	display.begin(SSD1306_SWITCHCAPVCC);
	//display.display();
	display.clearDisplay();

	dht.begin();

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
}

void displayTemp(String s_temp) {
	//display.clearDisplay();
	display.fillRect(0, 0, 128, 32, BLACK);
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(5,10);
	display.println(s_temp);
	display.display();
}

void displayText(String s_msg) {
	//display.clearDisplay();
	display.fillRect(0, 32, 128, 32, BLACK);
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(5,40);
	display.println(s_msg);
	display.display();
}
