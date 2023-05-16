#include "wifiUser.h"
#include "esp_spiffs.h"
#include <String.h>
#include <SPIFFS.h>
#include <string.h>

int connectTimeOut_s = 15;
 
const byte DNS_PORT = 53;                  //设置DNS端口号
const int webPort = 80;                    //设置Web端口号

DNSServer dnsServer;                       //创建dnsServer实例
WebServer server(webPort);                 //开启web服务, 创建TCP SERVER,参数: 端口号,最大连接数

String scanNetworksID = "";                //用于储存扫描到的WiFi ID
IPAddress apIP(192, 168, 4, 1);            //设置AP的IP地址 192.168.4.1
String wifi_ssid = "";                     //暂时存储wifi账号密码
String wifi_pass = "";                     //暂时存储wifi账号密码


void writeWifiConfig(String ssid, String password){
    if (SPIFFS.begin(true)) {
        File configFile = SPIFFS.open("/wifi.txt", "a");
        if (!configFile) {
            Serial.println("Failed to open wifi config file for writing");
            return;
        }

        // 写入wifi账号和密码到文件中
        configFile.print("ssid=");
        configFile.print(ssid);
        configFile.print(";password=");
        configFile.println(password);
        configFile.flush();
        configFile.close();

        Serial.println("Write wifi config success");
    } else {
        Serial.println("Failed to mount filesystem");
    }
}

void deleteWifiConfig(){
    if (SPIFFS.begin(true)) {
        if (SPIFFS.exists("/wifi.txt")){
            SPIFFS.remove("/wifi.txt");
            Serial.println("Delete wifi config success");
        }
        else{
            Serial.println("No wifi config file");
        }
    } else {
        Serial.println("Failed to mount filesystem");
    }
}

 
/*
 * 处理网站根目录的访问请求
 */
void handleRoot(){
    if (server.hasArg("selectSSID")) {
        server.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>");   //scanNetWprksID是扫描到的wifi
    } else {
        server.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>");   
    }
}
 
/*
 * 提交数据后的提示页面
 */
void handleConfigWifi(){               //返回http状态
    if (server.hasArg("ssid")){          //判断是否有账号参数
        Serial.print("get ssid:");
        wifi_ssid = server.arg("ssid");   //获取html表单输入框name名为"ssid"的内容
    
        Serial.println(wifi_ssid);
    }else{                                //没有参数
        Serial.println("error, not found ssid");
        server.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid"); //返回错误页面
        return;
    }

    //密码与账号同理
    if (server.hasArg("pass")){
        Serial.print("get password:");
        wifi_pass = server.arg("pass");  //获取html表单输入框name名为"pwd"的内容
        Serial.println(wifi_pass);
    }else{
        Serial.println("error, not found password");
        server.send(200, "text/html", "<meta charset='UTF-8'>error, not found password");
        return;
    }

    server.send(200, "text/html", "<meta charset='UTF-8'><p><span>SSID:" + wifi_ssid + "</p><br /><p><span>password:" + wifi_pass + "</p><br /><p><span>已取得WiFi信息,正在尝试连接,请手动关闭此页面。</p>"); //返回保存成功页面
    delay(2000);

    WiFi.softAPdisconnect(true);     //参数设置为true，设备将直接关闭接入点模式，即关闭设备所建立的WiFi网络。
    server.close();                  //关闭web服务
    WiFi.softAPdisconnect();         //在不输入参数的情况下调用该函数,将关闭接入点模式,并将当前配置的AP热点网络名和密码设置为空值.
    Serial.println("WiFi Connect SSID:" + wifi_ssid + "  PASS:" + wifi_pass);
    
    if (WiFi.status() != WL_CONNECTED){    //wifi没有连接成功
        connectToWiFi(connectTimeOut_s);
    }else{
        Serial.println("Connected successful in WEB configure");
    }
}
 
/*
 * 处理404情况的函数'handleNotFound'
 */
void handleNotFound(){           // 当浏览器请求的网络资源无法在服务器找到时通过此自定义函数处理         
    handleRoot();                 //访问不存在目录则返回配置页面
    //   server.send(404, "text/plain", "404: Not found");
}
 
/*
 * 进入AP模式
 */
void initSoftAP(){
    WiFi.mode(WIFI_AP);                                           //配置为AP模式
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   //设置AP热点IP和子网掩码
    if (WiFi.softAP(AP_SSID, AP_PWD)){                                     //开启AP热点,如需要密码则添加第二个参数                       
        //打印相关信息
        Serial.println("ESP-32S SoftAP is right.");
        Serial.print("Soft-AP IP address = ");
        Serial.println(WiFi.softAPIP());                                                //接入点ip
        Serial.println(String("MAC address = ")  + WiFi.softAPmacAddress().c_str());    //接入点mac
    }else{                                                  //开启AP热点失败
        Serial.println("WiFiAP Failed");
        delay(1000);
        Serial.println("restart now...");
        ESP.restart();                                      //重启复位esp32
    }
}
 
/*
 * 开启DNS服务器
 */
void initDNS(){
    if (dnsServer.start(DNS_PORT, "*", apIP)){   //判断将所有地址映射到esp32的ip上是否成功
        Serial.println("start dnsserver success.");
    } else {
        Serial.println("start dnsserver failed.");
    }
}
 
/*
 * 初始化WebServer
 */
void initWebServer(){
    if (MDNS.begin("esp32")){    //给设备设定域名esp32,完整的域名是esp32.local
        Serial.println("MDNS responder started");
    }

    //必须添加第二个参数HTTP_GET，以下面这种格式去写，否则无法强制门户
    server.on("/", HTTP_GET, handleRoot);                      //  当浏览器请求服务器根目录(网站首页)时调用自定义函数handleRoot处理，设置主页回调函数，必须添加第二个参数HTTP_GET，否则无法强制门户
    server.on("/configwifi", HTTP_POST, handleConfigWifi);     //  当浏览器请求服务器/configwifi(表单字段)目录时调用自定义函数handleConfigWifi处理
                                                                
    server.onNotFound(handleNotFound);                         //当浏览器请求的网络资源无法在服务器找到时调用自定义函数handleNotFound处理
    
    server.begin();                                           //启动TCP SERVER
    
    Serial.println("WebServer started!");
}
 
/*
 * 扫描附近的WiFi，为了显示在配网界面
 */
bool scanWiFi(){
    Serial.println("scan start");
    Serial.println("--------->");

    // 扫描附近WiFi
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
        scanNetworksID = "no networks found";
        return false;
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {

            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            scanNetworksID += "<P>" + WiFi.SSID(i) + "</P>";
            
            delay(10);
        }
        return true;
    }
}

 
/*
 * 连接WiFi
 */
void connectToWiFi(int timeOut_s){
    WiFi.hostname(HOST_NAME);   //设置设备名

    WiFi.mode(WIFI_STA);        //设置为STA模式并连接WIFI
    WiFi.setAutoConnect(true);  //设置自动连接    

    if (wifi_ssid != ""){       //wifi_ssid不为空，意味着从网页读取到wifi
        Serial.println("Read wifi cinfugure message in WEB");
        WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str()); //c_str(),获取该字符串的指针

        //设置为空，配网后可跳转到SPIFFS查询历史wifi信息
        wifi_ssid = "";
        wifi_pass = "";
    }else{
        Serial.println("Read wifi configure file in SPIFFS\r\n");

        bool isConnected = false;

        if (SPIFFS.begin(true)) {
            File configFile = SPIFFS.open("/wifi.txt", "r");

            if (configFile) {
                String fileContent = configFile.readString();
                configFile.close();

                char buf[fileContent.length() + 1]; // 加上字符串结尾符的长度
                fileContent.toCharArray(buf, sizeof(buf));
                Serial.printf("[INFO] File content:\r\n%s\r\n\n",buf);

                char *ssid_value = NULL;
                char *password_value = NULL;
                char *token = NULL, *next_token=NULL;
                const char *delim = ";=\r\n";
                
                //配置信息不为空，读取里面词条信息
                if(strlen(buf)!=0){
                    Serial.println("Ready to parse token");
                    token = strtok_r(buf, delim, &next_token);
                    Serial.printf("token=%s\r\n",token);
                }
                
                while(token != NULL) {
                    Serial.printf("In while: token=%s\r\n",token);
                    if (strcmp(token, "ssid") == 0) { // 找到ssid字段
                        token = strtok_r(NULL, delim, &next_token); // 获取ssid对应的值
                        Serial.printf("SSID=%s\r\n",token); // 打印ssid的值
                        ssid_value = token;
                    } else if (strcmp(token, "password") == 0) { // 找到password字段
                        token = strtok_r(NULL, delim, &next_token); // 获取password对应的值
                        Serial.printf("PWD=%s\r\n",token); // 打印password的值并且输出空格隔开下一个组合
                        password_value = token;

                        Serial.printf("SSID=%s PWD=%s\r\n",ssid_value,password_value);

                        if(ssid_value != NULL && password_value != NULL && !isConnected) {
                            Serial.print("[INFO] Connecting WiFi: ");
                            Serial.println(ssid_value);
                            WiFi.begin(ssid_value, password_value);
                            delay(500);
                            for (int j = 0; j < 20 && WiFi.status() != WL_CONNECTED; j++) {
                                Serial.print(".");
                                delay(500);
                            }
                            if (WiFi.status() == WL_CONNECTED) {
                                isConnected = true;
                                wifi_ssid = ssid_value;
                                wifi_pass = password_value;
                                Serial.println("");
                                Serial.print("[INFO] Connected seccessful,SSID: ");
                                Serial.print(WiFi.SSID());

                                break;
                            }
                        }
                    } else {
                        token = strtok_r(NULL, delim, &next_token); // 跳过其他字段(比如换行符)
                    }
                }

                //匹配不上存储的wifi信息，准备开启AP配网模式
                if (!isConnected) {
                    Serial.println("");
                    Serial.println("[INFO] Connected fail,please configure Wi-Fi");
                    wifiConfig();   // 进入网络配网模式
                    while (WiFi.status() != WL_CONNECTED){
                        checkDNS_HTTP();
                        delay(50);
                    }

                    //配网连接上网络，保存WIFI信号后返回
                    writeWifiConfig(wifi_ssid, wifi_pass);
                    return;
                }

            } else {
                Serial.println("[ERROR] Can not open wifi configure file!");
            }

        } else {
            Serial.println("[ERROR] Can not read wifi configure file");
            wifiConfig();
            while (WiFi.status() != WL_CONNECTED){
                checkDNS_HTTP();
                delay(50);
            }

            //配网连接上网络，保存WIFI信号后返回
            writeWifiConfig(wifi_ssid, wifi_pass);
            return;
        }
    }

    int Connect_time = 0;                       //用于连接计时，如果长时间连接不成功，复位设备
    while (WiFi.status() != WL_CONNECTED){      //等待WIFI连接成功 
        Serial.print(".");                      //一共打印30个点点  
        delay(500);
        Connect_time ++;
                                        
        if (Connect_time > 2 * timeOut_s){          //长时间连接不上，重新进入配网页面
            Serial.println("");                     //主要目的是为了换行符
            Serial.println("WIFI autoconnect fail, start AP for webconfig now...");
            wifiConfig();                           //开始配网功能
            while(WiFi.status() != WL_CONNECTED){
                checkDNS_HTTP();                  //检测客户端DNS&HTTP请求，也就是检查配网页面那部分
                delay(50);
            }
            break;;                               //跳出 防止无限初始化
        }
    }

    if (WiFi.status() == WL_CONNECTED){          //如果连接成功
        Serial.println("");
        Serial.println("[INFO] WIFI connect Success");
        Serial.printf("SSID:%s", WiFi.SSID().c_str());
        Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
        Serial.print("LocalIP:");
        Serial.print(WiFi.localIP());
        Serial.print(" ,GateIP:");
        Serial.println(WiFi.gatewayIP());
        Serial.print("WIFI status is:");
        Serial.print(WiFi.status());
        server.stop();                            //停止开发板所建立的网络服务器。

        //设置为空，断网后可跳转到SPIFFS查询历史wifi信息
        wifi_ssid = "";
        wifi_pass = "";

    }
}
 

/*
 * 配置配网功能
 */
void wifiConfig(){
    Serial.println("Start to wifi Config");
    initSoftAP();   
    initDNS();        
    initWebServer();  
    scanWiFi();       
}
 
 
/*
 * 删除保存的wifi信息，这里的删除是删除存储在flash的信息。删除后wifi读不到上次连接的记录，需重新配网
 */
void restoreWiFi() {
    delay(500);
    esp_wifi_restore();  //删除保存的wifi信息
    Serial.println("Restore wifi configure file,ready to reboot..");
    delay(10);
}
 
/*
 * 检查wifi是否已经连接
 */
void checkConnect(bool reConnect){
    if (WiFi.status() != WL_CONNECTED){         //wifi连接失败
        if (reConnect == true && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA ) {
            Serial.println("WIFI not connected.");
            Serial.println("WiFi Mode:");
            Serial.println(WiFi.getMode());
            Serial.println("WIFI connecting...");
            connectToWiFi(connectTimeOut_s);          //连接wifi函数 
        }
    }
}
 
 
/*
 * 检测客户端DNS&HTTP请求
 */
void checkDNS_HTTP(){
  dnsServer.processNextRequest();   //检查客户端DNS请求
  server.handleClient();            //检查客户端(浏览器)http请求
}
 