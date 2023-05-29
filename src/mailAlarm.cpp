#include "../lib/ESP Mail Client/src/ESP_Mail_Client.h"
#include "mailAlarm.h"
#include "wifiUser.h"

String btc_alarm_price = "0";
String eth_alarm_price = "0";
String bnb_alarm_price = "0";
String okb_alarm_price = "0";

SMTPSession smtp;
Session_Config config;


//获取邮箱状态
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}

/**
 * @brief SMTP初始化配置
 * 
 */
void smtpInit(){
    // /*  Set the network reconnection option */
  	MailClient.networkReconnect(true);

	smtp.debug(1);

  	// /* Set the callback function to get the sending results */
  	smtp.callback(smtpCallback);


	// /* Set the session config */
	config.server.host_name = SMTP_HOST;
	config.server.port = SMTP_PORT;
	config.login.email = AUTHOR_EMAIL;
	config.login.password = AUTHOR_PASSWORD;

	// // For client identity, assign invalid string can cause server rejection
  	config.login.user_domain = F("smtp.qq.com");

	// /* Set the NTP config time */
	config.time.ntp_server = F("cn.pool.ntp.org");
	config.time.gmt_offset = 8;
	config.time.day_light_offset = 0;
}

/**
 * @brief 发送邮件，传入邮件主信息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool sendMail(String msg){

	SMTP_Message message;

	if (!smtp.connect(&config)){
		ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
		return false;
	}

	// /* Set the message headers */
	message.sender.name = F(SENDER_NAME);
	message.sender.email = AUTHOR_EMAIL;
	message.subject = F(HEADER_CONTENT);
	message.addRecipient(F(RECIPIENT_NAME), RECIPIENT_EMAIL);

	message.text.charSet = F("utf-8");
	message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
	message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.text.content = msg;

	
	if (!smtp.isLoggedIn()){
    	Serial.println("\nNot yet logged in.");
	}else{
		if (smtp.isAuthenticated())
		Serial.println("\nSuccessfully logged in.");
		else
		Serial.println("\nConnected with no Auth.");
	}

	/* Start sending Email and close the session */
	if (!MailClient.sendMail(&smtp, &message)){
		ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        return false;
	}

    return true;
}

/**
 * @brief 保存设置的预警价格
 * 
 */
void writeAlarmPriceConfig(){
    if (SPIFFS.begin(true)) {
        File configFile = SPIFFS.open("/alarmprice.txt", "w");
        if (!configFile) {
            Serial.println("Failed to open alarm price config file for writing");
            return;
        }

        // 写入wifi账号和密码到文件中
        configFile.print("BTC=");
        configFile.println(btc_alarm_price);
        configFile.print("ETH=");
        configFile.println(eth_alarm_price);
		configFile.print("BNB=");
        configFile.println(bnb_alarm_price);
		configFile.print("OKB=");
        configFile.println(okb_alarm_price);
        configFile.flush();
        configFile.close();

        Serial.println("Write alarm price config success");
    } else {
        Serial.println("Failed to mount filesystem");
    }
}


void readAlarmPriceConfig(){
	if (SPIFFS.begin(true)) {
		File configFile = SPIFFS.open("/alarmprice.txt", "r");

		if (configFile) {
			String fileContent = configFile.readString();
			configFile.close();

			char buf[fileContent.length() + 1]; // 加上字符串结尾符的长度
			fileContent.toCharArray(buf, sizeof(buf));
			Serial.printf("[INFO] AlarmPrice File content:\r\n%s\r\n\n",buf);

			char *coinname_value = NULL;
			char *coinprice_value = NULL;
			char *token = NULL, *next_token=NULL;
			const char *delim = "=\r\n";
			
			//配置信息不为空，读取里面词条信息
			if(strlen(buf)!=0){
				token = strtok_r(buf, delim, &next_token);
			}
			
			while(token != NULL) {
				if (strcmp(token, "BTC") == 0) { // 找到BTC字段
					token = strtok_r(NULL, delim, &next_token); // 获取ETH对应的值
					Serial.printf("BTC=%s\r\n",token); // 打印BTC的值
					btc_alarm_price = token;
				} else if (strcmp(token, "ETH") == 0) { // 找到ETH字段
					token = strtok_r(NULL, delim, &next_token); // 获取ETH对应的值
					Serial.printf("ETH=%s\r\n",token); // 打印ETH的值
					eth_alarm_price = token;
				} else if (strcmp(token, "BNB") == 0){
					token = strtok_r(NULL, delim, &next_token); // 获取BNB对应的值
					Serial.printf("BNB=%s\r\n",token); // 打印BNB的值
					bnb_alarm_price = token;
				} else if (strcmp(token, "OKB") == 0){
					token = strtok_r(NULL, delim, &next_token); // 获取OKB对应的值
					Serial.printf("OKB=%s\r\n",token); // 打印OKB的值
					okb_alarm_price = token;
				} else {
					token = strtok_r(NULL, delim, &next_token); // 跳过其他字段(比如换行符)
				}
			}
		}
	}
}

/**
 * @brief 设置预警价格功能
 * 
 * @return true 
 * @return false 
 */
void setAlarmPrice(){
	bool flag = true;

	//配置AP热点
	initSoftAP();   
    initDNS();        
    initWebServer(); 

	while(true) {
		checkDNS_HTTP();
        delay(50);
		if(getSetAlarmPriceFlag() == false){
			break;
		}
	}

	//保存设置的价格到alarmprice.txt文件
	writeAlarmPriceConfig();
	return;
}

/**
 * @brief 
 * 
 */
void showAlarmPriceDebug(){
	Serial.printf("BTC/USD:%s\n",btc_alarm_price);
	Serial.printf("ETH/USD:%s\n",eth_alarm_price);
	Serial.printf("BNB/USD:%s\n",bnb_alarm_price);
	Serial.printf("OKB/USD:%s\n",okb_alarm_price);
}

/**
 * @brief 检查当前价格是否达到警戒值
 * 
 * @param price 
 * @param mode 
 * @return true 达到警戒值
 * @return false 未达到警戒值
 */
bool checkAlarmPrice(String price, int mode){
	if(price.toDouble() <= 0){
		Serial.println("Get price error");
		return false;
	}
	
	switch(mode){
		case BTC_MODE:
			if(price.toDouble() <= btc_alarm_price.toDouble()){
				return true;
			}else{
				return false;
			}
			break;
		case ETH_MODE:
			if(price.toDouble() <= eth_alarm_price.toDouble()){
				return true;
			}else{
				return false;
			}
			break;
		case BNB_MODE:
			if(price.toDouble() <= bnb_alarm_price.toDouble()){
				return true;
			}else{
				return false;
			}
			break;
		case OKB_MODE:
			if(price.toDouble() <= okb_alarm_price.toDouble()){
				return true;
			}else{
				return false;
			}
			break;
		default:
			Serial.println("Mode select fail");
			return false;
	}
}

/**
 * @brief 获取警戒值
 * 
 * @param price 
 * @param mode 
 * @return double 
 */
String getAlarmPrice(int mode){
	switch(mode){
		case BTC_MODE:
			return btc_alarm_price;
			break;
		case ETH_MODE:
			return eth_alarm_price;
			break;
		case BNB_MODE:
			return bnb_alarm_price;
			break;
		case OKB_MODE:
			return okb_alarm_price;
			break;
		default:
			return String("0");
	}
}