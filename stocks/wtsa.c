/*#define bool int
#define string char*
#define false 0
#define true 1
#define Bid 1
#define Ask 0
#define MODE_SPREAD 0
#define Point 0.0001
#define PERIOD_MN1 6
#define PERIOD_W1 6
#define PERIOD_D1 6
#define PERIOD_H4 6
#define PERIOD_H1 6
#define PERIOD_M30 6
#define PERIOD_M15 6
#define PERIOD_M5 6
#define PERIOD_M1 6
#define OP_BUY 6
#define OP_SELL 6

#define ObjectDelete()()
#define OrderTicket()()
#define OrderSwap()()
#define OrderModify()()
#define OrderType()()
#define OrderLots()()
#define AccountProfit()()
#define Symbol()()
#define StringFind()()
#define StrToInteger()()
#define StrToDouble()()
#define GetLastError()()
#define ErrorDescription()()
#define ObjectCreate()()
#define ObjectSet()()
#define ObjectSetText()()
#define ObjectDescription()()
#define iOpen()()
#define iClose()()
#define iHigh()()
#define iLow()()
#define MarketInfo()()
#define iLow()()
#define AccountBalance()()
#define NormalizeDouble()()
#define iCustom()()
#define iTime()()
#define RefreshRates()()
#define OrderSend()()
#define OrdersTotal()()
#define OrderSelect()()
#define OrderSymbol()()
#define OrderOpenPrice()()
#define OrderStopLoss()()
#define OrderTakeProfit()()
#define Print()()
#define Sleep()()*/
//+------------------------------------------------------------------+
//|                                                          wts.mq4 |
//|                        Copyright 2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include <stderror.mqh>
#include <stdlib.mqh>

#property copyright "Copyright 2013, MetaQuotes Software Corp."
#property link      "http://www.metaquotes.net"

#define ST_UNDEF    0
#define ST_CONG     1
#define ST_UP       2
#define ST_DOWN     3
//#define ST_NORISK   4
#define MIN_SWAP    -5.0

#define MAX_PERIOD      5
#define MIN_RISK_GAIN   5

#define TOTAL_BARS  500

int zzMagicNumber = 8888;

static double zzBottom[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzBottomTime[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzBottomTimeFrame[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzTop[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzTopTime[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzTopTimeFrame[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzTopBottomCount[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzState[MAX_PERIOD] = {ST_UNDEF, ST_UNDEF, ST_UNDEF, ST_UNDEF, ST_UNDEF};
static double gDisperce9[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double gDisperce21[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzRisk[MAX_PERIOD] = {0.16, 0.08, 0.04, 0.02, 0.01, 0.005, 0.01, 0.01, 0.01};
static double zzPerna[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz33Start[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz50Start[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz60Start[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz33Stop[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz50Stop[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz60Stop[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz33Target[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz50Target[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zz60Target[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzRiskGain[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzStopLoss[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzTakeProfit[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzLots[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzStart[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzStop[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static bool zzTradeBlock[MAX_PERIOD] = {false, false, false, false, false, false, false, false};
static double zzTradeBlockTime[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double gOrderPriceToTarget[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static bool gOrderCount[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzLucro[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzPerda[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static int zzReturn[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double gOrderGain[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
static double zzPercProt[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};

//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+

int init()
{
    int i;

    for (i = 0; i < MAX_PERIOD; i++) {
        ObjectDelete("top_" + wtsaGetPeriodByIndex(i));
        ObjectDelete("bottom_" + wtsaGetPeriodByIndex(i));
    }

    ObjectCreate("label_state1", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_state1", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_state1", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_state1", OBJPROP_YDISTANCE, 15); // Y coordinate

    ObjectCreate("label_state2", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_state2", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_state2", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_state2", OBJPROP_YDISTANCE, 28); // Y coordinate

    ObjectCreate("label_state3", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_state3", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_state3", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_state3", OBJPROP_YDISTANCE, 41); // Y coordinate

    ObjectCreate("label_state4", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_state4", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_state4", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_state4", OBJPROP_YDISTANCE, 54); // Y coordinate

    ObjectCreate("label_state5", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_state5", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_state5", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_state5", OBJPROP_YDISTANCE, 67); // Y coordinate

    return (0);
}

/******************************************************************************/

int deinit()
{
    return (0);
}

/******************************************************************************/

int start()
{
    int i;
    string desc;

    wtsaCalcDisperce9();
    wtsaCalcDisperce21();
    wtsCalcZigzag("MN1", PERIOD_MN1, 0);
    wtsCalcZigzag("W1", PERIOD_W1, 1);
    wtsCalcZigzag("D1", PERIOD_D1, 2);
    wtsCalcZigzag("H4", PERIOD_H4, 3);
    wtsCalcZigzag("H4", PERIOD_H1, 4);

    for (i = 0; i < MAX_PERIOD; i++) {
        ObjectDelete("zz33Start_" + wtsaGetPeriodByIndex(i));
        ObjectDelete("zz33Stop_" + wtsaGetPeriodByIndex(i));
        ObjectDelete("zz50Start_" + wtsaGetPeriodByIndex(i));
        ObjectDelete("zz50Stop_" + wtsaGetPeriodByIndex(i));
        ObjectDelete("zz60Start_" + wtsaGetPeriodByIndex(i));
        ObjectDelete("zz60Stop_" + wtsaGetPeriodByIndex(i));
    }

    //for (i = 0; i <= getIndexByPeriod(Period()); i++) {
    for (i = 0; i < MAX_PERIOD; i++) {

        if (i > 0 && zzTop[i] == zzTop[i - 1]) {
            ObjectCreate("top_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zzTop[i]);
            ObjectSet("top_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Gold);
            ObjectSet("top_" + wtsaGetPeriodByIndex(i), OBJPROP_FONTSIZE, 7);
            ObjectSet("top_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);

            desc = ObjectDescription("top_" + wtsaGetPeriodByIndex(i - 1));
            ObjectSetText("top_" + wtsaGetPeriodByIndex(i), desc + "_" + getPeriodNameByIndex(i), 7, "Arial", White);

            ObjectSetText("top_" + wtsaGetPeriodByIndex(i - 1), "", 7, "Arial", White);
            ObjectDelete("top_" + wtsaGetPeriodByIndex(i - 1));
        } else {
            ObjectCreate("top_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zzTop[i]);
            ObjectSet("top_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Gold);
            ObjectSet("top_" + wtsaGetPeriodByIndex(i), OBJPROP_FONTSIZE, 7);
            ObjectSet("top_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("top_" + wtsaGetPeriodByIndex(i),
                "[" + DoubleToStr(zzTop[i], 4) + "] TOP_" + getPeriodNameByIndex(i),
                7, "Arial", White);
        }

        if (i > 0 && zzBottom[i] == zzBottom[i - 1]) {
            ObjectCreate("bottom_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zzBottom[i]);
            ObjectSet("bottom_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Gold);
            ObjectSet("bottom_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);

            desc = ObjectDescription("bottom_" + wtsaGetPeriodByIndex(i - 1));
            ObjectSetText("bottom_" + wtsaGetPeriodByIndex(i), desc + "_" + getPeriodNameByIndex(i), 7, "Arial", White);

            ObjectSetText("bottom_" + wtsaGetPeriodByIndex(i - 1), "", 7, "Arial", White);
            ObjectDelete("bottom_" + wtsaGetPeriodByIndex(i - 1));
        } else {
            ObjectCreate("bottom_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zzBottom[i]);
            ObjectSet("bottom_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Gold);
            ObjectSet("bottom_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("bottom_" + wtsaGetPeriodByIndex(i),
                "[" + DoubleToStr(zzBottom[i], 4) + "] BOTTOM_" + getPeriodNameByIndex(i),
                7, "Arial", White);
        }

        //33%
        if ((zz33Start[i] > 0) &&
            ((zzState[i] == ST_DOWN && Bid < zz33Start[i]) ||
            ((zzState[i] == ST_UP && Bid > zz33Start[i])))) {

            //Start
            ObjectCreate("zz33Start_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz33Start[i]);

            if (zzState[i] == ST_UP) {
                ObjectSet("zz33Start_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Green);
            } else {
                ObjectSet("zz33Start_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Red);
            }
            ObjectSet("zz33Start_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("zz33Start_" + wtsaGetPeriodByIndex(i),
                "[33%] Start " + getPeriodNameByIndex(i),
                7, "Arial", White);

            //Stop
            ObjectCreate("zz33Stop_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz33Stop[i]);

            if (zzState[i] == ST_UP) {
                ObjectSet("zz33Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Blue);
            } else {
                ObjectSet("zz33Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Blue);
            }
            ObjectSet("zz33Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("zz33Stop_" + wtsaGetPeriodByIndex(i),
                "[33%] Stop " + getPeriodNameByIndex(i),
                7, "Arial", White);
        } else {
            ObjectDelete("zz33Start_" + wtsaGetPeriodByIndex(i));
            ObjectDelete("zz33Stop_" + wtsaGetPeriodByIndex(i));
        }

        //50%
        if ((zz50Start[i] > 0) &&
            ((zzState[i] == ST_DOWN && Bid < zz50Start[i]) ||
            ((zzState[i] == ST_UP && Bid > zz50Start[i])))) {

            ObjectCreate("zz50Start_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz50Start[i]);

            if (zzState[i] == ST_UP) {
                ObjectSet("zz50Start_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Green);
            } else {
                ObjectSet("zz50Start_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Red);
            }
            ObjectSet("zz50Start_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("zz50Start_" + wtsaGetPeriodByIndex(i),
                "[50%] Start " + getPeriodNameByIndex(i),
                7, "Arial", White);

            //Stop
            ObjectCreate("zz50Stop_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz50Stop[i]);

            if (zzState[i] == ST_UP) {
                ObjectSet("zz50Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Blue);
            } else {
                ObjectSet("zz50Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Blue);
            }
            ObjectSet("zz50Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("zz50Stop_" + wtsaGetPeriodByIndex(i),
                "[50%] Stop " + getPeriodNameByIndex(i),
                7, "Arial", White);
        } else {
            ObjectDelete("zz50Start_" + wtsaGetPeriodByIndex(i));
            ObjectDelete("zz50Stop_" + wtsaGetPeriodByIndex(i));
        }

        //60%
        if ((zz60Start[i] > 0) &&
            ((zzState[i] == ST_DOWN && Bid < zz60Start[i]) ||
            ((zzState[i] == ST_UP && Bid > zz60Start[i])))) {

            ObjectCreate("zz60Start_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz60Start[i]);

            if (zzState[i] == ST_UP) {
                ObjectSet("zz60Start_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Green);
            } else {
                ObjectSet("zz60Start_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Red);
            }
            ObjectSet("zz60Start_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("zz60Start_" + wtsaGetPeriodByIndex(i),
                "[60%] Start " + getPeriodNameByIndex(i),
                7, "Arial", White);

            //Stop
            ObjectCreate("zz60Stop_" + wtsaGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz60Stop[i]);

            if (zzState[i] == ST_UP) {
                ObjectSet("zz60Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Blue);
            } else {
                ObjectSet("zz60Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_COLOR, Blue);
            }
            ObjectSet("zz60Stop_" + wtsaGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("zz60Stop_" + wtsaGetPeriodByIndex(i),
                "[60%] Stop " + getPeriodNameByIndex(i),
                7, "Arial", White);
        } else {
            ObjectDelete("zz60Start_" + wtsaGetPeriodByIndex(i));
            ObjectDelete("zz60Stop_" + wtsaGetPeriodByIndex(i));
        }
    }

    ObjectSetText("label_state1",
        "| H1 | " + getStateName(zzState[4]) + " | " +
        "" + DoubleToStr(zzLots[4], 2) + " | " +
        "" + DoubleToStr(zzStart[4], 5) + " | " +
        "" + DoubleToStr(zzRiskGain[4], 0) + " | " +
        "" + zzTradeBlock[4] + ":" + gOrderCount[4] + ":" + (!validDecision(4)) + " | " +
        "" + DoubleToStr(gOrderGain[4], 2) + " | " +
        "" + DoubleToStr(gOrderPriceToTarget[4]*100, 2) + " | ",
        7, "Arial", DarkOrange);

    ObjectSetText("label_state2",
        "| H4 | " + getStateName(zzState[3]) + " | " +
        "" + DoubleToStr(zzLots[3], 2) + " | " +
        "" + DoubleToStr(zzStart[3], 5) + " | " +
        "" + DoubleToStr(zzRiskGain[3], 0) + " | " +
        "" + zzTradeBlock[3] + ":" + gOrderCount[3] + ":" + (!validDecision(3)) + " | " +
        "" + DoubleToStr(gOrderGain[3], 2) + " | " +
        "" + DoubleToStr(gOrderPriceToTarget[3]*100, 2) + " | ",
        7, "Arial", DarkOrange);

    ObjectSetText("label_state3",
        "| D1 | " + getStateName(zzState[2]) + " | " +
        "" + DoubleToStr(zzLots[2], 2) + " | " +
        "" + DoubleToStr(zzStart[2], 5) + " | " +
        "" + DoubleToStr(zzRiskGain[2], 0) + " | " +
        "" + zzTradeBlock[2] + ":" + gOrderCount[2] + ":" + (!validDecision(2)) + " | " +
        "" + DoubleToStr(gOrderGain[2], 2) + " | " +
        "" + DoubleToStr(gOrderPriceToTarget[2]*100, 2) + " | ",
        7, "Arial", DarkOrange);

    ObjectSetText("label_state4",
        "| W1 | " + getStateName(zzState[1]) + " | " +
        "" + DoubleToStr(zzLots[1], 2) + " | " +
        "" + DoubleToStr(zzStart[1], 5) + " | " +
        "" + DoubleToStr(zzRiskGain[1], 0) + " | " +
        "" + zzTradeBlock[1] + ":" + gOrderCount[1] + ":" + (!validDecision(1)) + " | " +
        "" + DoubleToStr(gOrderGain[1], 2) + " | " +
        "" + DoubleToStr(gOrderPriceToTarget[1]*100, 2) + " | ",
        7, "Arial", DarkOrange);

    ObjectSetText("label_state5",
        "| MN | " + getStateName(zzState[0]) + " | " +
        "" + DoubleToStr(zzLots[0], 2) + " | " +
        "" + DoubleToStr(zzStart[0], 5) + " | " +
        "" + DoubleToStr(zzRiskGain[0], 0) + " | " +
        "" + zzTradeBlock[0] + ":" + gOrderCount[0] + ":" + (!validDecision(0)) + " | " +
        "" + DoubleToStr(gOrderGain[0], 2) + " | " +
        "" + DoubleToStr(gOrderPriceToTarget[0]*100, 2) + " | ",
        7, "Arial", DarkOrange);

    checkOrders();
    checkDecision();

    return (0);
}
//+------------------------------------------------------------------+

bool is_high_clande(int timeframe, int shift)
{
    bool ret = false;
    double cdlOpen;
    double cdlClose;

    cdlOpen = iOpen(Symbol(), timeframe, shift);
    cdlClose = iClose(Symbol(), timeframe, shift);

    if (cdlClose > cdlOpen) {
        ret = true;
    }

    return (ret);
}

double getRisk(int index)
{
    double risk = 0;

    switch (zzState[index]) {
    case ST_UNDEF: risk = 0;
        break;

    case ST_CONG: risk = zzRisk[index];
        break;

    case ST_UP: risk = zzRisk[index];
        break;

    case ST_DOWN: risk = zzRisk[index];
        break;
    }

    if (AccountBalance() < 50) {
        risk *= 2;
    }

    return (risk);
}

void wtsCalcZigzag(string strPeriod, int period, int index)
{
    int i;
    double zz = 0;
    int zzTotal = 0;
    int spread = MarketInfo(Symbol(), MODE_SPREAD);
    int shift[MAX_PERIOD] = {0, 0, 0, 0, 0, 0, 0, 0};
    double zzBuffer[200];
    double zzBufferClose[200];
    double zzBufferTime[200];
    double zzShift[200];
    double pernaPerc = 0;
    double startPerc = 0;
    double targetPerc = 0;
    double swapLong = MarketInfo(Symbol(), MODE_SWAPLONG);
    double swapShort = MarketInfo(Symbol(), MODE_SWAPSHORT);

    for (i = TOTAL_BARS; i != 0; i--) {
        zz = iCustom(Symbol(), period, "ZigZag", 0, i);
        if (zz != 0) {
            zzBuffer[zzTotal] = zz;
            zzBufferTime[zzTotal] = iTime(Symbol(), period, i);
            zzBufferClose[zzTotal] = iClose(Symbol(), period, i);
            zzShift[zzTotal] = i;
            zzTotal++;
        }
    }

    if (zzBuffer[0] > zzBuffer[1]) {
        zzTop[index] = zzBuffer[0];
        zzBottom[index] = zzBuffer[1];
        zzTopTime[index] = zzBufferTime[0];
        zzBottomTime[index] = zzBufferTime[1];
        zzState[index] = ST_DOWN;
        shift[index] = zzShift[1];
    } else {
        zzTop[index] = zzBuffer[1];
        zzBottom[index] = zzBuffer[0];
        zzTopTime[index] = zzBufferTime[1];
        zzBottomTime[index] = zzBufferTime[0];
        zzState[index] = ST_UP;
        shift[index] = zzShift[0];
    }

    zzTopBottomCount[index] = 0;

    for (i = 2; i < zzTotal; i++) {
        zzTopBottomCount[index]++;

        if (zzBuffer[i] > zzTop[index] && zzBufferClose[i] > zzTop[index]) {
            zzTop[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i];
            zzBottom[index] = zzBuffer[i - 1];
            zzBottomTime[index] = zzBufferTime[i - 1];
            zzState[index] = ST_UP;
            zzTopBottomCount[index] = 0;
            shift[index] = zzShift[i];
        } else if (zzBuffer[i] > zzTop[index]) {
            zzTop[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i];
            zzBottom[index] = zzBuffer[i - 1];
            zzBottomTime[index] = zzBufferTime[i - 1];
            zzState[index] = ST_CONG;
            zzTopBottomCount[index] = 0;
            shift[index] = zzShift[i];
        }

        if (zzBuffer[i] < zzBottom[index] && zzBufferClose[i] < zzBottom[index]) {
            zzTop[index] = zzBuffer[i - 1];
            zzBottom[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i - 1];
            zzBottomTime[index] = zzBufferTime[i];
            zzState[index] = ST_DOWN;
            zzTopBottomCount[index] = 0;
            shift[index] = zzShift[i];
        } else if (zzBuffer[i] < zzBottom[index]) {
            zzTop[index] = zzBuffer[i - 1];
            zzTopTime[index] = zzBufferTime[i - 1];
            zzBottom[index] = zzBuffer[i];
            zzBottomTime[index] = zzBufferTime[i];
            zzState[index] = ST_CONG;
            zzTopBottomCount[index] = 0;
            shift[index] = zzShift[i];
        }
    }

    if (zzTopBottomCount[index] > 1) {
        zzState[index] = ST_UNDEF;
    }

    zzPerna[index] = (zzTop[index] - zzBottom[index]) / Point;

    //33%    
    if (zzState[index] == ST_UP) {
        pernaPerc = zzTop[index] / zzBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz33Start[index] = (zzTop[index] - (zzPerna[index] * 0.33 * Point)) * (1 - startPerc);
        zz33Stop[index] = zz33Start[index] - (gDisperce21[index] * Point);
        zz33Target[index] = zz33Stop[index] + (zzPerna[index] * Point * (1 - targetPerc));
    } else if (zzState[index] == ST_DOWN) {
        pernaPerc = 1 - zzBottom[index] / zzTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz33Start[index] = (zzBottom[index] + (zzPerna[index] * 0.33 * Point)) * (1 + startPerc) + spread * Point;
        zz33Stop[index] = zz33Start[index] + (gDisperce21[index] * Point);
        zz33Target[index] = zz33Stop[index] - (zzPerna[index] * Point * (1 - targetPerc));
    } else {
        zz33Start[index] = 0;
        zz33Stop[index] = 0;
    }

    //50%
    if (zzState[index] == ST_UP) {
        pernaPerc = zzTop[index] / zzBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz50Start[index] = (zzTop[index] - (zzPerna[index] * 0.5 * Point)) * (1 - startPerc);
        zz50Stop[index] = zz50Start[index] - (gDisperce21[index] * Point);
        zz50Target[index] = zz50Stop[index] + (zzPerna[index] * Point * (1 - targetPerc));
    } else if (zzState[index] == ST_DOWN) {
        pernaPerc = 1 - zzBottom[index] / zzTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz50Start[index] = (zzBottom[index] + (zzPerna[index] * 0.50 * Point)) * (1 + startPerc) + spread * Point;
        zz50Stop[index] = zz50Start[index] + (gDisperce21[index] * Point);
        zz50Target[index] = zz50Stop[index] - (zzPerna[index] * Point * (1 - targetPerc));
    } else {
        zz50Start[index] = 0;
        zz50Start[index] = 0;
    }

    //60%
    if (zzState[index] == ST_UP) {
        pernaPerc = zzTop[index] / zzBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz60Start[index] = (zzTop[index] - (zzPerna[index] * 0.6 * Point)) * (1 - startPerc);
        zz60Stop[index] = zz60Start[index] - (gDisperce21[index] * Point);
        zz60Target[index] = zz60Stop[index] + (zzPerna[index] * Point * (1 - targetPerc));
    } else if (zzState[index] == ST_DOWN) {
        pernaPerc = 1 - zzBottom[index] / zzTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz60Start[index] = (zzBottom[index] + (zzPerna[index] * 0.60 * Point)) * (1 + startPerc) + spread * Point;
        zz60Stop[index] = zz60Start[index] + (gDisperce21[index] * Point);
        zz60Target[index] = zz60Stop[index] - (zzPerna[index] * Point * (1 - targetPerc));
    } else {
        zz60Start[index] = 0;
        zz60Start[index] = 0;
    }

    for (i = shift[index] - 1; i != 0; i--) {
        if ((zzState[index] == ST_UP && (iLow(Symbol(), period, i) < zz60Start[index] || Ask < zz60Start[index])) ||
            (zzState[index] == ST_DOWN && ((iHigh(Symbol(), period, i) + spread * Point) > zz60Start[index]) || Bid > zz60Start[index]) && gOrderCount[index] == false) {
            zz33Start[index] = 0;
            zz50Start[index] = 0;
            zz60Start[index] = 0;
            zz33Stop[index] = 0;
            zz50Stop[index] = 0;
            zz60Stop[index] = 0;
            zz33Target[index] = 0;
            zz50Target[index] = 0;
            zz60Target[index] = 0;
            zzState[index] = ST_UNDEF;
        } else if ((zzState[index] == ST_UP && (iLow(Symbol(), period, i) < zz50Start[index] || Ask < zz50Start[index])) ||
            (zzState[index] == ST_DOWN && ((iHigh(Symbol(), period, i) + spread * Point) > zz50Start[index] || Bid > zz50Start[index]))) {
            zz33Start[index] = 0;
            zz50Start[index] = 0;
            zz33Stop[index] = 0;
            zz50Stop[index] = 0;
            zz33Target[index] = 0;
            zz50Target[index] = 0;
        } else if ((zzState[index] == ST_UP && (iLow(Symbol(), period, i) < zz33Start[index] || Ask < zz33Start[index])) ||
            (zzState[index] == ST_DOWN && ((iHigh(Symbol(), period, i) + spread * Point) > zz33Start[index]) || Bid > zz33Start[index])) {
            zz33Start[index] = 0;
            zz33Stop[index] = 0;
            zz33Target[index] = 0;
        }
    }

    zz33Start[index] = NormalizeDouble(zz33Start[index], 4);
    zz50Start[index] = NormalizeDouble(zz50Start[index], 4);
    zz60Start[index] = NormalizeDouble(zz60Start[index], 4);

    zz33Stop[index] = NormalizeDouble(zz33Stop[index], 4);
    zz50Stop[index] = NormalizeDouble(zz50Stop[index], 4);
    zz60Stop[index] = NormalizeDouble(zz60Stop[index], 4);

    zz33Target[index] = NormalizeDouble(zz33Target[index], 4);
    zz50Target[index] = NormalizeDouble(zz50Target[index], 4);
    zz60Target[index] = NormalizeDouble(zz60Target[index], 4);

    if (zz33Start[index] != 0 && zz33Stop[index] != 0) {
        zzStart[index] = zz33Start[index];
        zzStop[index] = zz33Stop[index];
        zzTakeProfit[index] = zz33Target[index];
        zzReturn[index] = 33;

    } else if (zz50Start[index] != 0 && zz50Stop[index] != 0) {

        zzStart[index] = zz50Start[index];
        zzStop[index] = zz50Stop[index];
        zzTakeProfit[index] = zz50Target[index];
        zzReturn[index] = 50;

    } else if (zz60Start[index] != 0 && zz60Stop[index] != 0) {

        zzStart[index] = zz60Start[index];
        zzStop[index] = zz60Stop[index];
        zzTakeProfit[index] = zz60Target[index];
        zzReturn[index] = 60;

    } else {
        zzStart[index] = 0;
        zzStop[index] = 0;
        zzRiskGain[index] = 0;
        zzStopLoss[index] = 0;
        zzLots[index] = 0;
        zzLucro[index] = 0;
        zzPerda[index] = 0;
    }

    if (zzStart[index] != 0) {
        if (zzState[index] == ST_UP) {
            zzStopLoss[index] = (zzStart[index] - zzStop[index]) / Point;
            zzRiskGain[index] = (zzTakeProfit[index] - (zzStart[index] + spread * Point)) / ((zzStart[index] + spread * Point) - zzStop[index]);
        } else {
            zzStopLoss[index] = (zzStop[index] - zzStart[index]) / Point;
            zzRiskGain[index] = ((zzStart[index] - spread * Point) - zzTakeProfit[index]) / (zzStop[index] - (zzStart[index] - spread * Point));
        }

        zzLots[index] = NormalizeDouble((AccountBalance() * getRisk(index)) / (zzStopLoss[index] + spread), 2);

        if (zzState[index] == ST_UP) {
            zzLucro[index] = zzLots[index] * ((zzTakeProfit[index] - zzStart[index]) / Point - spread);
            zzPerda[index] = zzLots[index] * ((zzStop[index] - zzStart[index]) / Point - spread);
        } else {
            zzLucro[index] = zzLots[index] * ((zzStart[index] - zzTakeProfit[index]) / Point - spread);
            zzPerda[index] = zzLots[index] * ((zzStart[index] - zzStop[index]) / Point - spread);
        }
    }

    zzRiskGain[index] = NormalizeDouble(zzRiskGain[index], 0);

    if (((zzRiskGain[index] < MIN_RISK_GAIN) && zzState[index] == ST_DOWN) ||
        ((zzRiskGain[index] < MIN_RISK_GAIN) && zzState[index] == ST_UP) ||
        (gOrderCount[index] == true) ||
        (zzState[index] == ST_UP && zzState[index - 1] == ST_DOWN && gOrderCount[index - 1] == true) ||
        (zzState[index] == ST_DOWN && zzState[index - 1] == ST_UP && gOrderCount[index - 1] == true) ||
        (zzLots[index] == 0)) {
        zz33Start[index] = 0;
        zz50Start[index] = 0;
        zz60Start[index] = 0;
        zz33Stop[index] = 0;
        zz50Stop[index] = 0;
        zz60Stop[index] = 0;
        zz33Target[index] = 0;
        zz50Target[index] = 0;
        zz60Target[index] = 0;
        //zzStart[index] = 0;
        //zzStop[index] = 0;
        //zzState[index] = ST_NORISK;
    }
}

int wtsaGetPeriodByIndex(int index)
{
    int ret = 0;

    switch (index) {
    case 0: ret = PERIOD_MN1;
        break;
    case 1: ret = PERIOD_W1;
        break;
    case 2: ret = PERIOD_D1;
        break;
    case 3: ret = PERIOD_H4;
        break;
    case 4: ret = PERIOD_H1;
        break;
    case 5: ret = PERIOD_M5;
        break;
    case 6: ret = PERIOD_M15;
        break;
    case 7: ret = PERIOD_M5;
        break;
    case 8: ret = PERIOD_M1;
        break;
    }

    return (ret);
}

string getPeriodNameByIndex(int index)
{
    string ret = 0;

    switch (index) {
    case 0: ret = "MN1";
        break;
    case 1: ret = "W1";
        break;
    case 2: ret = "D1";
        break;
    case 3: ret = "H4";
        break;
    case 4: ret = "H1";
        break;
    case 5: ret = "M5";
        break;
    case 6: ret = "M15";
        break;
    case 7: ret = "M5";
        break;
    case 8: ret = "M1";
        break;
    }

    return (ret);
}

string getStateName(int state)
{
    string name = "";

    switch (state) {
    case ST_UNDEF: name = "--";
        break;
    case ST_CONG: name = "cg";
        break;
    case ST_UP: name = "up";
        break;
    case ST_DOWN: name = "dw";
        break;
        //case ST_NORISK: name = "no"; break;
    }

    return (name);
}

int getIndexByPeriod(int period)
{
    int ret = 0;

    switch (period) {
    case PERIOD_MN1: ret = 0;
        break;
    case PERIOD_W1: ret = 1;
        break;
    case PERIOD_D1: ret = 2;
        break;
    case PERIOD_H4: ret = 3;
        break;
    case PERIOD_H1: ret = 4;
        break;
    case PERIOD_M30: ret = 5;
        break;
    case PERIOD_M15: ret = 5;
        break;
    case PERIOD_M5: ret = 5;
        break;
    case PERIOD_M1: ret = 8;
        break;
    }

    return (ret);
}

/******************************************************************************/

void wtsaCalcDisperce9()
{
    int i, j;
    int total = 9;
    double disperce = 0;
    double low;
    double high;

    for (j = 0; j < MAX_PERIOD; j++) {
        disperce = 0;

        for (i = total; i != 0; i--) {
            low = iLow(Symbol(), wtsaGetPeriodByIndex(j), i);
            high = iHigh(Symbol(), wtsaGetPeriodByIndex(j), i);

            disperce += high - low;
        }

        disperce /= total;
        disperce = NormalizeDouble(disperce / Point, 0);

        gDisperce9[j] = disperce;
    }
}

/******************************************************************************/

void wtsaCalcDisperce21()
{
    int i, j;
    int total = 21;
    double disperce = 0;
    double low;
    double high;

    for (j = 0; j < MAX_PERIOD; j++) {
        disperce = 0;

        for (i = total; i != 0; i--) {
            low = iLow(Symbol(), wtsaGetPeriodByIndex(j), i);
            high = iHigh(Symbol(), wtsaGetPeriodByIndex(j), i);

            disperce += high - low;
        }

        disperce /= total;
        disperce = NormalizeDouble(disperce / Point, 0);

        gDisperce21[j] = disperce;
    }
}

/******************************************************************************/

bool validDecision(int index)
{
    bool decision = false;

    RefreshRates();

    if (zzState[index] == ST_UP) {
        if (zzLots[index] > 0 &&
            zzTradeBlock[index] == false &&
            //NormalizeDouble(Bid, 4) == zzStart[index] &&
            zzRiskGain[index] >= MIN_RISK_GAIN &&
            gOrderCount[index] == false &&
            !(zzState[index - 1] == ST_DOWN && gOrderCount[index - 1] == true && index > 0)) {

            decision = true;
        }
    } else if (zzState[index] == ST_DOWN) {
        if (zzLots[index] > 0 &&
            zzTradeBlock[index] == false &&
            //NormalizeDouble(Ask, 4) == zzStart[index] &&
            zzRiskGain[index] >= MIN_RISK_GAIN &&
            gOrderCount[index] == false &&
            !(zzState[index - 1] == ST_UP && gOrderCount[index - 1] == true && index > 0)) {

            decision = true;
        }
    }

    return (decision);
}

void checkDecision()
{
    int i;
    int err;
    int ticket = -1;
    double swapLong = MarketInfo(Symbol(), MODE_SWAPLONG);
    double swapShort = MarketInfo(Symbol(), MODE_SWAPSHORT);

    for (i = 0; i < MAX_PERIOD; i++) {

        if (zzTradeBlockTime[i] != iTime(Symbol(), wtsaGetPeriodByIndex(i), 0)) {
            zzTradeBlock[i] = false;
        }

        RefreshRates();
        if (validDecision(i) && NormalizeDouble(Bid, 4) == zzStart[i]) {
            RefreshRates();
            ticket = OrderSend(Symbol(), OP_BUY, zzLots[i], Ask, 3,
                Bid - (zzStopLoss[i] * Point), zzTakeProfit[i],
                "[" + wtsaGetPeriodByIndex(i) + "][" + zzReturn[i] + "][" + DoubleToStr(zzRiskGain[i], 0) + "][" + DoubleToStr(AccountBalance(), 0) + "]",
                zzMagicNumber, 0, Blue);
            if (ticket < 0) {
                err = GetLastError();
                Print("error(", err, "): ", ErrorDescription(err));
                Sleep(1000);
            } else {
                zzTradeBlock[i] = true;
                zzTradeBlockTime[i] = iTime(Symbol(), wtsaGetPeriodByIndex(i), 0);
            }
        } else if (validDecision(i) && NormalizeDouble(Ask, 4) == zzStart[i]) {
            RefreshRates();
            ticket = OrderSend(Symbol(), OP_SELL, zzLots[i], Bid, 3,
                Ask + (zzStopLoss[i] * Point), zzTakeProfit[i],
                "[" + wtsaGetPeriodByIndex(i) + "][" + zzReturn[i] + "][" + DoubleToStr(zzRiskGain[i], 0) + "][" + DoubleToStr(AccountBalance(), 0) + "]",
                zzMagicNumber, 0, Blue);
            if (ticket < 0) {
                err = GetLastError();
                Print("error(", err, "): ", ErrorDescription(err));
                Sleep(1000);
            } else {
                zzTradeBlock[i] = true;
                zzTradeBlockTime[i] = iTime(Symbol(), wtsaGetPeriodByIndex(i), 0);
            }
        }
    }
}

void checkOrders()
{
    int index;
    int ticket;
    int total = OrdersTotal();
    int period;
    int str_open = 0;
    int str_close = 0;
    double leg = 0;
    double perc_gain = 0;
    double orderOP;
    double orderSL;
    double takeProfit;
    double lots;
    double recuo = 0;
    double risco_ganho = 0;
    double patrimonio = 0;
    double newStop = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);

    gOrderCount[0] = false;
    gOrderCount[1] = false;
    gOrderCount[2] = false;
    gOrderCount[3] = false;
    gOrderCount[4] = false;
    gOrderCount[5] = false;

    zzPercProt[0] = 0;
    zzPercProt[1] = 0;
    zzPercProt[2] = 0;
    zzPercProt[3] = 0;
    zzPercProt[4] = 0;

    gOrderGain[0] = 0;
    gOrderGain[1] = 0;
    gOrderGain[2] = 0;
    gOrderGain[3] = 0;
    gOrderGain[4] = 0;

    ObjectDelete("zzProt_" + wtsaGetPeriodByIndex(0) + "");
    ObjectDelete("zzProt_" + wtsaGetPeriodByIndex(1) + "");
    ObjectDelete("zzProt_" + wtsaGetPeriodByIndex(2) + "");
    ObjectDelete("zzProt_" + wtsaGetPeriodByIndex(3) + "");
    ObjectDelete("zzProt_" + wtsaGetPeriodByIndex(4) + "");

    //----
    for (int i = 0; i < total; i++) {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
        // check for symbol & magic number
        //if(OrderSymbol() == Symbol() && OrderMagicNumber() == zzMagicNumber) {
        if (OrderSymbol() == Symbol()) {

            RefreshRates();

            str_open = StringFind(OrderComment(), "[", 0);

            if (str_open < 0) {
                period = StrToInteger(OrderComment());
            } else {
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                period = StrToInteger(StringSubstr(OrderComment(), str_open, str_close - str_open));

                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                recuo = StrToInteger(StringSubstr(OrderComment(), str_open, str_close - str_open));

                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                risco_ganho = StrToInteger(StringSubstr(OrderComment(), str_open, str_close - str_open));

                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                patrimonio = StrToInteger(StringSubstr(OrderComment(), str_open, str_close - str_open));
            }

            ticket = OrderTicket();
            lots = OrderLots();
            orderOP = OrderOpenPrice();
            orderSL = OrderStopLoss();
            takeProfit = OrderTakeProfit();
            index = getIndexByPeriod(period);

            if (AccountBalance() < patrimonio) {
                perc_gain = (AccountProfit() - (patrimonio - AccountBalance())) / patrimonio;
            } else {
                perc_gain = AccountProfit() / AccountBalance();
            }

            // long position is opened
            if (OrderType() == OP_BUY) {

                gOrderGain[index] = ((takeProfit - orderOP)) * lots / Point;

                gOrderCount[index] = true;
                gOrderPriceToTarget[index] = (Bid - orderOP) / (takeProfit - orderOP);

                newStop = NormalizeDouble((iLow(Symbol(), period, 1) - (gDisperce21[index] / 2) * Point), Digits - 1);

                leg = Bid - orderOP;

                if (orderOP > orderSL) {
                    ObjectCreate("zzProt_" + period, OBJ_HLINE, 0, 0, NormalizeDouble(orderOP + (orderOP - orderSL), Digits - 1));
                    ObjectSet("zzProt_" + period, OBJPROP_COLOR, Pink);
                    ObjectSet("zzProt_" + period, OBJPROP_STYLE, 2);
                }

                if ((orderSL < orderOP) && ((Bid - orderOP) > (orderOP - orderSL))) {
                    if (!OrderModify(ticket, orderOP, orderOP, takeProfit, 0, Blue)) {
                        Sleep(10000);
                    }
                } else if (orderSL >= orderOP && newStop > orderSL && wtsaIsHighUpCandle(period) && gDisperce9[index] > gDisperce21[index]) {
                    if (!OrderModify(ticket, orderOP, newStop, takeProfit, 0, Yellow)) {
                        Sleep(10000);
                    }
                }
            } else if (OrderType() == OP_SELL) {

                gOrderGain[index] = ((orderOP - takeProfit)) * lots / Point;

                gOrderCount[index] = true;
                gOrderPriceToTarget[index] = (orderOP - Ask) / (orderOP - takeProfit);

                newStop = NormalizeDouble((iHigh(Symbol(), period, 1) + (gDisperce21[index] / 2 + spread) * Point), Digits - 1);

                leg = orderOP - Ask;

                if (orderOP < orderSL) {
                    ObjectCreate("zzProt_" + period, OBJ_HLINE, 0, 0, NormalizeDouble(orderOP - (orderSL - orderOP), Digits - 1));
                    ObjectSet("zzProt_" + period, OBJPROP_COLOR, Pink);
                    ObjectSet("zzProt_" + period, OBJPROP_STYLE, 2);
                }
                
                if ((orderSL > orderOP) && ((orderOP - Ask) > (orderSL - orderOP))) {
                    if (!OrderModify(ticket, orderOP, orderOP, takeProfit, 0, Blue)) {
                        Sleep(1000);
                    }
                } else if (orderSL <= orderOP && newStop < orderSL && wtsaIsLowDownCandle(period) && gDisperce9[index] > gDisperce21[index]) {
                    if (!OrderModify(ticket, orderOP, newStop, takeProfit, 0, Yellow)) {
                        Sleep(1000);
                    }
                }
            }
        }
    }
}

/******************************************************************************/

bool wtsaIsHighUpCandle(int period)
{
    bool cond = false;
    double close = 0;
    double open = 0;
    double low = 0;
    double prev_high = 0;
    double prev_low = 0;

    close = iClose(Symbol(), period, 1);
    open = iOpen(Symbol(), period, 1);
    low = iLow(Symbol(), period, 1);

    prev_high = iHigh(Symbol(), period, 2);
    prev_low = iLow(Symbol(), period, 2);

    if (close > open && close > prev_high && low > prev_low) {
        cond = true;
    }

    return (cond);
}

/******************************************************************************/

bool wtsaIsLowDownCandle(int period)
{
    bool cond = false;
    double close = 0;
    double high = 0;
    double open = 0;
    double prevLow = 0;
    double prevHigh = 0;

    close = iClose(Symbol(), period, 1);
    open = iOpen(Symbol(), period, 1);
    high = iHigh(Symbol(), period, 1);

    prevLow = iLow(Symbol(), period, 2);
    prevHigh = iHigh(Symbol(), period, 2);

    if (close < open && close < prevLow && high < prevHigh) {
        cond = true;
    }

    return (cond);
}

