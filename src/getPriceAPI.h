#ifndef __GETPRICEAPI_H__
#define __GETPRICEAPI_H__


#define OK_ACCESS_KEY "d15a2f3a-a9ce-4714-850e-3dedb66db001"
#define WBTC_TOKEN_CONTRACT_ADDRESS "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599"
#define STETH_TOKEN_CONTRACT_ADDRESS "0xae7ab96520de3a18e5e111b5eaab095312d7fe84"
#define BNB_TOKEN_CONTRACT_ADDRESS "0xb8c77482e45f1f44de1745f52c74426c631bdd52"
#define OKB_TOKEN_CONTRACT_ADDRESS "0x75231f58b43240c9718dd58b4967c5114342a86c"
#define OKLINK_TOKENLIST_URL_BASE "https://www.oklink.com/api/v5/explorer/token/token-list?"


double getBitcoinPrices(); //获取WBTC价格
double getETHPrices(); //获取stETH价格
double getBNBPrices(); //获取BNB价格
double getOKBPrices(); //获取OKB价格


#endif
