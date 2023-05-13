#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <lvgl.h>
#include <misc/lv_style.h>
// #include <font/lv_font.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "../lib/ArduinoJson/ArduinoJson.h"

#define VERSION "v1.0.02"
#define LV_USE_LOG 0

#define KEY35 35
#define SELF_REFRESH_TIME_SECONDS 10
#define SELF_REFRESH_INTERVAL_MILLISECONDS 500

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

static const uint16_t screenWidth  = TFT_HEIGHT;
static const uint16_t screenHeight = TFT_WIDTH;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenHeight*screenWidth / 10 ];

lv_obj_t *logoImg = NULL;
lv_obj_t *label = NULL;
static lv_style_t style;

DynamicJsonDocument doc(2048);

int getPriceErrCount = 0; //获取价格出错次数
double coinPrice = -1; //WBTC价格
String coinPriceStr = "";
String chainShortName = "eth";
int currentCoinDisplayMode = BTC_MODE;
int lastCoinDisplayMode = BTC_MODE;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

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
			tft.fillScreen(TFT_BLACK);
			return;
		}
		
	}
}

//KEY35中断处理函数
void changeCoinDisplayMode(){
	int count = 1000;

	if(digitalRead(KEY35) == LOW){
		
		//消抖
		while(count--){};

		if(digitalRead(KEY35) == LOW){

			//上一次中断后的处理流程未结束
			if(lastCoinDisplayMode != currentCoinDisplayMode){
				return;
			}

			// tft.fillScreen(TFT_BLACK);
			// tft.setTextColor(TFT_WHITE, TFT_BLACK);

			if(currentCoinDisplayMode == BTC_MODE){
				currentCoinDisplayMode = ETH_MODE;
				// tft.drawCentreString("ETH  MODE",120,60,4);

				//显示BTC LOGO
				LV_IMG_DECLARE(ETH_Logo_50x50);
				lv_img_set_src(logoImg, &ETH_Logo_50x50);
				lv_obj_set_pos(logoImg, 15, 42);

				lv_style_set_text_font(&style, &lv_font_montserrat_20);
				lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_GREY));
				lv_obj_add_style(label, &style, 0);
				lv_obj_align( label, LV_ALIGN_LEFT_MID, 90, 0 );
				lv_label_set_text( label, "ETH  MODE");
				lv_timer_handler();
			}else{
				currentCoinDisplayMode = BTC_MODE;
				//显示BTC LOGO
				LV_IMG_DECLARE(BTC_Logo_50x50);
				lv_img_set_src(logoImg, &BTC_Logo_50x50);
				lv_obj_set_pos(logoImg, 15, 42);

				lv_style_set_text_font(&style, &lv_font_montserrat_20);
				lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_ORANGE));
				lv_obj_add_style(label, &style, 0);
				lv_obj_align( label, LV_ALIGN_LEFT_MID, 90, 0 );
				lv_label_set_text( label, "BTC  MODE");
				lv_timer_handler();
			}
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
	tft.setTextColor(TFT_YELLOW, TFT_BLACK);

	lv_init();    // 初始化 LVGL 库
#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

	lv_disp_draw_buf_init( &draw_buf, buf, NULL,screenHeight*screenWidth / 20 );

	// 初始化缓冲区
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );

    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;   // 设置刷新回调函数
    disp_drv.draw_buf = &draw_buf;   // 设置缓冲区
    lv_disp_drv_register( &disp_drv ); // 注册驱动程序

	lv_color_t black = lv_color_make(0x00, 0x00, 0x00); // 设置颜色常量
    lv_obj_t * scr = lv_disp_get_scr_act(NULL); // 获取当前屏幕对象
	lv_obj_set_style_bg_color(scr, black, 0); // 设置屏幕背景色

	//LOGO对象
	logoImg = lv_img_create(lv_scr_act());

	//显示获取价格标签对象
    label = lv_label_create(lv_scr_act());

	//初始化样式
    lv_style_init(&style);


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
start:

	static char buf[10];
	static bool reconnectFlag = false;

	//设置字体大小
	lv_style_set_text_font(&style, &lv_font_montserrat_30);

	if(currentCoinDisplayMode == BTC_MODE){
		coinPrice = getBitcoinPrices();

		//显示BTC LOGO
		LV_IMG_DECLARE(BTC_Logo_50x50);
		lv_img_set_src(logoImg, &BTC_Logo_50x50);
		lv_obj_set_pos(logoImg, 15, 42);

		//设置BTC价格颜色
		lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_ORANGE));

	}else if(currentCoinDisplayMode == ETH_MODE){
		coinPrice = getETHPrices();

		//显示ETH LOGO
		LV_IMG_DECLARE(ETH_Logo_50x50);
		lv_img_set_src(logoImg, &ETH_Logo_50x50);
		lv_obj_set_pos(logoImg, 15, 42);

		//设置ETH价格颜色
		lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_GREY));

	}
	
	if(coinPrice < 0){
		if(++getPriceErrCount > 3){
			reconnectFlag = true;

			//重连后，重新计数出错次数
			getPriceErrCount = 0;
		}
		sprintf(buf, "ERROR");

		//设置出错颜色
		lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_RED));
	}else{
		sprintf(buf, "$%.0lf", coinPrice);
		
	}

	coinPriceStr = String(buf);
	Serial.println(coinPriceStr);

	lv_obj_add_style(label, &style, 0);
    lv_obj_align( label, LV_ALIGN_LEFT_MID, 90, 0 );
	lv_label_set_text( label, coinPriceStr.c_str() );
	lastCoinDisplayMode = currentCoinDisplayMode;

	//重新连接Wifi
	if(reconnectFlag){
		WiFi_Connect();
		lv_obj_remove_style_all(label);
		reconnectFlag = false;
		goto start;
	}

	lv_timer_handler();

	for(int i=0; i<(SELF_REFRESH_TIME_SECONDS*1000/SELF_REFRESH_INTERVAL_MILLISECONDS); i++){
		delay(SELF_REFRESH_INTERVAL_MILLISECONDS);
		if(lastCoinDisplayMode != currentCoinDisplayMode){
			break;
		}
	}
	lv_obj_remove_style_all(label);
}


