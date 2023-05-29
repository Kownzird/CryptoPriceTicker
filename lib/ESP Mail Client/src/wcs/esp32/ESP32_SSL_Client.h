/*
 * ESP32 SSL Client v2.0.5
 *
 * Created March 20, 2023
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Provide SSL/TLS functions to ESP32 with Arduino IDE
 * by Evandro Copercini - 2017 - Apache 2.0 License
 */

#ifndef ESP32_SSL_Client_H
#define ESP32_SSL_Client_H

#include <Arduino.h>
#include "ESP_Mail_FS.h"
#if defined(ESP32) && (defined(ENABLE_SMTP) || defined(ENABLE_IMAP))

#include "./extras/MB_FS.h"
#include "./wcs/base/TCP_Client_Base.h"

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

#if !defined(SILENT_MODE)
static const char esp_ssl_client_str_1[] PROGMEM = "Skipping SSL Verification. INSECURE!";
static const char esp_ssl_client_str_2[] PROGMEM = "starting socket";
static const char esp_ssl_client_str_3[] PROGMEM = "opening socket";
static const char esp_ssl_client_str_4[] PROGMEM = "could not get ip from host";
static const char esp_ssl_client_str_5[] PROGMEM = "connecting to Server";
static const char esp_ssl_client_str_6[] PROGMEM = "server connected";
static const char esp_ssl_client_str_7[] PROGMEM = "connect to Server failed!";
static const char esp_ssl_client_str_8[] PROGMEM = "root certificate, PSK identity or keys are required for secured connection";
static const char esp_ssl_client_str_9[] PROGMEM = "seeding the random number generator";
static const char esp_ssl_client_str_10[] PROGMEM = "setting up the SSL/TLS structure";
static const char esp_ssl_client_str_11[] PROGMEM = "loading CA cert";
static const char esp_ssl_client_str_12[] PROGMEM = "setting up PSK";
static const char esp_ssl_client_str_13[] PROGMEM = "pre-shared key not valid hex or too long";
static const char esp_ssl_client_str_14[] PROGMEM = "set mbedtls config";
static const char esp_ssl_client_str_15[] PROGMEM = "loading CRT cert";
static const char esp_ssl_client_str_16[] PROGMEM = "loading private key";
static const char esp_ssl_client_str_17[] PROGMEM = "setting hostname for TLS session";
static const char esp_ssl_client_str_18[] PROGMEM = "perform the SSL/TLS handshake";
static const char esp_ssl_client_str_19[] PROGMEM = "verifying peer X.509 certificate";
static const char esp_ssl_client_str_20[] PROGMEM = "failed to verify peer certificate!";
static const char esp_ssl_client_str_21[] PROGMEM = "certificate verified";
static const char esp_ssl_client_str_22[] PROGMEM = "cleaning SSL connection";
static const char esp_ssl_client_str_23[] PROGMEM = "fingerprint too short";
static const char esp_ssl_client_str_24[] PROGMEM = "invalid hex sequence";
static const char esp_ssl_client_str_25[] PROGMEM = "could not fetch peer certificate";
static const char esp_ssl_client_str_26[] PROGMEM = "fingerprint doesn't match";
static const char esp_ssl_client_str_27[] PROGMEM = "SSL/TLS negotiation";
#endif

typedef void (*_ConnectionRequestCallback)(const char *, int);

class ESP32_SSL_Client
{
    friend class ESP32_WCS;

public:
    ESP32_SSL_Client(){};

    typedef void (*DebugMsgCallback)(PGM_P msg, esp_mail_debug_tag_type type, bool newLine);

    // The SSL context
    typedef struct ssl_context_t
    {

        // using the basic Client
        Client *client = nullptr;

        mbedtls_ssl_context ssl_ctx;
        mbedtls_ssl_config ssl_conf;

        mbedtls_ctr_drbg_context drbg_ctx;
        mbedtls_entropy_context entropy_ctx;

        mbedtls_x509_crt ca_cert;
        mbedtls_x509_crt client_cert;
        mbedtls_pk_context client_key;

        DebugMsgCallback *_debugCallback = NULL;

        // milliseconds SSL handshake time out
        unsigned long handshake_timeout;
    } ssl_ctx;

    void ssl_init(ssl_ctx *ssl);

    /**
     * Upgrade the current connection by setting up the SSL and perform the SSL handshake.
     *
     * @param ssl The pointer to ssl data (context).
     * @param host The server host name.
     * @param rootCABuff The server's root CA or CA cert.
     * @param cli_cert The client cert.
     * @param cli_key The private key.
     * @param pskIdent The Pre Shared Key identity.
     * @param psKey The Pre Shared Key.
     * @param insecure The authentication by-pass option.
     * @return The socket for success or -1 for error.
     * @note The socket should be already open prior to calling this function or shared ssl context with start_tcp_connection.
     */
    int connect_ssl(ssl_ctx *ssl, const char *host, const char *rootCABuff, const char *cli_cert, const char *cli_key, const char *pskIdent, const char *psKey, bool insecure);

    /**
     * Stop the TCP connection and release resources.
     *
     * @param ssl The pointer to ssl data (context).
     * @param rootCABuff The server's root CA or CA cert.
     * @param cli_cert The client cert.
     * @param cli_key The private key.
     * @return The socket for success or -1 for error.
     */
    void stop_tcp_connection(ssl_ctx *ssl, const char *rootCABuff, const char *cli_cert, const char *cli_key);

    /**
     * Get the available data size to read.
     *
     * @param ssl The pointer to ssl data (context).
     * @return The avaiable data size or negative for error.
     */
    int data_to_read(ssl_ctx *ssl);

    /**
     * Send ssl encrypted data.
     *
     * @param ssl The pointer to ssl data (context).
     * @param data The unencrypted data to send.
     * @param len The length of data to send.
     * @return size of data that was successfully send or negative for error.
     */
    int send_ssl_data(ssl_ctx *ssl, const uint8_t *data, size_t len);

    /**
     * Receive ssl decrypted data.
     *
     * @param ssl The pointer to ssl data (context).
     * @param data The data buffer to store decrypted data.
     * @param length The length of decrypted data read.
     * @return size of decrypted data that was successfully read or negative for error.
     */
    int get_ssl_receive(ssl_ctx *ssl, uint8_t *data, int length);

    /**
     * Verify certificate's SHA256 fingerprint.
     *
     * @param ssl The pointer to ssl data (context).
     * @param fp The certificate's SHA256 fingerprint data to compare with server certificate's SHA256 fingerprint.
     * @param domain_name The optional domain name to check in server certificate.
     * @return verification result.
     */
    bool verify_ssl_fingerprint(ssl_ctx *ssl, const char *fp, const char *domain_name);

    /**
     * Verify ssl domain name.
     *
     * @param ssl The pointer to ssl data (context).
     * @param domain_name The domain name.
     * @return verification result.
     */
    bool verify_ssl_dn(ssl_ctx *ssl, const char *domain_name);

    /**
     * Send the mbedTLS error info to the callback.
     *
     * @param ssl The pointer to ssl data (context) which its ssl->_debugCallback will be used.
     * @param errNo The mbedTLS error number that will be translated to string via mbedtls_strerror.
     * @param type The debug tag type.
     */
    void ssl_client_send_mbedtls_error_cb(ssl_ctx *ssl, int errNo, esp_mail_debug_tag_type type);

    /**
     * Send the predefined flash string error to the callback.
     *
     * @param ssl The pointer to ssl data (context) which its ssl->_debugCallback will be used.
     * @param info The PROGMEM error string.
     * @param type The debug tag type.
     */
    void ssl_client_debug_pgm_send_cb(ssl_ctx *ssl, PGM_P info, esp_mail_debug_tag_type type);

    /**
     * Convert Hex char to decimal number
     *
     * @param pb The Hex char.
     * @param res The pointer to result data byte.
     * @return The parsing result.
     */
    bool parseHexNibble(char pb, uint8_t *res);

    /**
     * Compare a name from certificate and domain name
     *
     * @param name The name.
     * @param domainName The domain name.
     * @return The compare result. Return true if they match
     */
    bool matchName(const std::string &name, const std::string &domainName);

#if defined(ENABLE_CUSTOM_CLIENT)
    /**
     * Set the connection request callback.
     * @param connectCB The callback function that accepts the host name (const char*) and port (int) as parameters.
     */
    void connectionRequestCallback(_ConnectionRequestCallback connectCB)
    {
        this->connection_cb = connectCB;
    }

#endif

protected:
#if defined(ENABLE_CUSTOM_CLIENT)
    _ConnectionRequestCallback connection_cb = NULL;
#endif
};

#endif // ESP32

#endif // ESP32_SSL_Client_H
