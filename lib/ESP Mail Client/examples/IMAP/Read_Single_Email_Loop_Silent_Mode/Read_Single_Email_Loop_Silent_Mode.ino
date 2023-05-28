/**
 * This example shows how to read Email repeatedly.
 *
 * To get the incoming mail notification, see Mailbox_Changes_Notification.ino.
 *
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: suwatchai@outlook.com
 *
 * Github: https://github.com/mobizt/ESP-Mail-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

/**
 * To use library in silent mode (no debug printing and callback), please define this macro in src/ESP_Mail_FS.h.
 * #define SILENT_MODE
 */

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else

#endif

#include <ESP_Mail_Client.h>

#include <extras/SDHelper.h>

#define WIFI_SSID "<ssid>"
#define WIFI_PASSWORD "<password>"

#define IMAP_HOST "<host>"

#define IMAP_PORT 993

#define AUTHOR_EMAIL "<email>"
#define AUTHOR_PASSWORD "<password>"

void printImapData(IMAP_Status status);

void printAllMailboxesInfo(IMAPSession &imap);

void printSelectedMailboxInfo(SelectedFolderInfo sFolder);

void printMessages(MB_VECTOR<IMAP_MSG_Item> &msgItems, bool headerOnly);

void printAttacements(MB_VECTOR<IMAP_Attach_Item> &atts);

IMAPSession imap;

unsigned long readMillis = 0;
int totalMessage = 0;
int msgNum = 0;
int sign = -1;

Session_Config config;

IMAP_Data imap_data;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

// For Free Heap checking
#include "HeapStat.h"
HeapStat heapInfo;

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

    MailClient.networkReconnect(true);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    MailClient.clearAP();
    MailClient.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

#if defined(ESP_MAIL_DEFAULT_SD_FS) // defined in src/ESP_Mail_FS.h
    // Mount SD card.
    SD_Card_Mounting(); // See src/extras/SDHelper.h
#endif

    config.server.host_name = IMAP_HOST;
    config.server.port = IMAP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;

    imap_data.fetch.uid.clear();

    imap_data.search.criteria.clear();

    imap_data.search.unseen_msg = true;

    imap_data.storage.saved_path = F("/email_data");

    imap_data.storage.type = esp_mail_file_storage_type_sd;

    imap_data.download.header = false;
    imap_data.download.text = false;
    imap_data.download.html = false;
    imap_data.download.attachment = false;
    imap_data.download.inlineImg = false;

    imap_data.enable.html = true;
    imap_data.enable.text = true;

    imap_data.enable.recent_sort = true;

    imap_data.enable.download_status = true;

    imap_data.limit.search = 5;

    imap_data.limit.msg_size = 512;

    imap_data.limit.attachment_size = 1024 * 1024 * 5;

    if (!imap.connect(&config, &imap_data))
    {
        ESP_MAIL_PRINTF("Connection error, Error Code: %d, Reason: %s", imap.errorCode(), imap.errorReason().c_str());
        return;
    }

    if (imap.isAuthenticated())
        Serial.println("\nSuccessfully logged in.");
    else
        Serial.println("\nConnected with no Auth.");

    printAllMailboxesInfo(imap);

    if (!imap.selectFolder(F("INBOX")))
    {
        ESP_MAIL_PRINTF("Folder selection error, Error Code: %d, Reason: %s", imap.errorCode(), imap.errorReason().c_str());
        return;
    }

    printSelectedMailboxInfo(imap.selectedFolder());

    totalMessage = imap.selectedFolder().msgCount();

    msgNum = totalMessage;
    sign = -1; // count down
}

void loop()
{
    if (millis() - readMillis > 10000 || readMillis == 0)
    {
        readMillis = millis();

        if (!imap.connected())
        {
            if (!imap.connect(&config, &imap_data))
                return;

            if (!imap.selectFolder(F("INBOX")))
                return;

            if (totalMessage == 0)
            {
                totalMessage = imap.selectedFolder().msgCount();
                msgNum = totalMessage;
                sign = -1;
            }
        }

        if (msgNum <= 0)
        {
            msgNum = 1;
            sign = 1;
        }
        else if (msgNum >= totalMessage)
        {
            msgNum = totalMessage;
            sign = -1;
        }

        imap_data.fetch.number = msgNum;

        if (MailClient.readMail(&imap, false))
        {
            printImapData(imap.status());
        }
        else
        {
            ESP_MAIL_PRINTF("Message reading error, Error Code: %d, Reason: %s", imap.errorCode(), imap.errorReason().c_str());
        }

        imap.empty();

        msgNum += sign;
    }
}

void printImapData(IMAP_Status status)
{

    if (status.success())
    {
        IMAP_MSG_List msgList = imap.data();
        printMessages(msgList.msgItems, imap.headerOnly());

        imap.empty();

        heapInfo.collect();

        heapInfo.print();

        Serial.println();
    }
}

void printAllMailboxesInfo(IMAPSession &imap)
{

    FoldersCollection folders;

    if (imap.getFolders(folders))
    {
        for (size_t i = 0; i < folders.size(); i++)
        {
            FolderInfo folderInfo = folders.info(i);
            ESP_MAIL_PRINTF("%s%s%s", i == 0 ? "\nAvailable folders: " : ", ", folderInfo.name, i == folders.size() - 1 ? "\n" : "");
        }
    }
}

void printSelectedMailboxInfo(SelectedFolderInfo sFolder)
{
    /* Show the mailbox info */
    ESP_MAIL_PRINTF("\nInfo of the selected folder\nTotal Messages: %d\n", sFolder.msgCount());
    ESP_MAIL_PRINTF("UID Validity: %d\n", sFolder.uidValidity());
    ESP_MAIL_PRINTF("Predicted next UID: %d\n", sFolder.nextUID());
    ESP_MAIL_PRINTF("Unseen Message Index: %d\n", sFolder.unseenIndex());
    if (sFolder.modSeqSupported())
        ESP_MAIL_PRINTF("Highest Modification Sequence: %d\n", sFolder.highestModSeq());
    for (size_t i = 0; i < sFolder.flagCount(); i++)
        ESP_MAIL_PRINTF("%s%s%s", i == 0 ? "Flags: " : ", ", sFolder.flag(i).c_str(), i == sFolder.flagCount() - 1 ? "\n" : "");

    if (sFolder.flagCount(true))
    {
        for (size_t i = 0; i < sFolder.flagCount(true); i++)
            ESP_MAIL_PRINTF("%s%s%s", i == 0 ? "Permanent Flags: " : ", ", sFolder.flag(i, true).c_str(), i == sFolder.flagCount(true) - 1 ? "\n" : "");
    }
}

void printAttacements(MB_VECTOR<IMAP_Attach_Item> &atts)
{
    ESP_MAIL_PRINTF("Attachment: %d file(s)\n****************************\n", atts.size());
    for (size_t j = 0; j < atts.size(); j++)
    {
        IMAP_Attach_Item att = atts[j];
        ESP_MAIL_PRINTF("%d. Filename: %s, Name: %s, Size: %d, MIME: %s, Type: %s, Description: %s, Creation Date: %s\n", j + 1, att.filename, att.name, att.size, att.mime, att.type == esp_mail_att_type_attachment ? "attachment" : "inline", att.description, att.creationDate);
    }
    Serial.println();
}

void printMessages(MB_VECTOR<IMAP_MSG_Item> &msgItems, bool headerOnly)
{

    for (size_t i = 0; i < msgItems.size(); i++)
    {

        IMAP_MSG_Item msg = msgItems[i];

        Serial.println("****************************");
        ESP_MAIL_PRINTF("Number: %d\n", msg.msgNo);
        ESP_MAIL_PRINTF("UID: %d\n", msg.UID);
        ESP_MAIL_PRINTF("Messsage-ID: %s\n", msg.ID);

        ESP_MAIL_PRINTF("Flags: %s\n", msg.flags);

        ESP_MAIL_PRINTF("Attachment: %s\n", msg.hasAttachment ? "yes" : "no");

        if (strlen(msg.acceptLang))
            ESP_MAIL_PRINTF("Accept Language: %s\n", msg.acceptLang);
        if (strlen(msg.contentLang))
            ESP_MAIL_PRINTF("Content Language: %s\n", msg.contentLang);
        if (strlen(msg.from))
            ESP_MAIL_PRINTF("From: %s\n", msg.from);
        if (strlen(msg.sender))
            ESP_MAIL_PRINTF("Sender: %s\n", msg.sender);
        if (strlen(msg.to))
            ESP_MAIL_PRINTF("To: %s\n", msg.to);
        if (strlen(msg.cc))
            ESP_MAIL_PRINTF("CC: %s\n", msg.cc);
        if (strlen(msg.date))
        {
            ESP_MAIL_PRINTF("Date: %s\n", msg.date);
            ESP_MAIL_PRINTF("Timestamp: %d\n", (int)MailClient.Time.getTimestamp(msg.date));
        }
        if (strlen(msg.subject))
            ESP_MAIL_PRINTF("Subject: %s\n", msg.subject);
        if (strlen(msg.reply_to))
            ESP_MAIL_PRINTF("Reply-To: %s\n", msg.reply_to);
        if (strlen(msg.return_path))
            ESP_MAIL_PRINTF("Return-Path: %s\n", msg.return_path);
        if (strlen(msg.in_reply_to))
            ESP_MAIL_PRINTF("In-Reply-To: %s\n", msg.in_reply_to);
        if (strlen(msg.references))
            ESP_MAIL_PRINTF("References: %s\n", msg.references);
        if (strlen(msg.comments))
            ESP_MAIL_PRINTF("Comments: %s\n", msg.comments);
        if (strlen(msg.keywords))
            ESP_MAIL_PRINTF("Keywords: %s\n", msg.keywords);

        if (!headerOnly)
        {
            if (strlen(msg.text.content))
                ESP_MAIL_PRINTF("Text Message: %s\n", msg.text.content);
            if (strlen(msg.text.charSet))
                ESP_MAIL_PRINTF("Text Message Charset: %s\n", msg.text.charSet);
            if (strlen(msg.text.transfer_encoding))
                ESP_MAIL_PRINTF("Text Message Transfer Encoding: %s\n", msg.text.transfer_encoding);
            if (strlen(msg.html.content))
                ESP_MAIL_PRINTF("HTML Message: %s\n", msg.html.content);
            if (strlen(msg.html.charSet))
                ESP_MAIL_PRINTF("HTML Message Charset: %s\n", msg.html.charSet);
            if (strlen(msg.html.transfer_encoding))
                ESP_MAIL_PRINTF("HTML Message Transfer Encoding: %s\n\n", msg.html.transfer_encoding);

            if (msg.rfc822.size() > 0)
            {
                ESP_MAIL_PRINTF("\r\nRFC822 Messages: %d message(s)\n****************************\n", msg.rfc822.size());
                printMessages(msg.rfc822, headerOnly);
            }

            if (msg.attachments.size() > 0)
                printAttacements(msg.attachments);
        }

        Serial.println();
    }
}
