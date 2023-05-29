#include <HTTPClient.h>
#include "../lib/ArduinoJson/ArduinoJson.h"
#include "getPriceAPI.h"

DynamicJsonDocument doc(2048);
String chainShortName = "eth";

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

	if (httpCode > 0){
		if (httpCode == HTTP_CODE_OK){ // 收到正确的内容
			String resBuff = http.getString();

			//	使用ArduinoJson_6.x版本，具体请移步：https://github.com/bblanchon/ArduinoJson
			deserializeJson(doc, resBuff); //开始使用Json解析
			
			JsonVariant stETHPriceJsonVariant = doc["data"][0]["tokenList"][0]["price"];
			price = stETHPriceJsonVariant.as<double>();

		}
	}else{
		Serial.printf("HTTP Get Error: %s\r\n", http.errorToString(httpCode).c_str());
		price = -1;
	}

	http.end();
	return price;
}


//获取BNB价格
double getBNBPrices(){
	HTTPClient http;
	double price = 0;

	String stETHTokenListUrl = (String)OKLINK_TOKENLIST_URL_BASE + "chainShortName=" + chainShortName + "&tokenContractAddress=" + BNB_TOKEN_CONTRACT_ADDRESS;

	http.begin(stETHTokenListUrl);

	http.addHeader("Cookie", "aliyungf_tc=512e21b79e058e7ce565360b7ba5ff90d0c7a752b3e50b6c2a8e276264b02f8e; locale=en_US"); // 添加自定义 header
	http.addHeader("Host", "oklink.com");
	http.addHeader("User-Agent", "PostmanRuntime/7.29.0");
	http.addHeader("Accept", "*/*");
	http.addHeader("Accept-Encoding", "gzip, deflate, br");
	http.addHeader("Connection", "keep-alive");
	http.addHeader("Ok-Access-Key", "d15a2f3a-a9ce-4714-850e-3dedb66db001");
	int httpCode = http.GET();

	if (httpCode > 0){
		if (httpCode == HTTP_CODE_OK){ // 收到正确的内容
			String resBuff = http.getString();

			//	使用ArduinoJson_6.x版本，具体请移步：https://github.com/bblanchon/ArduinoJson
			deserializeJson(doc, resBuff); //开始使用Json解析
			
			JsonVariant stETHPriceJsonVariant = doc["data"][0]["tokenList"][0]["price"];
			price = stETHPriceJsonVariant.as<double>();

		}
	}else{
		Serial.printf("HTTP Get Error: %s\r\n", http.errorToString(httpCode).c_str());
		price = -1;
	}

	http.end();
	return price;
}


//获取BNB价格
double getOKBPrices(){
	HTTPClient http;
	double price = 0;

	String stETHTokenListUrl = (String)OKLINK_TOKENLIST_URL_BASE + "chainShortName=" + chainShortName + "&tokenContractAddress=" + OKB_TOKEN_CONTRACT_ADDRESS;

	http.begin(stETHTokenListUrl);

	http.addHeader("Cookie", "aliyungf_tc=512e21b79e058e7ce565360b7ba5ff90d0c7a752b3e50b6c2a8e276264b02f8e; locale=en_US"); // 添加自定义 header
	http.addHeader("Host", "oklink.com");
	http.addHeader("User-Agent", "PostmanRuntime/7.29.0");
	http.addHeader("Accept", "*/*");
	http.addHeader("Accept-Encoding", "gzip, deflate, br");
	http.addHeader("Connection", "keep-alive");
	http.addHeader("Ok-Access-Key", "d15a2f3a-a9ce-4714-850e-3dedb66db001");
	int httpCode = http.GET();

	if (httpCode > 0){
		if (httpCode == HTTP_CODE_OK){ // 收到正确的内容
			String resBuff = http.getString();

			//	使用ArduinoJson_6.x版本，具体请移步：https://github.com/bblanchon/ArduinoJson
			deserializeJson(doc, resBuff); //开始使用Json解析
			
			JsonVariant stETHPriceJsonVariant = doc["data"][0]["tokenList"][0]["price"];
			price = stETHPriceJsonVariant.as<double>();

		}
	}else{
		Serial.printf("HTTP Get Error: %s\r\n", http.errorToString(httpCode).c_str());
		price = -1;
	}

	http.end();
	return price;
}
