/*
 An example digital clock using a TFT LCD screen to show the time.
 Demonstrates use of the font printing routines. (Time updates but date does not.)
 
 For a more accurate clock, it would be better to use the RTClib library.
 But this is just a demo. 
 
 This examples uses the hardware SPI only. Non-hardware SPI
 is just too slow (~8 times slower!)
 
 Based on clock sketch by Gilchrist 6/2/2014 1.0
 Updated by Bodmer
A few colour codes:
 
code	color
0x0000	Black
0xFFFF	White
0xBDF7	Light Gray
0x7BEF	Dark Gray
0xF800	Red
0xFFE0	Yellow
0xFBE0	Orange
0x79E0	Brown
0x7E0	Green
0x7FF	Cyan
0x1F	Blue
0xF81F	Pink

 */

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "../lib/ArduinoJson/ArduinoJson.h"

#define WIFI_NAME "Redmi K40"
#define WIFI_PWD "123456789o"

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


byte omm = 99;
boolean initial = 1;
byte xcolon = 0;
unsigned int colour = 0;

DynamicJsonDocument doc(2048);

double wbtcPrice = 0; //WBTC价格
String wbtcPriceStr = "";
String OK_ACCESS_KEY = "d15a2f3a-a9ce-4714-850e-3dedb66db001";
String chainShortName = "eth";
String tokenContractAddress = "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599";
String oklinkBtcPriceUrl = "https://www.oklink.com/api/v5/explorer/token/token-list?chainShortName=" + chainShortName + "&tokenContractAddress=" + tokenContractAddress;

//	WiFi的初始化和连接
void WiFi_Connect()
{
	WiFi.begin(WIFI_NAME, WIFI_PWD);
	while (WiFi.status() != WL_CONNECTED)
	{ //这里是阻塞程序，直到连接成功
		delay(300);
		Serial.print(".");
    // tft.drawCentreString(".",0,0,7);
	}
}

//获取WBTC价格
double getBitcoinPrices(){
	HTTPClient http;
	double price = 0;

	http.begin(oklinkBtcPriceUrl);

	http.addHeader("Cookie", "aliyungf_tc=512e21b79e058e7ce565360b7ba5ff90d0c7a752b3e50b6c2a8e276264b02f8e; locale=en_US"); // 添加自定义 header
	http.addHeader("Host", "oklink.com");
	http.addHeader("User-Agent", "PostmanRuntime/7.29.0");
	http.addHeader("Accept", "*/*");
	http.addHeader("Accept-Encoding", "gzip, deflate, br");
	http.addHeader("Connection", "keep-alive");
	http.addHeader("Ok-Access-Key", "d15a2f3a-a9ce-4714-850e-3dedb66db001");
	int httpCode = http.GET();

	if (httpCode > 0)
	{
		// httpCode will be negative on error
		// Serial.printf("HTTP Get Code: %d\r\n", httpCode);

		if (httpCode == HTTP_CODE_OK) // 收到正确的内容
		{
			String resBuff = http.getString();

			// Serial.println(resBuff);

			//	使用ArduinoJson_6.x版本，具体请移步：https://github.com/bblanchon/ArduinoJson
			deserializeJson(doc, resBuff); //开始使用Json解析
			
			JsonVariant wbtcPriceJsonVariant = doc["data"][0]["tokenList"][0]["price"];
			price = wbtcPriceJsonVariant.as<double>();

			// char buf[10];
			// sprintf(buf, "%.2lf", wbtcPrice);
			// wbtcPriceStr = String(buf);

			// Serial.printf("WBTC/USD: %s\r\n",wbtcPriceStr);

		}
	}
	else
	{
		Serial.printf("HTTP Get Error: %s\r\n", http.errorToString(httpCode).c_str());
		price = 0;
	}

	http.end();
	return price;
}

void setup(void) {
  Serial.begin(115200); // open the serial port at 115200 bps;
	delay(100);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour

  Serial.print("Connecting..");

	WiFi_Connect();

	Serial.println("WiFi connected");

	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

}

void loop() {

	wbtcPrice = getBitcoinPrices();

	char buf[10];
	sprintf(buf, "%.2lf", wbtcPrice);
	wbtcPriceStr = String(buf);


	Serial.println(wbtcPriceStr);
	tft.setTextColor(0xFBE0, TFT_BLACK);
	tft.drawString(wbtcPriceStr, 20, TFT_WIDTH/2-20, 6);

	delay(1000 * 10);
}



