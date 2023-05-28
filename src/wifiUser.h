#ifndef __WIFIUSER_H__
#define __WIFIUSER_H__
 
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>      //用于设备域名 MDNS.begin("esp32")
#include <esp_wifi.h>     //用于esp_wifi_restore() 删除保存的wifi信息

#define AP_SSID  "CPTicker_AP"      //设置AP热点名称
#define AP_PWD   "o123456789"       //设置设置AP热点密码
#define HOST_NAME "CPTicker"        //设置设备名

#define ROOT_HTML  "<!DOCTYPE html><html><head><title>WIFI</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><style type=\"text/css\">.input{display: block; margin-top: 10px;}.input span{width: 100px; float: left; float: left; height: 36px; line-height: 36px;}.input input{height: 30px;width: 200px;}.btn{width: 120px; height: 35px; background-color: #000000; border:0px; color:#ffffff; margin-top:15px; margin-left:100px;}</style><body><a href=\"/configAlarmPrice\"><button>Set Alarm<button></a><form method=\"POST\" action=\"configwifi\"><label class=\"input\"><span>WiFi SSID</span><input type=\"text\" name=\"ssid\" value=\"\"></label><label class=\"input\"><span>WiFi PASS</span> <input type=\"text\"  name=\"pass\"></label><input class=\"btn\" type=\"submit\" name=\"submit\" value=\"Submit\"> <p><span> Nearby wifi:</P></form>"
// #define SET_ALARM_HTML(btc,eth,bnb,okb) "<!DOCTYPE html><html><head><title>ALARM</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><style type=\"text/css\">.input{display: block; margin-top: 10px;}.input span{width: 100px; float: left; float: left; height: 36px; line-height: 36px;}.input input{height: 30px;width: 200px;}.btn{width: 120px; height: 35px; background-color: #000000; border:0px; color:#ffffff; margin-top:15px; margin-left:100px;}</style><body><form method=\"POST\" action=\"configAlarmPrice\"><label class=\"input\"><span>BTC/USD</span><input type=\"text\" name=\"BTC\" placeholder="## btc ##" value=\"\"></label><label class=\"input\"><span>ETH/USD</span> <input type=\"text\"  name=\"ETH\" placeholder="## eth ##" value=\"\"></label><label class=\"input\"><span>BNB/USD</span><input type=\"text\" name=\"BNB\" placeholder="## bnb ## " value=\"\"></label><label class=\"input\"><span>OKB/USD</span><input type=\"text\" name=\"OKB\" placeholder="## btc ##" value=\"\"></label><input class=\"btn\" type=\"submit\" name=\"submit\" value=\"Submit\"></form></body></html>"

// extern const char* HOST_NAME;                 //设置设备名
// extern int connectTimeOut_s;                 //WiFi连接超时时间，单位秒
 

//===========需要调用的函数===========
void checkConnect(bool reConnect);    //检测wifi是否已经连接
void restoreWiFi();                   //删除保存的wifi信息
void checkDNS_HTTP();                 //检测客户端DNS&HTTP请求
void connectToWiFi(int timeOut_s);    //连接WiFi
 
//===========内部函数===========
void handleRoot();                    //处理网站根目录的访问请求
void handleConfigWifi() ;             //提交数据后的提示页面
void handleNotFound();                //处理404情况的函数'handleNotFound'
void initSoftAP();                    //进入AP模式
void initDNS();                       //开启DNS服务器
void initWebServer();                 //初始化WebServer
bool scanWiFi();                      //扫描附近的WiFi，为了显示在配网界面
void wifiConfig();                    //配置配网功能
void deleteWifiConfig();
bool getSetAlarmPriceFlag();
void setSetAlarmPriceFlag(bool enable);
 
#endif