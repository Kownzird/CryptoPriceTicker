/**
 * This example shows how to append new message to mailbox using the custom IMAP command.
 *
 * Email: suwatchai@outlook.com
 *
 * Github: https://github.com/mobizt/ESP-Mail-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

/** ////////////////////////////////////////////////
 *  Struct data names changed from v2.x.x to v3.x.x
 *  ////////////////////////////////////////////////
 *
 * "ESP_Mail_Session" changes to "Session_Config"
 * "IMAP_Config" changes to "IMAP_Data"
 *
 * Changes in the examples
 *
 * ESP_Mail_Session session;
 * to
 * Session_Config config;
 *
 * IMAP_Config config;
 * to
 * IMAP_Data imap_data;
 *
 */

/** For ESP8266, with BearSSL WiFi Client
 * The memory reserved for completed valid SSL response from IMAP is 16 kbytes which
 * may cause your device out of memory reset in case the memory
 * allocation error.
 */

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else

// Other Client defined here
// To use custom Client, define ENABLE_CUSTOM_CLIENT in  src/ESP_Mail_FS.h.
// See the example Custom_Client.ino for how to use.

#endif

#include <ESP_Mail_Client.h>

// To use only IMAP functions, you can exclude the SMTP from compilation, see ESP_Mail_FS.h.

#define WIFI_SSID "<ssid>"
#define WIFI_PASSWORD "<password>"

/** For Gmail, IMAP option should be enabled. https://support.google.com/mail/answer/7126229?hl=en
 * and also https://accounts.google.com/b/0/DisplayUnlockCaptcha
 *
 * Some Gmail user still not able to sign in using account password even above options were set up,
 * for this case, use "App Password" to sign in instead.
 * About Gmail "App Password", go to https://support.google.com/accounts/answer/185833?hl=en
 *
 * For Yahoo mail, log in to your yahoo mail in web browser and generate app password by go to
 * https://login.yahoo.com/account/security/app-passwords/add/confirm?src=noSrc
 *
 * To use Gmai and Yahoo's App Password to sign in, define the AUTHOR_PASSWORD with your App Password
 * and AUTHOR_EMAIL with your account email.
 */

/* The imap host name e.g. imap.gmail.com for GMail or outlook.office365.com for Outlook */
#define IMAP_HOST "<host>"

/** The imap port e.g.
 * 143  or esp_mail_imap_port_143
 * 993 or esp_mail_imap_port_993
 */
#define IMAP_PORT 993

/* The log in credentials */
#define AUTHOR_EMAIL "<email>"
#define AUTHOR_PASSWORD "<password>"

/* Declare the global used IMAPSession object for IMAP transport */
IMAPSession imap;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void customCommandCallback(IMAP_Response res)
{
    // The server responses will included tagged and/or untagged data.

    // Tagged data is the status which begins with command identifier (tag) i.e. "A01" in this case.
    // Tagged status responses included OK, NO, BAD, PREAUTH and BYE.

    // Untagged data is the information or result of the request which begins with *

    // When you send multiple commands with different tag simultaneously,
    // tag will be used as command identifier.

    ESP_MAIL_PRINTF("> C: TAG %s\n", res.tag.c_str());
    ESP_MAIL_PRINTF("< S: %s\n", res.text.c_str());

    if (res.completed)
    {
        ESP_MAIL_PRINTF("> C: Response finished with status %s\n\n", res.status.c_str());
    }
}

void setup()
{

    Serial.begin(115200);

#if defined(ARDUINO_ARCH_SAMD)
    while (!Serial)
        ;
    Serial.println();
    Serial.println("**** Custom built WiFiNINA firmware need to be installed.****\n");
    Serial.println("To install firmware, read the instruction here, https://github.com/mobizt/ESP-Mail-Client#install-custom-build-wifinina-firmware");
#endif

    Serial.println();

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    /*  Set the network reconnection option */
    MailClient.networkReconnect(true);

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    MailClient.clearAP();
    MailClient.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    /* Declare the Session_Config for user defined session credentials */
    Session_Config config;

    /* Set the session config */
    config.server.host_name = IMAP_HOST;
    config.server.port = IMAP_PORT;

    /* Connect to the server */
    if (!imap.customConnect(&config, customCommandCallback, F("A01") /* tag */))
        return;

    String cmd = F("LOGIN ");
    cmd += AUTHOR_EMAIL;
    cmd += F(" ");
    cmd += AUTHOR_PASSWORD;

    // You can also assign tag to the begining of the command e.g. "A01 FETCH 1 UID"
    // Do not assign tag to command when you assign tag to the last parameter of function.

    imap.sendCustomCommand(cmd, customCommandCallback, F("A02") /* tag */);

    
    if (imap.isAuthenticated())
        Serial.println("\nSuccessfully logged in.\n");
    else
        Serial.println("\nConnected with no Auth.\n");

    imap.sendCustomCommand(F("SELECT \"INBOX\""), customCommandCallback, F("A03") /* tag */);

    imap.sendCustomCommand(F("LIST \"\" *"), customCommandCallback, F("A04") /* tag */);

    String appendMsg = "Date: Thu, 16 Jun 2022 12:30:25 -0800 (PST)\r\n";

    appendMsg += "From: Jack <jack@host.com>\r\n";

    appendMsg += "Subject: Greeting from ESP Mail\r\n";

    appendMsg += "To: joe@host.com\r\n";

    appendMsg += "Message-Id: <jack@host.com>\r\n";

    appendMsg += "MIME-Version: 1.0\r\n";

    appendMsg += "Content-Type: text/plain; charset=\"us-ascii\"\r\n";

    appendMsg += "Content-transfer-encoding: 7bit\r\n";

    appendMsg += "\r\n";

    appendMsg += "Hello Joe, this is the append message\r\n";

    String appendMsgCmd = "APPEND INBOX {" + String(appendMsg.length()) + "}";

    imap.sendCustomCommand(appendMsgCmd, customCommandCallback, F("A05") /* tag */);

    imap.sendCustomData(appendMsg, true /* flag states the last data to send */);
}

void loop()
{
}
