#include <SPI.h>
#include <WiFi.h>
#include "../lib/TFT_eSPI/TFT_eSPI.h" // Graphics and font library for ST7735 driver chip
#include "../lib/lvgl/lvgl.h"
#include "../lib/ESP Mail Client/src/ESP_Mail_Client.h"

#include "wifiUser.h"
#include "getPriceAPI.h"
#include "mailAlarm.h"

#define VERSION "v1.0.04"
#define LV_USE_LOG 0

#define KEY35 35
#define KEY0  0
#define SELF_REFRESH_TIME_SECONDS 10
#define SELF_REFRESH_INTERVAL_MILLISECONDS 500
#define ALARM_PRICE_COUNT 3

#define BTC_MODE  1
#define ETH_MODE  2
#define BNB_MODE  3
#define OKB_MODE  4


static const uint16_t screenWidth  = TFT_HEIGHT;
static const uint16_t screenHeight = TFT_WIDTH;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenHeight*screenWidth / 10 ];

lv_obj_t *logoImg = NULL;
lv_obj_t *label = NULL;
lv_style_t style;


int getPriceErrCount = 0; //获取价格出错次数
int alarmPriceCount = ALARM_PRICE_COUNT; //检测出低于预警值价格后连续发出警报邮件次数
double coinPrice = -1; //WBTC价格
String coinPriceStr = "";
int currentCoinDisplayMode = BTC_MODE;
int lastCoinDisplayMode = BTC_MODE;
bool key0PressFlag = false;

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
void myDispFlush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ){
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}


//KEY35中断处理函数
void changeCoinDisplayModeHandler(){
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
				alarmPriceCount = ALARM_PRICE_COUNT;

				//显示BTC LOGO
				LV_IMG_DECLARE(ETH_Logo_ARRAY);
				lv_img_set_src(logoImg, &ETH_Logo_ARRAY);
				lv_obj_set_pos(logoImg, 15, 42);

				lv_style_set_text_font(&style, &lv_font_montserrat_20);
				lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_GREY));
				lv_obj_add_style(label, &style, 0);
				lv_obj_align( label, LV_ALIGN_LEFT_MID, 90, 0 );
				lv_label_set_text( label, "ETH  MODE");
				lv_timer_handler();
				lv_style_set_text_font(&style, &lv_font_montserrat_30);
			}else if(currentCoinDisplayMode == ETH_MODE){
				currentCoinDisplayMode = BNB_MODE;
				alarmPriceCount = ALARM_PRICE_COUNT;

				//显示BNB LOGO
				LV_IMG_DECLARE(BNB_Logo_ARRAY);
				lv_img_set_src(logoImg, &BNB_Logo_ARRAY);
				lv_obj_set_pos(logoImg, 15, 42);

				lv_style_set_text_font(&style, &lv_font_montserrat_20);
				lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_YELLOW));
				lv_obj_add_style(label, &style, 0);
				lv_obj_align( label, LV_ALIGN_LEFT_MID, 90, 0 );
				lv_label_set_text( label, "BNB  MODE");
				lv_timer_handler();
				lv_style_set_text_font(&style, &lv_font_montserrat_30);
			}else if(currentCoinDisplayMode == BNB_MODE){
				currentCoinDisplayMode = OKB_MODE;
				alarmPriceCount = ALARM_PRICE_COUNT;

				//显示OKB LOGO
				LV_IMG_DECLARE(OKB_Logo_Array);
				lv_img_set_src(logoImg, &OKB_Logo_Array);
				lv_obj_set_pos(logoImg, 15, 42);

				lv_style_set_text_font(&style, &lv_font_montserrat_20);
				lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_BLUE_GREY));
				lv_obj_add_style(label, &style, 0);
				lv_obj_align( label, LV_ALIGN_LEFT_MID, 90, 0 );
				lv_label_set_text( label, "OKB  MODE");
				lv_timer_handler();
				lv_style_set_text_font(&style, &lv_font_montserrat_30);
			}
			else{
				currentCoinDisplayMode = BTC_MODE;
				alarmPriceCount = ALARM_PRICE_COUNT;
				//显示BTC LOGO
				LV_IMG_DECLARE(BTC_Logo_ARRAY);
				lv_img_set_src(logoImg, &BTC_Logo_ARRAY);
				lv_obj_set_pos(logoImg, 15, 42);

				lv_style_set_text_font(&style, &lv_font_montserrat_20);
				lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_ORANGE));
				lv_obj_add_style(label, &style, 0);
				lv_obj_align( label, LV_ALIGN_LEFT_MID, 90, 0 );
				lv_label_set_text( label, "BTC  MODE");
				lv_timer_handler();
				lv_style_set_text_font(&style, &lv_font_montserrat_30);
			}
		}
	}
}

//KEY0中断处理函数
void key0PressHandler(){
	key0PressFlag = true;
}



//初始化函数
void setup(void) {
	//初始化PIN35,PIN0按键,设置为上拉输入
	pinMode(KEY35, INPUT_PULLUP);
	pinMode(KEY0, INPUT_PULLUP);

	//绑定币种切换显示模式函数，下降沿触发
	attachInterrupt(digitalPinToInterrupt(KEY35), changeCoinDisplayModeHandler, FALLING);
	//绑定清除网络配置函数，下降沿触发
	attachInterrupt(digitalPinToInterrupt(KEY0), key0PressHandler, FALLING);

	//初始化串口，波特率115200
  	Serial.begin(115200); 
	delay(100);

	//TFT屏初始化
	tft.init();
	tft.setRotation(1);
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_YELLOW, TFT_BLACK);

	// 初始化LVGL库
	lv_init();    
#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

	lv_disp_draw_buf_init( &draw_buf, buf, NULL,screenHeight*screenWidth / 20 );

	// 初始化缓冲区
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );

    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = myDispFlush;   // 设置刷新回调函数
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

	//连接网络
	connectToWiFi(15);

	//SMTP服务初始化，使能邮件功能
	smtpInit();
	
	//加载保存的预警价格
	readAlarmPriceConfig();

}


void loop(){
start:

	static char buf[10];
	static bool reconnectFlag = false;

	//设置字体大小
	lv_style_set_text_font(&style, &lv_font_montserrat_30);

	if(currentCoinDisplayMode == BTC_MODE){
		coinPrice = getBitcoinPrices();

		//显示BTC LOGO
		LV_IMG_DECLARE(BTC_Logo_ARRAY);
		lv_img_set_src(logoImg, &BTC_Logo_ARRAY);
		lv_obj_set_pos(logoImg, 15, 42);

		//设置BTC价格颜色
		lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_ORANGE));

	}else if(currentCoinDisplayMode == ETH_MODE){
		coinPrice = getETHPrices();

		//显示ETH LOGO
		LV_IMG_DECLARE(ETH_Logo_ARRAY);
		lv_img_set_src(logoImg, &ETH_Logo_ARRAY);
		lv_obj_set_pos(logoImg, 15, 42);

		//设置ETH价格颜色
		lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_GREY));
	}else if(currentCoinDisplayMode == BNB_MODE){
		coinPrice = getBNBPrices();

		//显示BNB LOGO
		LV_IMG_DECLARE(BNB_Logo_ARRAY);
		lv_img_set_src(logoImg, &BNB_Logo_ARRAY);
		lv_obj_set_pos(logoImg, 15, 42);

		//设置BNB价格颜色
		lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_YELLOW));
	}else if(currentCoinDisplayMode == OKB_MODE){
		coinPrice = getOKBPrices();

		//显示OKB LOGO
		LV_IMG_DECLARE(OKB_Logo_Array);
		lv_img_set_src(logoImg, &OKB_Logo_Array);
		lv_obj_set_pos(logoImg, 15, 42);

		//设置OKB价格颜色
		lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_BLUE_GREY));
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
		if(currentCoinDisplayMode == BNB_MODE){
			sprintf(buf, "$%.1lf", coinPrice);
		}else if(currentCoinDisplayMode == OKB_MODE){
			sprintf(buf, "$%.2lf", coinPrice);
		}else{
			sprintf(buf, "$%.0lf", coinPrice);
		}
		
		
	}

	coinPriceStr = String(buf);
	Serial.println(coinPriceStr);

	lv_obj_add_style(label, &style, 0);
    lv_obj_align( label, LV_ALIGN_LEFT_MID, 90, 0 );
	lv_label_set_text( label, coinPriceStr.c_str() );
	lastCoinDisplayMode = currentCoinDisplayMode;

	//判断价格是否在警戒值内
	if(checkAlarmPrice(String(coinPrice), currentCoinDisplayMode)){
		if(alarmPriceCount){
			alarmPriceCount--;
			String alarmContent = "";
			switch(currentCoinDisplayMode){
				case BTC_MODE:
					alarmContent = "BTC";
					break;
				case ETH_MODE:
					alarmContent = "ETH";
					break;
				case BNB_MODE:
					alarmContent = "BNB";
					break;
				case OKB_MODE:
					alarmContent = "OKB";
					break;
			}

			alarmContent += " Price: ";
			alarmContent += coinPriceStr;
			alarmContent += " \r\nLower than Alarmed: $";
			alarmContent += getAlarmPrice(currentCoinDisplayMode);
			sendMail(alarmContent);
		}
	}else{
		alarmPriceCount = ALARM_PRICE_COUNT;
	}

	//重新连接Wifi
	if(reconnectFlag){
		checkConnect(true);
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

		if(key0PressFlag){
			long long start_time = esp_timer_get_time();
			long long end_time = 0L;

			while(digitalRead(KEY0) == LOW){
				Serial.println("KEY0 is pressed");
				delay(200);
				if(digitalRead(KEY0) == LOW){
					end_time = esp_timer_get_time();

					//长按按键超过5s不释放，执行WIFI信息重置功能
					if((end_time -start_time) >= 5*1000*1000){
						Serial.println("Reset Wifi Message");

						//达到5秒，清除网络配置并重启
						deleteWifiConfig();
						restoreWiFi();
						ESP.restart();
					}
				}else{
					//释放按键，做按压时间判断
					end_time = esp_timer_get_time();
					//KEY0按压标志复位
					key0PressFlag = false;

					//按压时间1s，执行预警价格设置功能
					if((end_time - start_time) >= 1*1000*1000){
						Serial.println("Set Alarm Price");
						setSetAlarmPriceFlag(true);
						setAlarmPrice();
						alarmPriceCount = ALARM_PRICE_COUNT;
						Serial.println("Set Alarm Price Finushed");
						showAlarmPriceDebug();
					}
					
					goto end_sleep_fun;
				}
			}
		}
	}

end_sleep_fun:
	lv_obj_remove_style_all(label);
	checkDNS_HTTP();                  //检测客户端DNS&HTTP请求，也就是检查配网页面那部分
	checkConnect(true);               //检测网络连接状态，参数true表示如果断开重新连接
}


