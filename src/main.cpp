
/* 
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

#define VERSION "v1.0.02"

#define KEY35 35

#define BTC_MODE  1
#define ETH_MODE  2

#define OK_ACCESS_KEY "d15a2f3a-a9ce-4714-850e-3dedb66db001"
#define WBTC_TOKEN_CONTRACT_ADDRESS "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599"
#define STETH_TOKEN_CONTRACT_ADDRESS "0xae7ab96520de3a18e5e111b5eaab095312d7fe84"
#define OKLINK_TOKENLIST_URL_BASE "https://www.oklink.com/api/v5/explorer/token/token-list?"



String wifiMsgBuf[10][2] = {
	{"xxx1","yyy1"},
	{"xxx2","yyy2"},
	{"xxx3","yyy3"}
};


DynamicJsonDocument doc(2048);

int getPriceErrCount = 0; //获取价格出错次数
double coinPrice = -1; //WBTC价格
String coinPriceStr = "";
String chainShortName = "eth";
int currentCoinDisplayMode = BTC_MODE;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

//获取已存储wifi信息数量
int getWifiMsgCount(){
	int index = 0;
	int capacity = 0;

	//获取wifi信息总容量
	capacity = sizeof(wifiMsgBuf)/sizeof(wifiMsgBuf[0]);

	for(; index < capacity; index++){
		if(wifiMsgBuf[index][0] != "\0" && wifiMsgBuf[index][1]!="\0"){
			continue;
		}else{
			break;
		}
	}

	return index;
}

//	WiFi的初始化和连接
void WiFi_Connect()
{
	int wifiMsgCount = getWifiMsgCount();
	int index = 0;
	int timeout = 20;
	bool connectFlag = false;

	//循环遍历wifi信号
	for(; index < wifiMsgCount; index++){
		Serial.printf("Select Wifi: %s\n",wifiMsgBuf[index][0].c_str());

		//清屏
		tft.fillScreen(TFT_BLACK);
		WiFi.begin(wifiMsgBuf[index][0].c_str(), wifiMsgBuf[index][1].c_str());
		while (WiFi.status() != WL_CONNECTED)
		{
			connectFlag = true;

			//这里是阻塞程序，直到连接成功
			delay(300);
			Serial.print(".");
			
			tft.setTextColor(TFT_WHITE, TFT_BLACK);
			tft.drawCentreString("Connect WIFI",120,30,2);
      		tft.drawCentreString(wifiMsgBuf[index][0].c_str(),120,60,4);

			if((--timeout) <= 0){
				timeout = 20;
				connectFlag = false;
				Serial.println("Connect timeout");
				break;
			}
		}

		if(index >= (wifiMsgCount-1)){
			Serial.println("index = 0");
			//回到首个wifi继续尝试连接
			index = -1;
		}

		//未超时，已正常连接
		if(connectFlag){
			return;
		}
		
	}
}


//KEY35中断处理函数
void changeCoinDisplayMode(){

	if(digitalRead(KEY35) == LOW){

		//切换显示模式标志位
		Serial.println("change Display Mode");

		tft.fillScreen(TFT_BLACK);
		tft.setTextColor(TFT_WHITE, TFT_BLACK);

		if(currentCoinDisplayMode == BTC_MODE){
			currentCoinDisplayMode = ETH_MODE;
			tft.drawCentreString("ETH  MODE",120,60,4);
		}else{
			currentCoinDisplayMode = BTC_MODE;
			tft.drawCentreString("BTC  MODE",120,60,4);
		}

	}
}

//初始化函数
void setup(void) {
	//初始化PIN35按键,设置为上拉输入
	pinMode(KEY35, INPUT_PULLUP);
	//绑定币种切换显示模式函数，下降沿触发
	attachInterrupt(digitalPinToInterrupt(KEY35), changeCoinDisplayMode, FALLING);

	//初始化串口，波特率115200
  	Serial.begin(115200); 
	delay(100);

	tft.init();
	tft.setRotation(1);
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour

	Serial.print("Connecting..");

	WiFi_Connect();

	Serial.println("\r\nWiFi connected");

	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

}

//获取WBTC价格
double getBitcoinPrices(){
	HTTPClient http;
	double price = 0;

	String wbtcTokenListUrl = (String)OKLINK_TOKENLIST_URL_BASE + "chainShortName=" + chainShortName + "&tokenContractAddress=" + WBTC_TOKEN_CONTRACT_ADDRESS;

	http.begin(wbtcTokenListUrl);

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

		}
	}
	else
	{
		Serial.printf("HTTP Get Error: %s\r\n", http.errorToString(httpCode).c_str());
		price = -1;
	}

	http.end();
	return price;
}

//获取stETH价格
double getETHPrices(){
	HTTPClient http;
	double price = 0;

	String stETHTokenListUrl = (String)OKLINK_TOKENLIST_URL_BASE + "chainShortName=" + chainShortName + "&tokenContractAddress=" + STETH_TOKEN_CONTRACT_ADDRESS;

	http.begin(stETHTokenListUrl);

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
		if (httpCode == HTTP_CODE_OK) // 收到正确的内容
		{
			String resBuff = http.getString();

			//	使用ArduinoJson_6.x版本，具体请移步：https://github.com/bblanchon/ArduinoJson
			deserializeJson(doc, resBuff); //开始使用Json解析
			
			JsonVariant stETHPriceJsonVariant = doc["data"][0]["tokenList"][0]["price"];
			price = stETHPriceJsonVariant.as<double>();

		}
	}
	else
	{
		Serial.printf("HTTP Get Error: %s\r\n", http.errorToString(httpCode).c_str());
		price = -1;
	}

	http.end();
	return price;
}

void loop() {
	char buf[10];
	int fontType = 6;
	bool reconnectFlag = false;

	if(currentCoinDisplayMode == BTC_MODE){
		coinPrice = getBitcoinPrices();
	}else if(currentCoinDisplayMode == ETH_MODE){
		coinPrice = getETHPrices();
	}
	
	if(coinPrice < 0){
		if(++getPriceErrCount > 3){
			reconnectFlag = true;

			//重连后，重新计数出错次数
			getPriceErrCount = 0;
			
		}

		fontType = 4;
		tft.fillScreen(TFT_BLACK);
		tft.setTextColor(TFT_RED, TFT_BLACK);
		sprintf(buf, "ERROR");
	}else{
		fontType = 6;
		tft.fillScreen(TFT_BLACK);
		tft.setTextColor(TFT_ORANGE, TFT_BLACK);
		sprintf(buf, "%.2lf", coinPrice);
	}

	coinPriceStr = String(buf);
	Serial.println(coinPriceStr);
	
	// tft.drawString(coinPriceStr, 20, TFT_WIDTH/2-20, 6);
	tft.drawCentreString(coinPriceStr.c_str(),120,50, fontType);

	if(reconnectFlag){
		//重新连接Wifi
		WiFi_Connect();
	}

	delay(1000 * 10);
}



