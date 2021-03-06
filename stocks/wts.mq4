//+------------------------------------------------------------------+
//|                                                          wts.mq4 |
//|                        Copyright 2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
//gnome-shell&
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

#define MAX_PERIOD      3

#define MIN_RISK_GAIN   99.0

#define TOTAL_BARS  500

#define WTS_PROTECTION_VALUE        "wts_protectionValue_"
#define WTS_PROTECTION_VALUE_PREV   "wts_protectionValuePrev_"

int zzMagicNumber = 8888;

static double m_candleBottom[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleBottomTime[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleBottomPrev[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleBottomShift[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleBottomShiftPrev[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleBottomTimePrev[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleBottomTimeFrame[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleFibo33[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleFibo50[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleFibo60[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleHighestRetration[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleLastBarShift[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleLowestRetration[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleHighestAfterLowest[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleLowestAfterHighest[MAX_PERIOD] = {0,0,0,0,0,0,0,0};

static double m_candleMA[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleMA21[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleTop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleTopTime[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleTopPrev[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleTopTimePrev[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleTopShift[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_candleTopShiftPrev[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzTopTimeFrame[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_candleTopBottomCount[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_agressiveState[MAX_PERIOD] = {ST_UNDEF,ST_UNDEF,ST_UNDEF,ST_UNDEF,ST_UNDEF};
static double m_candleShadowMed[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzRisk[MAX_PERIOD] = {0.01,0.05,0.10,0.04,0.02,0.010,0.005,0.01,0.01};
static double zzRiskAgressive[MAX_PERIOD] = {0.01,0.08,0.04,0.02,0.01,0.005,0.0025,0.01,0.01};
static double m_candleLeg[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz33Start[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz50Start[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz60Start[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz33Stop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz50Stop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz60Stop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz33Target[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz50Target[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zz60Target[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzRiskGain[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzStopLoss[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzTakeProfit[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzLots[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzStart[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzStop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderPercent[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzLucro[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double zzPerda[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int zzReturn[MAX_PERIOD] = {0,0,0,0,0,0,0,0};

static double m_aux[MAX_PERIOD] = {0,0,0,0,0,0,0,0};

static double m_normalTradeStartMultiply[MAX_PERIOD] = {1,1,1,0,0,0,0,0};
static double m_normalTradeRiskGainMin[MAX_PERIOD] = {5,7.5,10,0,0,0,0,0};

static int    m_normalState[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_normalTradeLots[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_normalTradeStart[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_normalTradeStop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_normalTradeRisk[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_normalTradeRiskGain[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_normalTradeTakeProfit[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_normalTradeType[MAX_PERIOD] = {0,0,0,0,0,0,0,0};

static bool   m_orderActive[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderLots[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_orderOpenBarShift[MAX_PERIOD] = {-1,-1,-1,-1,-1,-1,-1,-1};
static double m_orderOpenPrice[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderOpenTime[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderPercentToProtection[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderPercentToProtectionPrev[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderProtectionValue[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderProtectionValuePrev[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_orderRiskGain[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderStopLoss[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderStopLossGain[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderTakeProfit[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static double m_orderTakeProfitGain[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
static int    m_orderTicket[MAX_PERIOD] = {-1,-1,-1,-1,-1,-1,-1,-1};

//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
{
    int i;
    
    ObjectCreate("label_start", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_start", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_start", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_start", OBJPROP_YDISTANCE, 15);// Y coordinate

//    for (i = 0; i < MAX_PERIOD; i++) {
    for (i = 0; i < 3; i++) {
        ObjectDelete("top_"+wtsGetPeriodByIndex(i));
        ObjectDelete("bottom_"+wtsGetPeriodByIndex(i));    

        ObjectCreate("label_state"+i, OBJ_LABEL, 0, 0, 0);// Creating obj.
        ObjectSet("label_state"+i, OBJPROP_CORNER, 0);    // Reference corner
        ObjectSet("label_state"+i, OBJPROP_XDISTANCE, 10);// X coordinate
        ObjectSet("label_state"+i, OBJPROP_YDISTANCE, 28 + (13*i));// Y coordinate
    }
    
    return(0);
}
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
int deinit()
  {
//----
   //ObjectsDeleteAll();
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
int start()
{
    int i;
    int j;
    int retration[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
    int afterRetration[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
    bool aux;
    string desc;
    string comment = "";
    double rg;
    double gain = 0;
    double loss = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    double swapLong = MarketInfo(Symbol(), MODE_SWAPLONG);
    double swapShort = MarketInfo(Symbol(), MODE_SWAPSHORT);
    
    calcMedShadow();
    wtsCalcZigzag("H1", PERIOD_H1, 0);
    wtsCalcZigzag("H4", PERIOD_H4, 1);
    wtsCalcZigzag("D1", PERIOD_D1, 2);
    //wtsCalcZigzag("H4", PERIOD_H4, 3);
    //wtsCalcZigzag("H1", PERIOD_H1, 4);
    //wtsCalcZigzag("M15", PERIOD_M15, 5);
    //wtsCalcZigzag("M5", PERIOD_M5, 6);
    
    for (i = 0; i < MAX_PERIOD; i++) {
        ObjectDelete("zz33Start_"+wtsGetPeriodByIndex(i));
        ObjectDelete("zz33Stop_"+wtsGetPeriodByIndex(i));
        ObjectDelete("zz50Start_"+wtsGetPeriodByIndex(i));
        ObjectDelete("zz50Stop_"+wtsGetPeriodByIndex(i));
        ObjectDelete("zz60Start_"+wtsGetPeriodByIndex(i));
        ObjectDelete("zz60Stop_"+wtsGetPeriodByIndex(i));
    }
    
    //for (i = 0; i <= getIndexByPeriod(Period()); i++) {
    for (i = 0; i < MAX_PERIOD; i++) {

        if (i > 0 && m_candleTop[i] == m_candleTop[i-1]) {
            //ObjectCreate("top_"+wtsGetPeriodByIndex(i), OBJ_HLINE, 0, 0, m_candleTop[i]);
            ObjectCreate("top_"+wtsGetPeriodByIndex(i), OBJ_TREND, 0, 0, m_candleTop[i]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Gold); 
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_FONTSIZE, 7); 
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_TIME1, m_candleTopTime[i]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_PRICE1, m_candleTop[i]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_PRICE2, m_candleTop[i]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);

            //desc = ObjectDescription("top_"+wtsGetPeriodByIndex(i-1));
            //ObjectSetText("top_"+wtsGetPeriodByIndex(i), desc+"_"+wtsGetPeriodNameByIndex(i), 7, "Arial", White);
            //ObjectSetText("top_"+wtsGetPeriodByIndex(i-1), "", 7, "Arial", White);
            ObjectDelete("top_"+wtsGetPeriodByIndex(i-1));
        } else { 
            ObjectCreate("top_"+wtsGetPeriodByIndex(i), OBJ_TREND, 0, 0, m_candleTop[i]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Gold);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_FONTSIZE, 7); 
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_TIME1, m_candleTopTime[i]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_PRICE1, m_candleTop[i]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_PRICE2, m_candleTop[i]);
            ObjectSet("top_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            //ObjectSetText("top_"+wtsGetPeriodByIndex(i), "["+DoubleToStr(m_candleTop[i], 4)+"] TOP_"+wtsGetPeriodNameByIndex(i), 7, "Arial", White);
        }
        
        if (i > 0 && m_candleBottom[i] == m_candleBottom[i-1]) {
            ObjectCreate("bottom_"+wtsGetPeriodByIndex(i), OBJ_TREND, 0, 0, m_candleBottom[i]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Gold); 
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_TIME1, m_candleBottomTime[i]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_PRICE1, m_candleBottom[i]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_PRICE2, m_candleBottom[i]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);

  //          desc = ObjectDescription("bottom_"+wtsGetPeriodByIndex(i-1));
//            ObjectSetText("bottom_"+wtsGetPeriodByIndex(i), desc+"_"+wtsGetPeriodNameByIndex(i), 7, "Arial", White);

            //ObjectSetText("bottom_"+wtsGetPeriodByIndex(i-1), "", 7, "Arial", White);
            ObjectDelete("bottom_"+wtsGetPeriodByIndex(i-1));
        } else { 
            ObjectCreate("bottom_"+wtsGetPeriodByIndex(i), OBJ_TREND, 0, 0, m_candleBottom[i]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Gold); 
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_TIME1, m_candleBottomTime[i]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_PRICE1, m_candleBottom[i]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_PRICE2, m_candleBottom[i]);
            ObjectSet("bottom_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);
            //ObjectSetText("bottom_"+wtsGetPeriodByIndex(i), 
              //            "["+DoubleToStr(m_candleBottom[i], 4)+"] BOTTOM_"+wtsGetPeriodNameByIndex(i),
                //          7, "Arial", White);
        }

        loss = zzStopLoss[i] * zzLots[i];

        if (m_agressiveState[i] == ST_UP) {
        
            gain = (zzTakeProfit[i] - zzStart[i]) * zzLots[i] / Point;
        
            comment = "WTSa_" + wtsGetPeriodNameByIndex(i) +"_"+zzReturn[i]+": " +
                      "" + DoubleToStr(zzLots[i], 2) +" | "+
                      "-$" + DoubleToStr(loss, 2) +" | "+
                      "$" + DoubleToStr(gain, 2) +" | "+
                      "" + DoubleToStr(zzRiskGain[i], 1) +" | "+
                      "" + validDecision(i) +" | ";

        } else if (m_agressiveState[i] == ST_DOWN) {

            gain = (zzStart[i] - zzTakeProfit[i]) * zzLots[i] / Point;

            comment = "WTSa_" + wtsGetPeriodNameByIndex(i) +"_"+zzReturn[i]+": " +
                      "" + DoubleToStr(zzLots[i], 2) +" | "+
                      "-$" + DoubleToStr(loss, 2) +" | "+
                      "$" + DoubleToStr(gain, 2) +" | "+
                      "" + DoubleToStr(zzRiskGain[i], 1) +" | "+
                      "" + validDecision(i) +" | ";

        }                              
     
        //wtsPrintLineTrade("zzStart", i, zzStart[i], Blue, comment, validDecision(i));
        //wtsPrintLineTrade("zzStop", i, zzStop[i], Red, "", validDecision(i));
            
/*        
        //33%
        if ((zz33Start[i] > 0) && 
            ((m_agressiveState[i] == ST_DOWN && Bid < zz33Start[i]) ||
            ((m_agressiveState[i] == ST_UP && Bid > zz33Start[i])))) {
                
                //Start
                ObjectCreate("zz33Start_"+wtsGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz33Start[i]);
            
                if (m_agressiveState[i] == ST_UP) {
                    ObjectSet("zz33Start_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Green); 
                } else {
                    ObjectSet("zz33Start_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Red); 
                }
                ObjectSet("zz33Start_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz33Start_"+wtsGetPeriodByIndex(i), 
                              "[33%] Start "+wtsGetPeriodNameByIndex(i),
                              7, "Arial", White);
                
                //Stop
                ObjectCreate("zz33Stop_"+wtsGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz33Stop[i]);
            
                if (m_agressiveState[i] == ST_UP) {
                    ObjectSet("zz33Stop_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                } else {
                    ObjectSet("zz33Stop_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                }
                ObjectSet("zz33Stop_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz33Stop_"+wtsGetPeriodByIndex(i), 
                              "[33%] Stop "+wtsGetPeriodNameByIndex(i),
                              7, "Arial", White);
        } else {
            ObjectDelete("zz33Start_"+wtsGetPeriodByIndex(i));
            ObjectDelete("zz33Stop_"+wtsGetPeriodByIndex(i));
        }

        //50%
        if ((zz50Start[i] > 0) && 
            ((m_agressiveState[i] == ST_DOWN && Bid < zz50Start[i]) ||
            ((m_agressiveState[i] == ST_UP && Bid > zz50Start[i])))) {
                
                ObjectCreate("zz50Start_"+wtsGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz50Start[i]);
            
                if (m_agressiveState[i] == ST_UP) {
                    ObjectSet("zz50Start_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Green); 
                } else {
                    ObjectSet("zz50Start_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Red); 
                }
                ObjectSet("zz50Start_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz50Start_"+wtsGetPeriodByIndex(i), 
                              "[50%] Start "+wtsGetPeriodNameByIndex(i),
                              7, "Arial", White);

                //Stop
                ObjectCreate("zz50Stop_"+wtsGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz50Stop[i]);
            
                if (m_agressiveState[i] == ST_UP) {
                    ObjectSet("zz50Stop_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                } else {
                    ObjectSet("zz50Stop_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                }
                ObjectSet("zz50Stop_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz50Stop_"+wtsGetPeriodByIndex(i), 
                              "[50%] Stop "+wtsGetPeriodNameByIndex(i),
                              7, "Arial", White);
        } else {
            ObjectDelete("zz50Start_"+wtsGetPeriodByIndex(i));
            ObjectDelete("zz50Stop_"+wtsGetPeriodByIndex(i));
        }

        //60%
        if ((zz60Start[i] > 0) && 
            ((m_agressiveState[i] == ST_DOWN && Bid < zz60Start[i]) ||
            ((m_agressiveState[i] == ST_UP && Bid > zz60Start[i])))) {
                
                ObjectCreate("zz60Start_"+wtsGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz60Start[i]);
            
                if (m_agressiveState[i] == ST_UP) {
                    ObjectSet("zz60Start_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Green); 
                } else {
                    ObjectSet("zz60Start_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Red); 
                }
                ObjectSet("zz60Start_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz60Start_"+wtsGetPeriodByIndex(i), 
                              "[60%] Start "+wtsGetPeriodNameByIndex(i),
                              7, "Arial", White);

                //Stop
                ObjectCreate("zz60Stop_"+wtsGetPeriodByIndex(i), OBJ_HLINE, 0, 0, zz60Stop[i]);
            
                if (m_agressiveState[i] == ST_UP) {
                    ObjectSet("zz60Stop_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                } else {
                    ObjectSet("zz60Stop_"+wtsGetPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                }
                ObjectSet("zz60Stop_"+wtsGetPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz60Stop_"+wtsGetPeriodByIndex(i), 
                              "[60%] Stop "+wtsGetPeriodNameByIndex(i),
                              7, "Arial", White);
        } else {
            ObjectDelete("zz60Start_"+wtsGetPeriodByIndex(i));
            ObjectDelete("zz60Stop_"+wtsGetPeriodByIndex(i));
        }
*/
    }
/*
    ObjectSetText("label_start",
                  "SPREAD: " + DoubleToStr(spread, 0) + 
                  " LUCRO: " + DoubleToStr(m_orderStopLossGain, 2) + 
                  " TGT: " + DoubleToStr(m_orderTakeProfitGain, 2),
                  7, "Arial", DarkOrange);
*/
    rg = ((m_candleTop[getIndexByPeriod(Period())] - m_candleBottom[getIndexByPeriod(Period())]) / Point) / (m_normalTradeRisk[getIndexByPeriod(Period())] + spread);

    ObjectSetText("label_start",
                  "Spread("+DoubleToStr(spread, 0)+") "+
                  "Leg("+DoubleToStr((m_candleTop[getIndexByPeriod(Period())] - m_candleBottom[getIndexByPeriod(Period())]) / Point, 0)+") "+
                  "Risk("+DoubleToStr(m_normalTradeRisk[getIndexByPeriod(Period())] + spread, 0)+") "+
                  "RG("+DoubleToStr(rg, 1)+") "+
                  "$"+DoubleToStr(AccountBalance() * getRisk(getIndexByPeriod(Period())) * rg, 2)+" ",
                  7, "Arial", DarkOrange);

    for (i = 0; i < 1; i++) {
//        ObjectSetText("label_state"+i,
//                      "| "+wtsGetPeriodNameByIndex(i)+" | " + getStateName(m_normalState[i]) + "(" + getStateName(m_agressiveState[i]) + ") | " +
                      //"" + DoubleToStr(m_normalTradeRiskGain[i], 0) + "(" + DoubleToStr(zzRiskGain[i], 0) + ") | " +
                      //"" + m_orderOpenBarShift[i] + ":" + m_orderActive[i] + ":" + (!wtsNormalTradeValidDecision(i)) + "(" +(!validDecision(i)) + ") | " + 
                      //"" + DoubleToStr(m_orderTakeProfitGain[i], 2) + " | " +
                      //"" + DoubleToStr(m_orderPercent[i]*100,2) + " | ", 
                      //7, "Arial", DarkOrange);

    }
    
    ObjectSetText("label_state0",
                  "Retration: up("+m_candleLowestRetration[getIndexByPeriod(Period())]+":"+m_candleHighestAfterLowest[getIndexByPeriod(Period())]+") down("+m_candleHighestRetration[getIndexByPeriod(Period())]+":"+m_candleLowestAfterHighest[getIndexByPeriod(Period())]+")",
                  7, "Arial", DarkOrange);

    ObjectSetText("label_state1",
                  "State: "+
                  "H1("+getStateName(m_normalState[4])+") "+
                  "H4("+getStateName(m_normalState[3])+") "+
                  "D1("+getStateName(m_normalState[2])+") "+
                  "W1("+getStateName(m_normalState[1])+") "+
                  "MN("+getStateName(m_normalState[0])+") ",
                  7, "Arial", DarkOrange);


    ObjectSetText("label_state2",
                  "RG: "+
                  "H1("+DoubleToStr(m_candleMA[0], Digits)+") "+
                  "H4("+DoubleToStr(m_candleMA21[0], Digits)+") "+
                  "D1("+DoubleToStr(m_normalTradeRiskGain[2], 1)+") "+
                  "W1("+DoubleToStr(m_normalTradeRiskGain[1], 1)+") "+
                  "MN("+DoubleToStr(m_normalTradeRiskGain[0], 1)+") ",
                  7, "Arial", DarkOrange);
    
    checkOrdersOk();
    checkDecision();
    
    for (i = 0; i < MAX_PERIOD; i++) {
        wtsNormalTradeDecision(i);
    }
    
    return(0);
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
    double med = 0;
    double risk = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    
    switch (m_normalState[index]) {
        case ST_UNDEF: risk = 0; break;
        case ST_CONG: risk = zzRisk[index]; break;
        //case ST_NORISK: risk = zzRisk[index]; break;
        case ST_UP: risk = zzRisk[index]; break;
        case ST_DOWN: risk = zzRisk[index]; break;
    }

/*    
    if (m_agressiveState[index] == ST_UP || m_agressiveState[index] == ST_DOWN) {
        if (med > spread * 8) {
            risk = 0.16;    
        } else if ((med > spread * 4) && (med < spread * 8)) {
            risk = 0.08;
        } else if ((med > spread * 2) && (med < spread * 4)) {
            risk = 0.04;
        } else if ((med > spread * 1) && (med < spread * 2)) {
            risk = 0.02;
        } else {
            risk = 0.01;
        } 
    }
*/    
//    if (AccountBalance() < 50) {
//        risk *= 2;
//    }

    return (risk);
}

double getRiskAgressive(int index)
{
    double med = 0;
    double risk = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    
    switch (m_agressiveState[index]) {
        case ST_UNDEF: risk = 0; break;
        case ST_CONG: risk = zzRiskAgressive[index]; break;
        //case ST_NORISK: risk = zzRisk[index]; break;
        case ST_UP: risk = zzRiskAgressive[index]; break;
        case ST_DOWN: risk = zzRiskAgressive[index]; break;
    }

/*    
    if (m_agressiveState[index] == ST_UP || m_agressiveState[index] == ST_DOWN) {
        if (med > spread * 8) {
            risk = 0.16;    
        } else if ((med > spread * 4) && (med < spread * 8)) {
            risk = 0.08;
        } else if ((med > spread * 2) && (med < spread * 4)) {
            risk = 0.04;
        } else if ((med > spread * 1) && (med < spread * 2)) {
            risk = 0.02;
        } else {
            risk = 0.01;
        } 
    }
*/    
    if (AccountBalance() < 50) {
        risk *= 2;
    }

    return (risk);
}


void wtsCalcZigzag(string strPeriod, int period, int index)
{
    int i;
    double zz = 0;
    int highest_idx;
    int lowest_idx;
    int zzTotal = 0;
    int spread = MarketInfo(Symbol(), MODE_SPREAD);
    double zzBuffer[200];
    double zzBufferClose[200];
    double zzBufferTime[200];
    double zzShift[200];
    double pernaPerc = 0;
    double startPerc = 0;
    double targetPerc = 0;
    double highest = 0;
    double lowest = 0;
    double swapLong = MarketInfo(Symbol(), MODE_SWAPLONG);
    double swapShort = MarketInfo(Symbol(), MODE_SWAPSHORT);

    for (i = TOTAL_BARS; i != 0; i--) {
        zz = iCustom(Symbol(), period, "ZUP_v64", 0, i);
        if (zz != 0) {
            zzBuffer[zzTotal] = zz;
            zzBufferTime[zzTotal] = iTime(Symbol(), period, i);
            zzBufferClose[zzTotal] = iClose(Symbol(), period, i);
            zzShift[zzTotal] = i;
            zzTotal++;
        }
    } 
    
    m_candleMA[index] = iMA(Symbol(), period, 9, 0, MODE_EMA, PRICE_CLOSE, 0);
    m_candleMA21[index] = iMA(Symbol(), period, 21, 0, MODE_SMA, PRICE_CLOSE, 0);
    
    if (zzBuffer[0] > zzBuffer[1]) {
        m_candleTop[index] = zzBuffer[0];
        m_candleBottom[index] = zzBuffer[1];
        m_candleTopTime[index] = zzBufferTime[0];
        m_candleBottomTime[index] = zzBufferTime[1];
        m_normalState[index] = ST_DOWN;
        m_agressiveState[index] = ST_DOWN;
        m_candleLastBarShift[index] = zzShift[1];
    } else {
        m_candleTop[index] = zzBuffer[1];
        m_candleBottom[index] = zzBuffer[0];
        m_candleTopTime[index] = zzBufferTime[1];
        m_candleBottomTime[index] = zzBufferTime[0];
        m_agressiveState[index] = ST_UP;
        m_normalState[index] = ST_UP;
        m_candleLastBarShift[index] = zzShift[0];
    }
    
    m_candleTopBottomCount[index] = 0;
    
    for (i = 2; i < zzTotal; i++) {
        m_candleTopBottomCount[index]++;
        
        if (zzBuffer[i] > m_candleTop[index] && zzBufferClose[i] > m_candleTop[index]) {
            //Prev
            m_candleTopPrev[index] = m_candleTop[index];
            m_candleTopTimePrev[index] = m_candleTopTime[index];
            m_candleTopShiftPrev[index] = m_candleTopShift[index];
            m_candleBottomPrev[index] = m_candleBottom[index];
            m_candleBottomTimePrev[index] = m_candleBottomTime[index];
            m_candleBottomShiftPrev[index] = m_candleBottomShift[index];

            m_candleTop[index] = zzBuffer[i];
            m_candleTopTime[index] = zzBufferTime[i];
            m_candleTopShift[index] = zzShift[i];

            m_candleBottom[index] = zzBuffer[i-1];
            m_candleBottomTime[index] = zzBufferTime[i-1];
            m_candleBottomShift[index] = zzShift[i-1];

            m_agressiveState[index] = ST_UP;
            m_normalState[index] = ST_UP;
            m_candleTopBottomCount[index] = 0;
            m_candleLastBarShift[index] = zzShift[i];
        } else if (zzBuffer[i] > m_candleTop[index]) {
            //Prev
            m_candleTopPrev[index] = m_candleTop[index];
            m_candleTopTimePrev[index] = m_candleTopTime[index];
            m_candleTopShiftPrev[index] = m_candleTopShift[index];
            m_candleBottomPrev[index] = m_candleBottom[index];
            m_candleBottomTimePrev[index] = m_candleBottomTime[index];
            m_candleBottomShiftPrev[index] = m_candleBottomShift[index];

            m_candleTop[index] = zzBuffer[i];
            m_candleTopTime[index] = zzBufferTime[i];
            m_candleTopShift[index] = zzShift[i];

            m_candleBottom[index] = zzBuffer[i-1];
            m_candleBottomTime[index] = zzBufferTime[i-1];
            m_candleBottomShift[index] = zzShift[i-1];

            m_agressiveState[index] = ST_UP;
            m_normalState[index] = ST_UP;
            m_candleTopBottomCount[index] = 0;
            m_candleLastBarShift[index] = zzShift[i];
        }
        
        if (zzBuffer[i] < m_candleBottom[index] && zzBufferClose[i] < m_candleBottom[index]) {
            //Prev
            m_candleTopPrev[index] = m_candleTop[index];
            m_candleTopTimePrev[index] = m_candleTopTime[index];
            m_candleTopShiftPrev[index] = m_candleTopShift[index];
            m_candleBottomPrev[index] = m_candleBottom[index];
            m_candleBottomTimePrev[index] = m_candleBottomTime[index];
            m_candleBottomShiftPrev[index] = m_candleBottomShift[index];

            m_candleTop[index] = zzBuffer[i-1];
            m_candleTopTime[index] = zzBufferTime[i-1];
            m_candleTopShift[index] = zzShift[i-1];
            
            m_candleBottom[index] = zzBuffer[i];
            m_candleBottomTime[index] = zzBufferTime[i];
            m_candleBottomShift[index] = zzShift[i];

            m_agressiveState[index] = ST_DOWN;
            m_normalState[index] = ST_DOWN;
            m_candleTopBottomCount[index] = 0;
            m_candleLastBarShift[index] = zzShift[i];
        } else if (zzBuffer[i] < m_candleBottom[index]) {
            //Prev
            m_candleTopPrev[index] = m_candleTop[index];
            m_candleTopTimePrev[index] = m_candleTopTime[index];
            m_candleTopShiftPrev[index] = m_candleTopShift[index];
            m_candleBottomPrev[index] = m_candleBottom[index];
            m_candleBottomTimePrev[index] = m_candleBottomTime[index];
            m_candleBottomShiftPrev[index] = m_candleBottomShift[index];

            m_candleTop[index] = zzBuffer[i-1];
            m_candleTopTime[index] = zzBufferTime[i-1];
            m_candleTopShift[index] = zzShift[i-1];

            m_candleBottom[index] = zzBuffer[i];
            m_candleBottomTime[index] = zzBufferTime[i];
            m_candleBottomShift[index] = zzShift[i];
            
            m_agressiveState[index] = ST_DOWN;
            m_normalState[index] = ST_DOWN;
            m_candleTopBottomCount[index] = 0;
            m_candleLastBarShift[index] = zzShift[i];
        }
    }

    if (m_candleTopBottomCount[index] > 1) {
        m_agressiveState[index] = ST_UNDEF;
    }

    m_candleLeg[index] = (m_candleTop[index] - m_candleBottom[index]) / Point;

    wtsSetCandleFibo(index);

    wtsNormalTrade(index);
    //wtsPrintLT(index);

    //33%    
    if (m_agressiveState[index] == ST_UP) {
        pernaPerc = m_candleTop[index] / m_candleBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz33Start[index] = m_candleFibo33[index] * (1 - startPerc);
        zz33Stop[index] = zz33Start[index] - ((2 * m_candleShadowMed[index]) * Point);
        zz33Target[index] = zz33Stop[index] + (m_candleLeg[index] * Point * (1 - targetPerc));
    } else if (m_agressiveState[index] == ST_DOWN) {
        pernaPerc = 1 - m_candleBottom[index] / m_candleTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz33Start[index] = m_candleFibo33[index] * (1 + startPerc) + spread * Point;
        zz33Stop[index] = zz33Start[index] + (2 * m_candleShadowMed[index] * Point);
        zz33Target[index] = zz33Stop[index] - (m_candleLeg[index] * Point * (1 - targetPerc));
    } else {
        zz33Start[index] = 0;
        zz33Stop[index] = 0;
    }

    //50%
    if (m_agressiveState[index] == ST_UP) {
        pernaPerc = m_candleTop[index] / m_candleBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz50Start[index] = m_candleFibo50[index] * (1 - startPerc);
        zz50Stop[index] = zz50Start[index] - ((2 * m_candleShadowMed[index]) * Point);
        zz50Target[index] = zz50Stop[index] + (m_candleLeg[index] * Point * (1 - targetPerc));
    } else if (m_agressiveState[index] == ST_DOWN) {
        pernaPerc = 1 - m_candleBottom[index] / m_candleTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz50Start[index] = m_candleFibo50[index] * (1 + startPerc) + spread * Point;
        zz50Stop[index] = zz50Start[index] + (2 * m_candleShadowMed[index] * Point);
        zz50Target[index] = zz50Stop[index] - (m_candleLeg[index] * Point * (1 - targetPerc));
    } else {
        zz50Start[index] = 0;
        zz50Start[index] = 0;
    }

    //60%
    if (m_agressiveState[index] == ST_UP) {
        pernaPerc = m_candleTop[index] / m_candleBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz60Start[index] = m_candleFibo60[index] * (1 - startPerc);
        zz60Stop[index] = zz60Start[index] - ((2 * m_candleShadowMed[index]) * Point);
        zz60Target[index] = zz60Stop[index] + (m_candleLeg[index] * Point * (1 - targetPerc));
    } else if (m_agressiveState[index] == ST_DOWN) {
        pernaPerc = 1 - m_candleBottom[index] / m_candleTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        targetPerc = pernaPerc * 0.0015 / 0.1;
        zz60Start[index] = m_candleFibo60[index] * (1 + startPerc) + spread * Point;
        zz60Stop[index] = zz60Start[index] + (2 * m_candleShadowMed[index] * Point);
        zz60Target[index] = zz60Stop[index] - (m_candleLeg[index] * Point * (1 - targetPerc));
    } else {
        zz60Start[index] = 0;
        zz60Start[index] = 0;
    }


    highest_idx = iHighest(Symbol(), period, MODE_HIGH, m_candleLastBarShift[index]);
    highest = iHigh(Symbol(), period, highest_idx)+spread*Point;

    lowest_idx = iLowest(Symbol(), period, MODE_LOW, m_candleLastBarShift[index]);
    lowest = iLow(Symbol(), period, lowest_idx);

    if (
        ((lowest_idx > 0 && m_agressiveState[index] == ST_UP && lowest < zz60Start[index]) || (lowest_idx == 0 && Ask < zz60Start[index])) ||
        ((highest_idx > 0 && m_agressiveState[index] == ST_DOWN && highest > zz60Start[index]) || (highest_idx == 0 && Bid > zz60Start[index])) ||
        (m_orderActive[index] == true)) {
        zz33Start[index] = 0;
        zz50Start[index] = 0;
        zz60Start[index] = 0;
        zz33Stop[index] = 0;
        zz50Stop[index] = 0;
        zz60Stop[index] = 0;
        zz33Target[index] = 0;
        zz50Target[index] = 0;
        zz60Target[index] = 0;
        m_agressiveState[index] = ST_UNDEF;
    } else if (
        ((lowest_idx > 0 && m_agressiveState[index] == ST_UP && lowest < zz50Start[index]) || (lowest_idx == 0 && Ask < zz50Start[index])) ||
        ((highest_idx > 0 && m_agressiveState[index] == ST_DOWN && highest > zz50Start[index]) || (highest_idx == 0 && Bid > zz50Start[index]))) {
        zz33Start[index] = 0;
        zz50Start[index] = 0;
        zz33Stop[index] = 0;
        zz50Stop[index] = 0;
        zz33Target[index] = 0;
        zz50Target[index] = 0;
    } else if (
        ((lowest_idx > 0 && m_agressiveState[index] == ST_UP && lowest < zz33Start[index]) || (lowest_idx == 0 && Ask < zz33Start[index])) ||
        ((highest_idx > 0 && m_agressiveState[index] == ST_DOWN && highest > zz33Start[index]) || (highest_idx == 0 && Bid > zz33Start[index]))) {
        zz33Start[index] = 0;
        zz33Stop[index] = 0;
        zz33Target[index] = 0;
    } 

    zz33Start[index] = NormalizeDouble(zz33Start[index] ,Digits-1);
    zz50Start[index] = NormalizeDouble(zz50Start[index] ,Digits-1);
    zz60Start[index] = NormalizeDouble(zz60Start[index] ,Digits-1);

    zz33Stop[index] = NormalizeDouble(zz33Stop[index] ,Digits-1);
    zz50Stop[index] = NormalizeDouble(zz50Stop[index] ,Digits-1);
    zz60Stop[index] = NormalizeDouble(zz60Stop[index] ,Digits-1);


    zz33Target[index] = NormalizeDouble(zz33Target[index] ,Digits-1);
    zz50Target[index] = NormalizeDouble(zz50Target[index] ,Digits-1);
    zz60Target[index] = NormalizeDouble(zz60Target[index] ,Digits-1);

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
    
    if (index == 0) {
        m_aux[2] = lowest_idx;
        m_aux[3] = lowest;
        m_aux[4] = zzStop[0];
    }
    
    if (lowest_idx == 0 && m_agressiveState[index] == ST_UP && lowest < zzStop[index]) {
        zzStop[index] = NormalizeDouble(lowest-30*m_normalTradeStartMultiply[index]*Point, Digits-1);
        zzReturn[index] += 1;
    } else if (highest_idx == 0 && m_agressiveState[index] == ST_DOWN && highest > zzStop[index]) {
        zzStop[index] = NormalizeDouble(highest+30*m_normalTradeStartMultiply[index]*Point, Digits-1);
        zzReturn[index] += 1;
    }
    
/*    
    if (index > 0 && zzStart[index] != 0) {
    
//        if ((m_agressiveState[index] == ST_UP && m_agressiveState[index-1] == ST_DOWN) ||
            //(m_agressiveState[index] == ST_DOWN && m_agressiveState[index-1] == ST_UP)) {
            //zzTakeProfit[index] = zzStart[index-1];       
                      
        if (m_agressiveState[index] == ST_UP && m_agressiveState[index-1] == ST_DOWN) {
            zzTakeProfit[index] = m_candleTop[index-1] - m_candleShadowMed[index-1]*Point;
        } else if (m_agressiveState[index] == ST_DOWN && m_agressiveState[index-1] == ST_UP) {
            zzTakeProfit[index] = m_candleBottom[index-1] + m_candleShadowMed[index-1]*Point;
        } else if (m_agressiveState[index] == ST_UP && m_agressiveState[index-1] == ST_UP && m_candleTop[index] == m_candleTop[index-1]) {
            zzTakeProfit[index] = zzTakeProfit[index-1];
        } else if ((m_agressiveState[index] == ST_UP && (m_agressiveState[index-1] == ST_UP || m_agressiveState[index-1] == ST_CONG || m_agressiveState[index-1] == ST_UNDEF))) {
            zzTakeProfit[index] = m_candleTop[index-1] - m_candleShadowMed[index-1]*Point;
        } else if (m_agressiveState[index] == ST_DOWN && m_agressiveState[index-1] == ST_DOWN && m_candleBottom[index] == m_candleBottom[index-1]) {
            zzTakeProfit[index] = zzTakeProfit[index-1];
        } else if ((m_agressiveState[index] == ST_DOWN && (m_agressiveState[index-1] == ST_DOWN || m_agressiveState[index-1] == ST_CONG || m_agressiveState[index-1] == ST_UNDEF))) {
            zzTakeProfit[index] = m_candleBottom[index-1] + m_candleShadowMed[index-1]*Point;
        }
    }
*/    
/*
    if (index > 1 && zzStart[index] != 0) {
        if ((m_agressiveState[index] == ST_UP && m_agressiveState[index-2] == ST_UP) ||
            (m_agressiveState[index] == ST_DOWN && m_agressiveState[index-2] == ST_DOWN)) {
            zzTakeProfit[index] = zzTakeProfit[index-2];
        } else if ((m_agressiveState[index] == ST_UP && m_agressiveState[index-2] == ST_DOWN) ||
                   (m_agressiveState[index] == ST_DOWN && m_agressiveState[index-2] == ST_UP)) {
            zzTakeProfit[index] = zzStart[index-2];                 
        } else if ((m_agressiveState[index] == ST_UP && m_agressiveState[index-2] == ST_CONG)) {
            zzTakeProfit[index] = m_candleTop[index-2] - m_candleShadowMed[index-2]*Point;
        } else if ((m_agressiveState[index] == ST_DOWN && m_agressiveState[index-2] == ST_CONG)) {
            zzTakeProfit[index] = m_candleBottom[index-2] + m_candleShadowMed[index-2]*Point;
        }
    }
*/    
    if (zzStart[index] != 0) {
        if (m_agressiveState[index] == ST_UP) {
            zzStopLoss[index] = (zzStart[index] - zzStop[index]) / Point;
            zzRiskGain[index] = (zzTakeProfit[index] - (zzStart[index])) / ((zzStart[index]) - zzStop[index]);
        } else {
            zzStopLoss[index] = (zzStop[index] - zzStart[index]) / Point;
            zzRiskGain[index] = ((zzStart[index]) - zzTakeProfit[index]) / (zzStop[index] - (zzStart[index]));
        }

        zzLots[index] = NormalizeDouble((AccountBalance() * getRiskAgressive(index)) / (zzStopLoss[index] + spread), 2);
            
        if (m_agressiveState[index] == ST_UP) {
            zzLucro[index] = zzLots[index] * ((zzTakeProfit[index] - zzStart[index]) / Point - spread);
            zzPerda[index] = zzLots[index] * ((zzStop[index] - zzStart[index]) / Point - spread);
        } else {
            zzLucro[index] = zzLots[index] * ((zzStart[index] - zzTakeProfit[index]) / Point - spread);
            zzPerda[index] = zzLots[index] * ((zzStart[index] - zzStop[index]) / Point - spread);
        }
    }

    zzRiskGain[index] = NormalizeDouble(zzRiskGain[index], 0);
    
    // Remove trade agressivo para 1 hora
    //if (index == 4) {
    //zzRiskGain[index] = 0;    
    //}

    if (((zzRiskGain[index] < MIN_RISK_GAIN) && m_agressiveState[index] == ST_DOWN) || 
        ((zzRiskGain[index] < MIN_RISK_GAIN) && m_agressiveState[index] == ST_UP) || 
        (m_orderActive[index] == true) ||
        (m_agressiveState[index] == ST_UP && m_agressiveState[index-1] == ST_DOWN && m_orderActive[index-1] == true) ||
        (m_agressiveState[index] == ST_DOWN && m_agressiveState[index-1] == ST_UP && m_orderActive[index-1] == true) ||
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
            //m_agressiveState[index] = ST_NORISK;
    }
}

int wtsGetPeriodByIndex(int index) 
{
    int ret = 0;
    
    switch (index) {
        case 0: ret = PERIOD_H1; break;
        case 1: ret = PERIOD_H4; break;
        case 2: ret = PERIOD_D1; break;
        case 3: ret = PERIOD_H4; break;
        case 4: ret = PERIOD_H1; break;
        case 5: ret = PERIOD_M15; break;
        case 6: ret = PERIOD_M5; break;
        case 7: ret = PERIOD_M5; break;
        case 8: ret = PERIOD_M1; break;
    }

    return (ret);
}

string wtsGetPeriodNameByIndex(int index) 
{
    string ret = 0;
    
    switch (index) {
        case 0: ret = "H1"; break;
        case 1: ret = "H4"; break;
        case 2: ret = "D1"; break;
        case 3: ret = "H4"; break;
        case 4: ret = "H1"; break;
        case 5: ret = "M15"; break;
        case 6: ret = "M5"; break;
        case 7: ret = "M5"; break;
        case 8: ret = "M1"; break;
    }

    return (ret);
}


string getStateName(int state)
{
    string name = "";
    
    switch (state) {
        case ST_UNDEF:  name = "--"; break;
        case ST_CONG:   name = "cg"; break;
        case ST_UP:     name = "up"; break;
        case ST_DOWN:   name = "dw"; break;
        //case ST_NORISK: name = "no"; break;
    }
    
    return (name);
}

int getIndexByPeriod(int period) 
{
    int ret = 0;
    
    switch (period) {
        case PERIOD_H1: ret = 0; break;
        case PERIOD_H4: ret = 1; break;
        case PERIOD_D1: ret = 2; break;
        case PERIOD_W1: ret = 3; break;
        case PERIOD_MN1: ret = 4; break;
        case PERIOD_M15: ret = 5; break;
        case PERIOD_M5: ret = 6; break;
        //case PERIOD_M5: ret = 5; break;
        case PERIOD_M1: ret = 8; break;
    }

    return (ret);
}

void calcMedShadow()
{
    int i, j;
    int medTotal = 24;
    double medLowShadow = 0;
    double medHighShadow = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    double cdlOpenCloseLow;
    double cdlOpenCloseHigh;
    double cdlLow;
    double cdlHigh;

    for (j = 0; j < MAX_PERIOD; j++) {
        
        medLowShadow = 0;
        medHighShadow = 0;
        
        for (i = medTotal; i != 0; i--) {
    
            if (is_high_clande(wtsGetPeriodByIndex(j), i)) {
                cdlOpenCloseLow = iOpen(Symbol(), wtsGetPeriodByIndex(j), i);
                cdlOpenCloseHigh = iClose(Symbol(), wtsGetPeriodByIndex(j), i);
            } else {
                cdlOpenCloseLow = iClose(Symbol(), wtsGetPeriodByIndex(j), i);
                cdlOpenCloseHigh = iOpen(Symbol(), wtsGetPeriodByIndex(j), i);
            }
            
            cdlLow = iLow(Symbol(), wtsGetPeriodByIndex(j), i);
            cdlHigh = iHigh(Symbol(), wtsGetPeriodByIndex(j), i);
        
            medLowShadow += cdlOpenCloseLow - cdlLow;
            medHighShadow += cdlHigh - cdlOpenCloseHigh;
        }
    
        medLowShadow /= medTotal;
        medLowShadow = NormalizeDouble(medLowShadow / Point, 0);
    
        if (medLowShadow < spread) {
            //medLowShadow = spread;
        }

        medHighShadow /= medTotal;
        medHighShadow = NormalizeDouble(medHighShadow / Point, 0);
    
        if (medHighShadow < spread) {
            //medHighShadow = spread;
        }
        
        m_candleShadowMed[j] = NormalizeDouble((medLowShadow + medHighShadow) / 2, 0);
    }
}

bool wtsNormalTradeValidDecision(int index)
{
    bool decision = false;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    
    RefreshRates();
    
    if (m_normalState[index] == ST_UP) {
        if (m_normalTradeLots[index] > 0 && 
            m_normalTradeRiskGain[index] >= m_normalTradeRiskGainMin[index] &&
            m_orderActive[index] == false &&
            m_normalTradeStart[index] >= m_candleMA[index]) {
        
            decision = true;
        }
    } else if (m_normalState[index] == ST_DOWN) {
        if (m_normalTradeLots[index] > 0 && 
            m_normalTradeRiskGain[index] >= m_normalTradeRiskGainMin[index] &&
            m_orderActive[index] == false &&
            m_normalTradeStart[index] <= (m_candleMA[index] + spread*Point)) {
            
            decision = true;
        }
    }

    return (decision);
}

bool validDecision(int index)
{
    bool decision = false;
    
    RefreshRates();
    
    if (m_agressiveState[index] == ST_UP) {
        if (zzLots[index] > 0 && 
            m_orderOpenBarShift[index] < 0 && 
            zzRiskGain[index] >= MIN_RISK_GAIN &&
            m_orderActive[index] == false) {
        
            decision = true;
        }
    } else if (m_agressiveState[index] == ST_DOWN) {
        if (zzLots[index] > 0 && 
            m_orderOpenBarShift[index] < 0 && 
            zzRiskGain[index] >= MIN_RISK_GAIN &&
            m_orderActive[index] == false) {
            
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

        RefreshRates();
        if (validDecision(i) && NormalizeDouble(Bid, Digits-1) == zzStart[i]) {
            RefreshRates();
            ticket = OrderSend(Symbol(), OP_BUY, zzLots[i], Ask, 3, 
                               Bid - (zzStopLoss[i] * Point), zzTakeProfit[i], 
                               "["+wtsGetPeriodByIndex(i)+"]["+zzReturn[i]+"]["+DoubleToStr(zzRiskGain[i], 0)+"]["+DoubleToStr(AccountBalance(),0)+"]", 
                               zzMagicNumber, 0, Blue);
            if(ticket < 0) {
                err=GetLastError();
                Print("error(",err,"): ",ErrorDescription(err));
                Sleep(1000);
            } 
        } else if (validDecision(i) && NormalizeDouble(Ask, Digits-1) == zzStart[i]) {
            RefreshRates();
            ticket = OrderSend(Symbol(), OP_SELL, zzLots[i], Bid, 3, 
                               Ask + (zzStopLoss[i] * Point), zzTakeProfit[i], 
                               "["+wtsGetPeriodByIndex(i)+"]["+zzReturn[i]+"]["+DoubleToStr(zzRiskGain[i], 0)+"]["+DoubleToStr(AccountBalance(),0)+"]", 
                               zzMagicNumber, 0, Blue);
            if(ticket < 0) {
                err=GetLastError();
                Print("error(",err,"): ",ErrorDescription(err));
                Sleep(1000);
            }
        }
    }
}

void wtsNormalTradeDecision(int index)
{
    int i;
    int err;
    int ticket = -1;

    RefreshRates();
    if (wtsNormalTradeValidDecision(index) && NormalizeDouble(Bid, Digits-1) == m_normalTradeStart[index] && m_normalState[index] == ST_UP) {
        RefreshRates();
        ticket = OrderSend(Symbol(), OP_BUY, m_normalTradeLots[index], Ask, 3, 
                           Bid - (m_normalTradeRisk[index] * Point), m_normalTradeTakeProfit[index], 
                           "["+wtsGetPeriodByIndex(index)+"]["+m_normalTradeType[index]+"]["+DoubleToStr(m_normalTradeRiskGain[index], 0)+"]["+DoubleToStr(AccountBalance(),0)+"]", 
                           zzMagicNumber, 0, Blue);
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(1000);
        } 
    } else if (wtsNormalTradeValidDecision(index) && NormalizeDouble(Ask, Digits-1) == m_normalTradeStart[index] && m_normalState[index] == ST_DOWN) {
        RefreshRates();
        ticket = OrderSend(Symbol(), OP_SELL, m_normalTradeLots[index], Bid, 3, 
                           Ask + (m_normalTradeRisk[index] * Point), m_normalTradeTakeProfit[index], 
                           "["+wtsGetPeriodByIndex(index)+"]["+m_normalTradeType[index]+"]["+DoubleToStr(m_normalTradeRiskGain[index], 0)+"]["+DoubleToStr(AccountBalance(),0)+"]", 
                           zzMagicNumber, 0, Blue);
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(1000);
        }
    }
}

void checkOrders()
{
    int i;
    int index;
    int total = OrdersTotal();   
    int period;
    int str_open = 0;
    int str_close = 0;
    double candlePrevClose = 0;
    double stopLoss = 0;
    double perna = 0;
    double perc_gain = 0;
    double threshold;
    double recuo = 0;
    double riskGain = 0;
    double patrimonio = 0;
    double newStop = 0;
    double lucro003 = 0;
    double lucro005 = 0;
    double lucro008 = 0;
    double lucro013 = 0;
    double lucro021 = 0;
    double lucro034 = 0;
    double lucro055 = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    
    for (i = 0; i < MAX_PERIOD; i++) {
        if (iBarShift(Symbol(), wtsGetPeriodPrev(wtsGetPeriodByIndex(i)), m_orderOpenTime[i]) > 0) {
            m_orderOpenTime[i] = 0;
            m_orderOpenBarShift[i] = -1;
        }
        
        m_orderActive[i] = false;
        m_orderLots[i] = 0;
        m_orderOpenPrice[i] = 0;
        m_orderProtectionValue[i] = 0;
        m_orderPercentToProtection[i] = 0;
        m_orderPercentToProtectionPrev[i] = 0;
        m_orderRiskGain[i] = 0;
        m_orderStopLoss[i] = 0;
        m_orderStopLossGain[i] = 0;
        m_orderTakeProfit[i] = 0;
        m_orderTakeProfitGain[i] = 0;
        m_orderTicket[i] = 0;
        
        ObjectDelete(WTS_PROTECTION_VALUE + wtsGetPeriodByIndex(i) + "");
    }

    for(i = 0; i < total; i++) {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES); 
        // check for symbol & magic number
        //if(OrderSymbol() == Symbol() && OrderMagicNumber() == zzMagicNumber) {
        if(OrderSymbol() == Symbol()) {
            
            RefreshRates();
      
            str_open = StringFind(OrderComment(), "[", 0);
            
            if (str_open < 0) {
                period = StrToInteger(OrderComment());
            } else {
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                period = StrToInteger(StringSubstr(OrderComment(), str_open, str_close-str_open));
                
                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                recuo = StrToInteger(StringSubstr(OrderComment(), str_open, str_close-str_open));
                
                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                riskGain = StrToInteger(StringSubstr(OrderComment(), str_open, str_close-str_open));
                
                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                patrimonio = StrToInteger(StringSubstr(OrderComment(), str_open, str_close-str_open));
            }
            
            index = getIndexByPeriod(period);
            m_orderLots[index] = OrderLots();
            m_orderOpenPrice[index] = OrderOpenPrice();
            m_orderStopLoss[index] = OrderStopLoss();
            m_orderTakeProfit[index] = OrderTakeProfit();
            m_orderTicket[index] = OrderTicket();
            m_orderRiskGain[index] = riskGain;
            m_orderOpenTime[index] = OrderOpenTime();
            m_orderOpenBarShift[index] = iBarShift(Symbol(), period, m_orderOpenTime[index]);

            if (AccountBalance() < patrimonio) {
                perc_gain = (AccountProfit() - (patrimonio - AccountBalance())) / patrimonio;
            } else {
                perc_gain = AccountProfit() / AccountBalance();
            }

            // long position is opened
            if(OrderType() == OP_BUY) {
            
                m_orderActive[index] = true;
                
                m_orderStopLossGain[index] = ((m_orderStopLoss[index] - m_orderOpenPrice[index])) * m_orderLots[index] / Point;
                m_orderStopLossGain[index] += OrderSwap();
                
                m_orderTakeProfitGain[index] = ((m_orderTakeProfit[index] - m_orderOpenPrice[index]) / Point) * m_orderLots[index];

                m_orderPercent[index] = (Bid - m_orderOpenPrice[index]) / (m_orderTakeProfit[index] - m_orderOpenPrice[index]);
                
                stopLoss = wtsStopLossCalc(index, OP_BUY);
                
                candlePrevClose = NormalizeDouble(iClose(Symbol(), period, 1), Digits-1);
                
                threshold = NormalizeDouble((iLow(Symbol(), period, 1) - (m_candleShadowMed[index]) * Point), 5);
                
                if (NormalizeDouble(Bid, Digits-1) > m_orderProtectionValuePrev[index]) {
                    m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
                    m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];
                }
                
                if ((m_orderStopLoss[index] < m_orderOpenPrice[index]) && ((Bid - m_orderOpenPrice[index]) > ((m_orderOpenPrice[index] - m_orderStopLoss[index])*2))) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], m_orderOpenPrice[index], m_orderTakeProfit[index], 0, Blue)) {
                        Sleep(1000);
                    } 
                } else if (stopLoss > m_orderStopLoss[index] && NormalizeDouble(Bid, Digits-1) == m_orderProtectionValuePrev[index]) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], stopLoss, m_orderTakeProfit[index], 0, Green)) {
                        Sleep(1000);
                    } else {
                        m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
                        m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];                    
                    }
                } else if ((m_orderPercent[index] >= 0.85) && threshold > m_orderStopLoss[index] && is_high_up_candle(period)) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], threshold, m_orderTakeProfit[index], 0, Yellow)) {
                        Sleep(1000);
                    }
                }
            } else if (OrderType() == OP_SELL) {

                m_orderStopLossGain[index] = ((m_orderOpenPrice[index] - m_orderStopLoss[index])) * m_orderLots[index] / Point;
                m_orderStopLossGain[index] += OrderSwap();
                
                m_orderTakeProfitGain[index] = ((m_orderOpenPrice[index] - m_orderTakeProfit[index])) * m_orderLots[index] / Point;

                m_orderActive[index] = true;
                m_orderPercent[index] = (m_orderOpenPrice[index] - Ask) / (m_orderOpenPrice[index] - m_orderTakeProfit[index]);

                stopLoss = wtsStopLossCalc(index, OP_SELL);

                candlePrevClose = NormalizeDouble(iClose(Symbol(), period, 1), Digits-1);
                
                threshold = NormalizeDouble((iHigh(Symbol(), period, 1) + (m_candleShadowMed[index] + spread) * Point), 5);
                
                if (NormalizeDouble(Ask, Digits-1) < m_orderProtectionValuePrev[index]) {
                    m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
                    m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];
                }
                
                if ((m_orderStopLoss[index] > m_orderOpenPrice[index]) && ((m_orderOpenPrice[index] - Ask) > ((m_orderStopLoss[index] - m_orderOpenPrice[index])*2))) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], m_orderOpenPrice[index], m_orderTakeProfit[index], 0, Blue)) {
                        Sleep(1000);
                    }
                } else if (stopLoss < m_orderStopLoss[index] && NormalizeDouble(Ask, Digits-1) == m_orderProtectionValuePrev[index] && stopLoss != 0) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], stopLoss, m_orderTakeProfit[index], 0, Green)) {
                        Sleep(1000);
                    } else {
                        m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
                        m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];                    
                    }
                } else if ((m_orderPercent[index] >= 0.85) && threshold < m_orderStopLoss[index] && is_low_down_candle(period)) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], threshold, m_orderTakeProfit[index], 0, Yellow)) {
                        Sleep(1000);
                    }
                }
            }
        }
    }
}


void checkOrdersOk()
{
    int i;
    int index;
    int total = OrdersTotal();   
    int period;
    int str_open = 0;
    int str_close = 0;
    double candlePrevClose = 0;
    double stopLoss = 0;
    double perna = 0;
    double perc_gain = 0;
    double threshold;
    double recuo = 0;
    double riskGain = 0;
    double patrimonio = 0;
    double newStop = 0;
    double protection = 0;
    double orderParcialGain = 0;
    double orderGain = 0;
    double orderPercent = 0;
    double orderPercent55 = 0;
    double orderPercent85 = 0;
    double swapStop = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    
    for (i = 0; i < MAX_PERIOD; i++) {
        //if (iBarShift(Symbol(), wtsGetPeriodPrev(wtsGetPeriodByIndex(i)), m_orderOpenTime[i]) > 0) {
            m_orderOpenTime[i] = 0;
            m_orderOpenBarShift[i] = -1;
        //}
        
        m_orderActive[i] = false;
        m_orderLots[i] = 0;
        m_orderOpenPrice[i] = 0;
        m_orderProtectionValue[i] = 0;
        m_orderPercentToProtection[i] = 0;
        m_orderPercentToProtectionPrev[i] = 0;
        m_orderRiskGain[i] = 0;
        m_orderStopLoss[i] = 0;
        m_orderStopLossGain[i] = 0;
        m_orderTakeProfit[i] = 0;
        m_orderTakeProfitGain[i] = 0;
        m_orderTicket[i] = 0;

        m_candleHighestRetration[i] = 0;
        m_candleLowestAfterHighest[i] = 0;
        m_candleLowestRetration[i] = 0;
        m_candleHighestAfterLowest[i] = 0;
        
        ObjectDelete(WTS_PROTECTION_VALUE + wtsGetPeriodByIndex(i) + "");
    }

    for(i = 0; i < total; i++) {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES); 
        // check for symbol & magic number
        //if(OrderSymbol() == Symbol() && OrderMagicNumber() == zzMagicNumber) {
        if(OrderSymbol() == Symbol()) {
            
            RefreshRates();
      
            str_open = StringFind(OrderComment(), "[", 0);

            if (str_open < 0) {
                if ((OrderType() == OP_BUY && OrderOpenPrice() > OrderStopLoss()) ||
                    (OrderType() == OP_SELL && OrderOpenPrice() < OrderStopLoss())) {
                    if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), OrderTakeProfit(), 0, Blue)) {
                        Sleep(1000);
                    }
                }
                
                if (OrderType() == OP_BUY) {
                    orderPercent55 = NormalizeDouble(OrderOpenPrice() + (OrderTakeProfit() - OrderOpenPrice()) * 0.50, Digits-1);
                    orderPercent85 = NormalizeDouble(OrderOpenPrice() + (OrderTakeProfit() - OrderOpenPrice()) * 0.80, Digits-1);
                    
                    if (OrderSwap() < 0) {
                        swapStop = - (OrderSwap() / OrderLots()) * Point;
                    }
                    
                    if (NormalizeDouble(Bid, Digits-1) >= orderPercent85 && orderPercent55 > OrderStopLoss()) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), orderPercent55, OrderTakeProfit(), 0, Yellow)) {
                            Sleep(1000);
                        }
                    } else if (OrderSwap() < 0 && Bid > OrderOpenPrice() && OrderStopLoss() >= OrderOpenPrice() && ((OrderStopLoss() - OrderOpenPrice()) < swapStop)) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + swapStop, OrderTakeProfit(), 0, Green)) {
                            Sleep(1000);
                        }
                    }
                    
                    orderParcialGain = (orderPercent55 - OrderOpenPrice()) / Point * OrderLots();
                    orderGain = (OrderTakeProfit() - OrderOpenPrice()) / Point * OrderLots();
                    orderPercent = (Bid - OrderOpenPrice()) / (OrderTakeProfit() - OrderOpenPrice()) * 100;
                } else if (OrderType() == OP_SELL) {
                    orderPercent55 = NormalizeDouble(OrderOpenPrice() - (OrderOpenPrice() - OrderTakeProfit()) * 0.50, Digits-1);
                    orderPercent85 = NormalizeDouble(OrderOpenPrice() - (OrderOpenPrice() - OrderTakeProfit()) * 0.80, Digits-1);
                    
                    if (OrderSwap() < 0) {
                       swapStop = - (OrderSwap() / OrderLots()) * Point;
                    }
                    
                    if (NormalizeDouble(Ask, Digits-1) <= orderPercent85 && orderPercent55 < OrderStopLoss()) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), orderPercent55, OrderTakeProfit(), 0, Yellow)) {
                            Sleep(1000);
                        }
                    } else if (OrderSwap() < 0 && Ask < OrderOpenPrice() && OrderStopLoss() <= OrderOpenPrice() && ((OrderOpenPrice() - OrderStopLoss()) < swapStop)) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - swapStop, OrderTakeProfit(), 0, Green)) {
                            Sleep(1000);
                        }
                    }
                    
                    orderParcialGain = (OrderOpenPrice() - orderPercent55) / Point * OrderLots();
                    orderGain = (OrderOpenPrice() - OrderTakeProfit()) / Point * OrderLots();
                    orderPercent = (OrderOpenPrice() - Ask) / (OrderOpenPrice() - OrderTakeProfit()) * 100;
                }
                
                wtsPrintLine("prot start"+OrderTicket(), orderPercent85, Pink, "prot start");
                wtsPrintLine("prot stop"+OrderTicket(), orderPercent55, Pink, "prot stop: $"+DoubleToStr(orderParcialGain, 2)+" ($"+DoubleToStr(orderGain, 2)+") "+DoubleToStr(orderPercent, 2)+"%");
                
            } else {
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                period = StrToInteger(StringSubstr(OrderComment(), str_open, str_close-str_open));
                
                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                recuo = StrToInteger(StringSubstr(OrderComment(), str_open, str_close-str_open));
                
                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                riskGain = StrToInteger(StringSubstr(OrderComment(), str_open, str_close-str_open));
                
                str_open = StringFind(OrderComment(), "[", str_close);
                str_open += 1;
                str_close = StringFind(OrderComment(), "]", str_open);
                patrimonio = StrToInteger(StringSubstr(OrderComment(), str_open, str_close-str_open));
                
                index = getIndexByPeriod(period);
                m_orderLots[index] = OrderLots();
                m_orderOpenPrice[index] = OrderOpenPrice();
                m_orderStopLoss[index] = OrderStopLoss();
                m_orderTakeProfit[index] = OrderTakeProfit();
                m_orderTicket[index] = OrderTicket();
                m_orderRiskGain[index] = riskGain;
                m_orderOpenTime[index] = OrderOpenTime();
                m_orderOpenBarShift[index] = iBarShift(Symbol(), period, m_orderOpenTime[index]);

                if (AccountBalance() < patrimonio) {
                    perc_gain = (AccountProfit() - (patrimonio - AccountBalance())) / patrimonio;
                } else {
                    perc_gain = AccountProfit() / AccountBalance();
                }

                // long position is opened
                if(OrderType() == OP_BUY) {
            
                    m_orderActive[index] = true;
                
                    m_orderStopLossGain[index] = ((m_orderStopLoss[index] - m_orderOpenPrice[index])) * m_orderLots[index] / Point;
                    m_orderStopLossGain[index] += OrderSwap();
                
                    m_orderTakeProfitGain[index] = ((m_orderTakeProfit[index] - m_orderOpenPrice[index]) / Point) * m_orderLots[index];

                    m_orderPercent[index] = (Bid - m_orderOpenPrice[index]) / (m_orderTakeProfit[index] - m_orderOpenPrice[index]);
                    orderPercent = (Bid - OrderOpenPrice()) / (OrderTakeProfit() - OrderOpenPrice()) * 100;
                    
                    stopLoss = wtsStopLossCalc(index, OP_BUY);
                
                    candlePrevClose = NormalizeDouble(iClose(Symbol(), period, 1), Digits-1);
                
                    threshold = NormalizeDouble((iLow(Symbol(), period, 1) - (m_candleShadowMed[index]) * Point), 5);
                
                    protection = NormalizeDouble((OrderOpenPrice()-OrderStopLoss()) + OrderOpenPrice(), Digits-1);
                    
                    orderParcialGain = (protection - OrderOpenPrice()) / Point * NormalizeDouble(OrderLots()/2, 2);
                    orderGain = (OrderTakeProfit() - OrderOpenPrice()) / Point * NormalizeDouble(OrderLots()/2, 2);
                    
                    wtsPrintLine("protection"+index, protection, Pink, "protection: $"+DoubleToStr(orderParcialGain, 2)+" ($"+DoubleToStr(orderGain, 2)+") "+DoubleToStr(orderPercent, 2)+"%");
                
                    if ((OrderStopLoss() < OrderOpenPrice()) && ((NormalizeDouble(Bid, Digits-1) >= protection))) {
                    
                        if (OrderLots() > 0.01) {
                            if (!OrderClose(OrderTicket(), NormalizeDouble(OrderLots()/2, 2), NormalizeDouble(Bid, Digits-1), 3, Blue)) {
                                Sleep(1000);
                            } else {
                                if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), OrderTakeProfit(), 0, Blue)) {
                                    Sleep(1000);
                                }                    
                            }
                        } else {
                            if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), OrderTakeProfit(), 0, Blue)) {
                                    Sleep(1000);
                            }
                        }
                    } else if (OrderStopLoss() >= OrderOpenPrice() && OrderLots() == 0.01) {
                        orderPercent55 = NormalizeDouble(OrderOpenPrice() + (OrderTakeProfit() - OrderOpenPrice()) * 0.50, Digits-1);
                        orderPercent85 = NormalizeDouble(OrderOpenPrice() + (OrderTakeProfit() - OrderOpenPrice()) * 0.80, Digits-1);
                    
                        if (OrderSwap() < 0) {
                           swapStop = - (OrderSwap() / OrderLots()) * Point;
                        }
                    
                        if (NormalizeDouble(Bid, Digits-1) >= orderPercent85 && orderPercent55 > OrderStopLoss()) {
                            if (!OrderModify(OrderTicket(), OrderOpenPrice(), orderPercent55, OrderTakeProfit(), 0, Yellow)) {
                                Sleep(1000);
                            }
                        } else if (OrderSwap() < 0 && Bid > OrderOpenPrice() && OrderStopLoss() >= OrderOpenPrice() && ((OrderStopLoss() - OrderOpenPrice()) < swapStop)) {
                            if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + swapStop, OrderTakeProfit(), 0, Green)) {
                                Sleep(1000);
                            }
                        } 
                        
                        orderParcialGain = (orderPercent55 - OrderOpenPrice()) / Point * OrderLots();
                        orderGain = (OrderTakeProfit() - OrderOpenPrice()) / Point * OrderLots();
                        orderPercent = (Bid - OrderOpenPrice()) / (OrderTakeProfit() - OrderOpenPrice()) * 100;
                        wtsPrintLine("prot start"+OrderTicket(), orderPercent85, Pink, "prot start");
                        wtsPrintLine("prot stop"+OrderTicket(), orderPercent55, Pink, "prot stop: $"+DoubleToStr(orderParcialGain, 2)+" ($"+DoubleToStr(orderGain, 2)+") "+DoubleToStr(orderPercent, 2)+"%");
                    }
/*                
                if (NormalizeDouble(Bid, Digits-1) > m_orderProtectionValuePrev[index]) {
                    m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
                    m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];
                }
                
                if ((m_orderStopLoss[index] < m_orderOpenPrice[index]) && ((Bid - m_orderOpenPrice[index]) > ((m_orderOpenPrice[index] - m_orderStopLoss[index])*2))) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], m_orderOpenPrice[index], m_orderTakeProfit[index], 0, Blue)) {
                        Sleep(1000);
                    } 
                } else if (stopLoss > m_orderStopLoss[index] && NormalizeDouble(Bid, Digits-1) == m_orderProtectionValuePrev[index]) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], stopLoss, m_orderTakeProfit[index], 0, Green)) {
                        Sleep(1000);
                    } else {
                        m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
                        m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];                    
                    }
                } else if ((m_orderPercent[index] >= 0.85) && threshold > m_orderStopLoss[index] && is_high_up_candle(period)) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], threshold, m_orderTakeProfit[index], 0, Yellow)) {
                        Sleep(1000);
                    }
                }
*/                
                
                } else if (OrderType() == OP_SELL) {

                    m_orderStopLossGain[index] = ((m_orderOpenPrice[index] - m_orderStopLoss[index])) * m_orderLots[index] / Point;
                    m_orderStopLossGain[index] += OrderSwap();
                
                    m_orderTakeProfitGain[index] = ((m_orderOpenPrice[index] - m_orderTakeProfit[index])) * m_orderLots[index] / Point;

                    m_orderActive[index] = true;
                    m_orderPercent[index] = (m_orderOpenPrice[index] - Ask) / (m_orderOpenPrice[index] - m_orderTakeProfit[index]);
                    orderPercent = (OrderOpenPrice() - Ask) / (OrderOpenPrice() - OrderTakeProfit()) * 100;
                    
                    stopLoss = wtsStopLossCalc(index, OP_SELL);
    
                    candlePrevClose = NormalizeDouble(iClose(Symbol(), period, 1), Digits-1);
                
                    threshold = NormalizeDouble((iHigh(Symbol(), period, 1) + (m_candleShadowMed[index] + spread) * Point), 5);

                    protection = NormalizeDouble(OrderOpenPrice() - (OrderStopLoss()-OrderOpenPrice()), Digits-1);
                    
                    orderParcialGain = (OrderOpenPrice() - protection) / Point * NormalizeDouble(OrderLots()/2, 2);
                    orderGain = (OrderOpenPrice() - OrderTakeProfit()) / Point * NormalizeDouble(OrderLots()/2, 2);
                    
                    wtsPrintLine("protection"+index, protection, Pink, "protection: $"+DoubleToStr(orderParcialGain, 2)+" ($"+DoubleToStr(orderGain, 2)+") "+DoubleToStr(orderPercent, 2)+"%");

                    if ((OrderStopLoss() > OrderOpenPrice()) && ((NormalizeDouble(Ask, Digits-1) <= protection))) {
                    
                        if (OrderLots() > 0.01) {
                            if (!OrderClose(OrderTicket(), NormalizeDouble(OrderLots()/2, 2), NormalizeDouble(Ask, Digits-1), 3, Blue)) {
                                Sleep(1000);
                            } else {
                                if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), OrderTakeProfit(), 0, Blue)) {
                                    Sleep(1000);
                                }                    
                            }
                        } else {
                            if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), OrderTakeProfit(), 0, Blue)) {
                                Sleep(1000);
                            }   
                        }
                    } else if (OrderStopLoss() <= OrderOpenPrice() && OrderLots() == 0.01) {
                        orderPercent55 = NormalizeDouble(OrderOpenPrice() - (OrderOpenPrice() - OrderTakeProfit()) * 0.50, Digits-1);
                        orderPercent85 = NormalizeDouble(OrderOpenPrice() - (OrderOpenPrice() - OrderTakeProfit()) * 0.80, Digits-1);
                    
                        if (OrderSwap() < 0) {
                           swapStop = - (OrderSwap() / OrderLots()) * Point;
                        }
                    
                        if (NormalizeDouble(Ask, Digits-1) <= orderPercent85 && orderPercent55 < OrderStopLoss()) {
                            if (!OrderModify(OrderTicket(), OrderOpenPrice(), orderPercent55, OrderTakeProfit(), 0, Yellow)) {
                                Sleep(1000);
                            }
                        } else if (OrderSwap() < 0 && Ask < OrderOpenPrice() && OrderStopLoss() <= OrderOpenPrice() && ((OrderOpenPrice() - OrderStopLoss()) < swapStop)) {
                            if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - swapStop, OrderTakeProfit(), 0, Green)) {
                                Sleep(1000);
                            }
                        }
                    
                        orderParcialGain = (OrderOpenPrice() - orderPercent55) / Point * OrderLots();
                        orderGain = (OrderOpenPrice() - OrderTakeProfit()) / Point * OrderLots();
                        orderPercent = (OrderOpenPrice() - Ask) / (OrderOpenPrice() - OrderTakeProfit()) * 100;
                        wtsPrintLine("prot start"+OrderTicket(), orderPercent85, Pink, "prot start");
                        wtsPrintLine("prot stop"+OrderTicket(), orderPercent55, Pink, "prot stop: $"+DoubleToStr(orderParcialGain, 2)+" ($"+DoubleToStr(orderGain, 2)+") "+DoubleToStr(orderPercent, 2)+"%");
                    }
/*                
                if (NormalizeDouble(Ask, Digits-1) < m_orderProtectionValuePrev[index]) {
                    m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
                    m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];
                }
                
                if ((m_orderStopLoss[index] > m_orderOpenPrice[index]) && ((m_orderOpenPrice[index] - Ask) > ((m_orderStopLoss[index] - m_orderOpenPrice[index])*2))) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], m_orderOpenPrice[index], m_orderTakeProfit[index], 0, Blue)) {
                        Sleep(1000);
                    }
                } else if (stopLoss < m_orderStopLoss[index] && NormalizeDouble(Ask, Digits-1) == m_orderProtectionValuePrev[index] && stopLoss != 0) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], stopLoss, m_orderTakeProfit[index], 0, Green)) {
                        Sleep(1000);
                    } else {
                        m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
                        m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];                    
                    }
                } else if ((m_orderPercent[index] >= 0.85) && threshold < m_orderStopLoss[index] && is_low_down_candle(period)) {
                    if (!OrderModify(m_orderTicket[index], m_orderOpenPrice[index], threshold, m_orderTakeProfit[index], 0, Yellow)) {
                        Sleep(1000);
                    }
                }
*/                
                }
            }
        }
    }
}

bool is_high_up_candle(int period)
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

bool is_low_down_candle(int period)
{
    bool cond = false;
    double close = 0;
    double high = 0;
    double open = 0;
    double prev_low = 0;
    double prev_high = 0;
    
    close = iClose(Symbol(), period, 1);
    open = iOpen(Symbol(), period, 1);
    high = iHigh(Symbol(), period, 1);
    
    prev_low = iLow(Symbol(), period, 2);
    prev_high = iHigh(Symbol(), period, 2);
    
    if (close < open && close < prev_low && high < prev_high) {
        cond = true;
    }
    
    return (cond);
}

bool wtsCandleIsHigh(int period, int shift)
{
    bool cond = false;
    double close = 0;
    double open = 0;
    
    close = iClose(Symbol(), period, shift);
    open = iOpen(Symbol(), period, shift);
    
    if (close >= open) {
        cond = true;
    }
    
    return (cond);
}

bool wtsCandleIsLow(int period, int shift)
{
    bool cond = false;
    double close = 0;
    double open = 0;
    
    close = iClose(Symbol(), period, shift);
    open = iOpen(Symbol(), period, shift);
    
    if (close <= open) {
        cond = true;
    }
    
    return (cond);
}


int wtsNormalTrade(int index)
{
    int period = wtsGetPeriodByIndex(index);
    int spread = MarketInfo(Symbol(), MODE_SPREAD);
    int candleStop = 0;
    int candleStart = 0;
    bool tradeOk = false;
    double gain = 0;
    double loss = 0;
    double candleHigh = 0;
    double candleHighOpt = 0;
    double candleLow = 0;
    double candleLowOpt = 0;
    double candleHighNext = 0;
    double candleLowNext = 0;
    string comment = "";
    double risk = 0;
    
    m_normalTradeType[index] = 0;

    RefreshRates();

    switch (m_normalState[index]) {
        case ST_UP:
       
            m_candleLowestRetration[index] = iLowest(Symbol(), period, MODE_LOW, m_candleLastBarShift[index]);

            m_candleHighestAfterLowest[index] = iHighest(Symbol(), period, MODE_HIGH, m_candleLowestRetration[index]+1);

            candleLow = iLow(Symbol(), period, m_candleLowestRetration[index]);
            candleHigh = iHigh(Symbol(), period, m_candleLowestRetration[index]);
            
            candleStop = m_candleLowestRetration[index];
            
            if (m_candleLowestRetration[index] != m_candleHighestAfterLowest[index] && (iHigh(Symbol(), period, m_candleHighestAfterLowest[index]) < candleHigh + m_candleShadowMed[index]*Point) || m_candleHighestAfterLowest[index] == 0) {
                candleStart = candleStop;
            } else {
                candleStart = m_candleHighestAfterLowest[index];
            }

            if (candleStart == candleStop) {
            
                tradeOk = true;
                m_normalTradeType[index] = 99;
                
/*                
                if (candleStop > 2 && wtsCandleIsHigh(period, 1)) {
                    candleHighNext = iHigh(Symbol(), period, 1);
            
                    if (candleHighNext < candleHigh) {
                        candleHigh = candleHighNext;
                        m_normalTradeType[index] = 77;
                    }
                } else if (candleStop > 1 && wtsCandleIsHigh(period, candleStop-1)) {
                    candleHighNext = iHigh(Symbol(), period, candleStop-1);
                
                    if (candleHighNext < candleHigh) {
                        m_normalTradeType[index] = 88;
                        candleHigh = candleHighNext;
                    }
                }
*/
            } /*else if (zzReturn[index] != 0 && wtsCandleIsHigh(period, 1)) {
                tradeOk = true;
                m_normalTradeType[index] = 88;
            }*/        
            
            m_normalTradeStart[index] = NormalizeDouble(candleHigh + 30*m_normalTradeStartMultiply[index]*Point, Digits-1);
            m_normalTradeStop[index] = NormalizeDouble(candleLow - 30*m_normalTradeStartMultiply[index]*Point, Digits-1);
            m_normalTradeRisk[index] = (m_normalTradeStart[index] - m_normalTradeStop[index]) / Point;
            
            if (/*((wtsCandleIsHigh(period, candleStop) && candleStop > 0) || */
                 /* (wtsCandleIsHigh(period, candleStop-1) && candleStop > 1) || */
                 /*(wtsCandleIsHigh(period, 1) && candleStop > 2)) &&*/ ((tradeOk == true && candleStop > 0) /*|| (candleStart == 0 && wtsCandleIsHigh(period, 0))*/)) {
                
                
                if ((candleHigh >= m_candleFibo60[index] && candleLow <= m_candleFibo60[index]) || 
                    (candleHigh >= m_candleFibo50[index] && candleLow <= m_candleFibo50[index]) ||
                    (candleHigh >= m_candleFibo33[index] && candleLow <= m_candleFibo33[index]) ||
                    (candleHigh <= m_candleFibo33[index] && candleLow >= m_candleFibo50[index]/* && wtsCandleIsHigh(period, candleStop)*/) ||
                    (candleHigh <= m_candleFibo50[index] && candleLow >= m_candleFibo60[index]/* && wtsCandleIsHigh(period, candleStop)*/)) {
/*
                    if (wtsCandleIsHigh(period, 1) && candleStop > 1) {
                        candleHighNext = iHigh(Symbol(), period, 1);
            
                        if (candleHighNext < candleHigh && Bid < (candleHighNext + 30*m_normalTradeStartMultiply[index]*Point)) {
                            candleHigh = candleHighNext;
                            m_normalTradeType[index] = 77;
                        }
                    }
*/
                    //m_normalTradeStart[index] = NormalizeDouble(candleHigh + 30*m_normalTradeStartMultiply[index]*Point, Digits-1);
                    //m_normalTradeStop[index] = NormalizeDouble(candleLow - 30*m_normalTradeStartMultiply[index]*Point, Digits-1);
                    //m_normalTradeRisk[index] = (m_normalTradeStart[index] - m_normalTradeStop[index]) / Point;
                    
                    risk = getRisk(index);
                    
                    m_normalTradeLots[index] = NormalizeDouble((AccountBalance() * risk) / (m_normalTradeRisk[index] + spread), 2);

                    if (m_normalTradeLots[index] < 0.01 && AccountBalance() <= 100) {
                        m_normalTradeLots[index] = 0.01;
                    }
/* Para debug                    
                    comment = "Lots("+ DoubleToStr(m_normalTradeLots[index], 2) + 
                              " "+ DoubleToStr(AccountBalance(), 2) + 
                              " "+ DoubleToStr(getRisk(index), 2) + 
                              " "+ DoubleToStr(m_normalTradeRisk[index] + spread, 0);
                    m_normalTradeRiskGain[index] = 4;
                    wtsPrintLine("apagar", index, candleHigh, Blue, comment);
*/                    
                    m_normalTradeTakeProfit[index] = NormalizeDouble(m_normalTradeStop[index] + (m_candleLeg[index] - spread) * Point, Digits-1);
                    
                    gain = (m_normalTradeTakeProfit[index] - m_normalTradeStart[index]) * m_normalTradeLots[index] / Point;
                    
                    loss = m_normalTradeRisk[index] * m_normalTradeLots[index];
                    
                    if (loss != 0) {
                        m_normalTradeRiskGain[index] = gain / loss;
                    } else {
                        m_normalTradeRiskGain[index] = -2;
                    }
                   
                    if (iClose(Symbol(), period, 1) > m_normalTradeStart[index]) {
                        m_normalTradeRiskGain[index] = -1;
                    }
                   
                    comment = "" + wtsGetPeriodNameByIndex(index) +": " +
                              "" + DoubleToStr(m_normalTradeLots[index], 2) +" | "+
                              "$-" + DoubleToStr(loss, 2) +" | "+
                              "$" + DoubleToStr(gain, 2) +" | "+
                              "" + DoubleToStr(m_normalTradeRiskGain[index], 1) +" | "+
                              "" + wtsNormalTradeValidDecision(index) +" | "+
                              "" + m_normalTradeRisk[index] +" | "+
                              "" + m_normalTradeType[index];

                } else {
                    //m_normalState[index] = ST_UNDEF;
                    m_normalTradeRiskGain[index] = -3;
                }
            } else {
                //m_normalState[index] = ST_UNDEF;
                m_normalTradeRiskGain[index] = -4;
            }

            break;
 
        case ST_DOWN:
            
            m_candleHighestRetration[index] = iHighest(Symbol(), period, MODE_HIGH, m_candleLastBarShift[index]);
            
            m_candleLowestAfterHighest[index] = iLowest(Symbol(), period, MODE_LOW, m_candleHighestRetration[index]+1);

            candleLow = iLow(Symbol(), period, m_candleHighestRetration[index]);
            candleHigh = iHigh(Symbol(), period, m_candleHighestRetration[index]);
            
            candleStop = m_candleHighestRetration[index];
            
            if (m_candleHighestRetration[index] != m_candleLowestAfterHighest[index] && (iLow(Symbol(), period, m_candleLowestAfterHighest[index]) > (candleLow - m_candleShadowMed[index]*Point) || m_candleLowestAfterHighest[index] == 0)) {
                candleStart = candleStop;
            } else {
                candleStart = m_candleLowestAfterHighest[index];
            }

            if (candleStart == candleStop) {
            
                tradeOk = true;
                m_normalTradeType[index] = 99;

/*                
                if (candleStop > 2 && wtsCandleIsLow(period, 1)) {
                    candleLowNext = iLow(Symbol(), period, 1);
            
                    if (candleLowNext > candleLow) {
                        candleLow = candleLowNext;
                        m_normalTradeType[index] = 77;
                    }
                } else if (candleStop > 1 && wtsCandleIsLow(period, candleStop-1)) {
                    candleLowNext = iLow(Symbol(), period, candleStop-1);
            
                    if (candleLowNext > candleLow) {
                        candleLow = candleLowNext;
                        m_normalTradeType[index] = 88;
                    }
                }
*/
            } /*else if (zzReturn[index] != 0 && wtsCandleIsLow(period, 1)) {
                tradeOk = true;
                m_normalTradeType[index] = 88;
            }*/ 

            m_normalTradeStop[index] = NormalizeDouble(candleHigh + (30 + spread)*Point, Digits-1);
            m_normalTradeStart[index] = NormalizeDouble(candleLow - (30 - spread)*Point, Digits-1);
            m_normalTradeRisk[index] = (m_normalTradeStop[index] - m_normalTradeStart[index]) / Point;

            if (/*((wtsCandleIsLow(period, candleStop) && candleStop > 0) || 
                 (wtsCandleIsLow(period, candleStop-1) && candleStop > 1) ||
                 (wtsCandleIsLow(period, 1) && candleStop > 2)) && */((tradeOk == true && candleStop > 0)/*|| (candleStart == 0/* && wtsCandleIsLow(period, 0))*/)) {

                if ((candleHigh >= m_candleFibo60[index] && candleLow <= m_candleFibo60[index]) || 
                    (candleHigh >= m_candleFibo50[index] && candleLow <= m_candleFibo50[index]) ||
                    (candleHigh >= m_candleFibo33[index] && candleLow <= m_candleFibo33[index]) ||
                    (candleHigh <= m_candleFibo50[index] && candleLow >= m_candleFibo33[index]/* && wtsCandleIsLow(period, candleStop)*/) ||
                    (candleHigh <= m_candleFibo60[index] && candleLow >= m_candleFibo50[index]/* && wtsCandleIsLow(period, candleStop)*/)) {
/*
                    if (wtsCandleIsLow(period, 1) && candleStop > 1) {
                        candleLowNext = iLow(Symbol(), period, 1);
            
                        if (candleLowNext > candleLow && Ask > (candleLowNext + 30*m_normalTradeStartMultiply[index]*Point)) {
                            candleLow = candleLowNext;
                            m_normalTradeType[index] = 77;
                        }
                    }
*/
                    m_normalTradeStop[index] = NormalizeDouble(candleHigh + (30 + spread)*Point, Digits-1);
                    m_normalTradeStart[index] = NormalizeDouble(candleLow - (30 - spread)*Point, Digits-1);
                    m_normalTradeRisk[index] = (m_normalTradeStop[index] - m_normalTradeStart[index]) / Point;
                    
                    risk = getRisk(index);
                    
                    m_normalTradeLots[index] = NormalizeDouble((AccountBalance() * risk) / (m_normalTradeRisk[index] + spread), 2);
                     
                    if (m_normalTradeLots[index] < 0.01 && AccountBalance() <= 100) {
                        m_normalTradeLots[index] = 0.01;
                    }
                                         
                    m_normalTradeTakeProfit[index] = NormalizeDouble(m_normalTradeStop[index] - (m_candleLeg[index] - spread) * Point, Digits-1);
                    
                    gain = (m_normalTradeStart[index] - m_normalTradeTakeProfit[index]) * m_normalTradeLots[index] / Point;

                    loss = m_normalTradeRisk[index] * m_normalTradeLots[index];
                    
                    if (loss != 0) {
                        m_normalTradeRiskGain[index] = gain / loss;
                    } else {
                        m_normalTradeRiskGain[index] = -2;
                    }
                   
                    if ((iClose(Symbol(), period, 1) + spread * Point) < m_normalTradeStart[index]) {
                        m_normalTradeRiskGain[index] = -1;
                    }
                    
                    comment = "" + wtsGetPeriodNameByIndex(index) +": " +
                              "" + DoubleToStr(m_normalTradeLots[index], 2) +" | "+
                              "-$" + DoubleToStr(loss, 2) +" | "+
                              "$" + DoubleToStr(gain, 2) +" | "+
                              "" + DoubleToStr(m_normalTradeRiskGain[index], 1) +" | "+
                              "" + wtsNormalTradeValidDecision(index) +" | "+
                              "" + m_normalTradeRisk[index] +" | "+
                              "" + m_normalTradeType[index];
                              
                } else {
                    //m_normalState[index] = ST_UNDEF;
                    m_normalTradeRiskGain[index] = -3;
                }
            } else {
                //m_normalState[index] = ST_UNDEF;
                m_normalTradeRiskGain[index] = -4;
            }
            
            break;
    
        default:
            break; 
    }       
    
    m_normalTradeRiskGain[index] = NormalizeDouble(m_normalTradeRiskGain[index], 1);
    
    if ((m_normalTradeRiskGain[index] < m_normalTradeRiskGainMin[index]+1) && m_normalTradeType[index] == 77) {
        //fix-me
        //m_normalTradeRiskGain[index] -= 1.0;
    }
    
    wtsPrintLineTrade("wts_normalTradeStart", index, m_normalTradeStart[index], Green, comment, wtsNormalTradeValidDecision(index));
    wtsPrintLineTrade("wts_normalTradeStop", index, m_normalTradeStop[index], Red, "", wtsNormalTradeValidDecision(index));

    if (m_normalState[index] == ST_UNDEF || m_normalTradeRiskGain[index] < m_normalTradeRiskGainMin[index]) {
        m_normalTradeStart[index] = 0;
        m_normalTradeStop[index] = 0;
    } 

    wtsSetCandleFibo(index);
    
    return (0);
}

double wtsStopLossCalc(int index, int op_type) 
{
    int i = 0;
    double perc = 0;
    double prot = 0;
    double stopLoss = 0;
    double range = 0.1;
    
    perc = NormalizeDouble(m_orderPercent[index], 3);
    
    for (i = 0; i <= 50; i++) {
        
        stopLoss = wtsGetStopLossFibo(index, op_type, range);
    
        m_orderPercentToProtectionPrev[index] = prot;
        m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
    
        prot = NormalizeDouble(range*10/m_orderRiskGain[index], 3);

        m_orderPercentToProtection[index] = prot;
        
        switch (op_type) {
            case OP_BUY:
                m_orderProtectionValue[index] = NormalizeDouble(m_orderOpenPrice[index] + ((m_orderTakeProfit[index] - m_orderOpenPrice[index]) * m_orderPercentToProtection[index]), Digits-1);
                break;
            
            case OP_SELL:
                m_orderProtectionValue[index] = NormalizeDouble(m_orderOpenPrice[index] - ((m_orderOpenPrice[index] - m_orderTakeProfit[index]) * m_orderPercentToProtection[index]), Digits-1);
                break;
            
            default:
                m_orderProtectionValue[index] = 0;
                break;
        }
            
        //primeira amostra de protecao
        if (range == 0.1) {
            m_orderProtectionValuePrev[index] = m_orderProtectionValue[index];
            m_orderPercentToProtectionPrev[index] = m_orderPercentToProtection[index];
        }

        if (perc < prot) {
            break;
        }
        
        range += 0.1;
    }

    //wtsPrintProtectionLine(index);
    
    return (stopLoss);
}

double wtsGetStopLossFibo(int index, int op_type, double range)
{
    double leg = 0;
    double stop = 0;
                
    switch (op_type) {
        case OP_BUY:
            leg = Bid - m_orderOpenPrice[index];
        
            switch (NormalizeDouble(range, 1)) {
                case 0.1: stop = NormalizeDouble(m_orderOpenPrice[index] + (leg * 0.03), 5); break;
                case 0.2: stop = NormalizeDouble(m_orderOpenPrice[index] + (leg * 0.05), 5); break;
                case 0.3: stop = NormalizeDouble(m_orderOpenPrice[index] + (leg * 0.08), 5); break;
                case 0.4: stop = NormalizeDouble(m_orderOpenPrice[index] + (leg * 0.13), 5); break;
                case 0.5: stop = NormalizeDouble(m_orderOpenPrice[index] + (leg * 0.21), 5); break;
                case 0.6: stop = NormalizeDouble(m_orderOpenPrice[index] + (leg * 0.34), 5); break;
                case 0.7: stop = NormalizeDouble(m_orderOpenPrice[index] + (leg * 0.55), 5); break;

                default:
                    stop = NormalizeDouble(m_orderOpenPrice[index] + (leg * 0.55), 5); break;
            }
            
            break;
        
        case OP_SELL:
            leg = m_orderOpenPrice[index] - Ask;
                
            switch (range) {
                case 0.1: stop = NormalizeDouble(m_orderOpenPrice[index] - (leg * 0.03), 5); break;
                case 0.2: stop = NormalizeDouble(m_orderOpenPrice[index] - (leg * 0.05), 5); break;
                case 0.3: stop = NormalizeDouble(m_orderOpenPrice[index] - (leg * 0.08), 5); break;
                case 0.4: stop = NormalizeDouble(m_orderOpenPrice[index] - (leg * 0.13), 5); break;
                case 0.5: stop = NormalizeDouble(m_orderOpenPrice[index] - (leg * 0.21), 5); break;
                case 0.6: stop = NormalizeDouble(m_orderOpenPrice[index] - (leg * 0.34), 5); break;
                case 0.7: stop = NormalizeDouble(m_orderOpenPrice[index] - (leg * 0.55), 5); break;
            }
        
            break;
            
        default:
            stop = NormalizeDouble(m_orderOpenPrice[index] - (leg * 0.55), 5); break;
    }

    return (stop);
}

int wtsPrintProtectionLine(int index)
{
    int period = wtsGetPeriodByIndex(index);
    
    ObjectDelete(WTS_PROTECTION_VALUE + period);
    ObjectCreate(WTS_PROTECTION_VALUE + period, OBJ_HLINE, 0, 0, m_orderProtectionValue[index]);
    ObjectSet(WTS_PROTECTION_VALUE + period, OBJPROP_COLOR, Pink); 
    ObjectSet(WTS_PROTECTION_VALUE + period, OBJPROP_STYLE, 2);
    ObjectSetText(WTS_PROTECTION_VALUE + period, 
                  "Next Protection "+wtsGetPeriodNameByIndex(index)+" ["+DoubleToStr(m_orderPercentToProtection[index]*100, 0)+"%]"+
                  " "+DoubleToStr(m_orderStopLossGain[index], 2),
                  7, "Arial", White);

    ObjectDelete(WTS_PROTECTION_VALUE_PREV + period);
    
    if (m_orderProtectionValue[index] != m_orderProtectionValuePrev[index]) {
        ObjectCreate(WTS_PROTECTION_VALUE_PREV + period, OBJ_HLINE, 0, 0, m_orderProtectionValuePrev[index]);
        ObjectSet(WTS_PROTECTION_VALUE_PREV + period, OBJPROP_COLOR, Pink); 
        ObjectSet(WTS_PROTECTION_VALUE_PREV + period, OBJPROP_STYLE, 2);
        ObjectSetText(WTS_PROTECTION_VALUE_PREV + period, 
                      "Protection "+wtsGetPeriodNameByIndex(index)+" ["+DoubleToStr(m_orderPercentToProtectionPrev[index]*100, 0)+"%]"+
                      " "+DoubleToStr(m_orderStopLossGain[index], 2), 7, "Arial", White);
    }
    return (0);
}

int wtsPrintLine(string name, double value, int colored, string comment)
{
    ObjectDelete(name);

    if (value > 0) {
        ObjectCreate(name, OBJ_HLINE, 0, 0, value);
        ObjectSet(name, OBJPROP_COLOR, colored); 
        ObjectSet(name, OBJPROP_STYLE, 2);
        ObjectSetText(name, comment, 7, "Arial", White);
    }
    return (0);
}

int wtsPrintLineTrade(string name, int index, double value, int colored, string comment, int decision)
{
    int period = wtsGetPeriodByIndex(index);
    
    ObjectDelete(name + period);

    if (value > 0 && m_orderActive[index] == false && decision == 1) {
        ObjectCreate(name + period, OBJ_HLINE, 0, 0, value);
        ObjectSet(name + period, OBJPROP_COLOR, colored); 
        ObjectSet(name + period, OBJPROP_STYLE, 2);
        ObjectSetText(name + period, comment, 7, "Arial", White);
    }
    return (0);
}

int wtsPrintTrend(string name, int index, double value, double initTime)
{
    int period = wtsGetPeriodByIndex(index);
    
    ObjectDelete(name + wtsGetPeriodNameByIndex(index));

    if (value > 0 && index == getIndexByPeriod(Period())) {
        ObjectCreate(name + wtsGetPeriodNameByIndex(index), OBJ_TREND, 0, 0, value);
        ObjectSet(name + wtsGetPeriodNameByIndex(index), OBJPROP_COLOR, PaleTurquoise); 
        ObjectSet(name + wtsGetPeriodNameByIndex(index), OBJPROP_FONTSIZE, 7); 
        ObjectSet(name + wtsGetPeriodNameByIndex(index), OBJPROP_TIME1, initTime);
        ObjectSet(name + wtsGetPeriodNameByIndex(index), OBJPROP_TIME2, iTime(Symbol(), wtsGetPeriodByIndex(index), 0));
        ObjectSet(name + wtsGetPeriodNameByIndex(index), OBJPROP_PRICE1, value);
        ObjectSet(name + wtsGetPeriodNameByIndex(index), OBJPROP_PRICE2, value);
        ObjectSet(name + wtsGetPeriodNameByIndex(index), OBJPROP_STYLE, 2);
    }
    return (0);
}

int wtsPrintTrendFull(string name, int index, double price1, double time1, double price2, double time2, int colored)
{
    int period = wtsGetPeriodByIndex(index);
    
    //if (index == getIndexByPeriod(Period())) {
        ObjectDelete(name);
        ObjectCreate(name, OBJ_TREND, 0, 0, price2);
        ObjectSet(name, OBJPROP_COLOR, colored); 
        ObjectSet(name, OBJPROP_FONTSIZE, 7); 
        ObjectSet(name, OBJPROP_TIME1, time1);
        ObjectSet(name, OBJPROP_TIME2, time2);
        ObjectSet(name, OBJPROP_PRICE1, price1);
        ObjectSet(name, OBJPROP_PRICE2, price2);
//        ObjectSet(name + wtsGetPeriodNameByIndex(index), OBJPROP_STYLE, 2);
    //}
    return (0);
}

int wtsSetCandleFibo(int index) 
{
    double initTime = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    
    if (m_normalState[index] == ST_UP) {
        m_candleFibo33[index] = m_candleTop[index] - (m_candleLeg[index] * 0.33 * Point);
        m_candleFibo50[index] = m_candleTop[index] - (m_candleLeg[index] * 0.50 * Point);
        m_candleFibo60[index] = m_candleTop[index] - (m_candleLeg[index] * 0.60 * Point);
        initTime = m_candleBottomTime[index];

    } else if (m_normalState[index] == ST_DOWN) {
        m_candleFibo33[index] = m_candleBottom[index] + ((m_candleLeg[index] * 0.33) * Point);
        m_candleFibo50[index] = m_candleBottom[index] + ((m_candleLeg[index] * 0.50) * Point);
        m_candleFibo60[index] = m_candleBottom[index] + ((m_candleLeg[index] * 0.60) * Point);
        initTime = m_candleTopTime[index];

    } else {
        m_candleFibo33[index] = 0;
        m_candleFibo50[index] = 0;
        m_candleFibo60[index] = 0;
    }

    wtsPrintTrend("Fibo33", index, m_candleFibo33[index], initTime);
    wtsPrintTrend("Fibo50", index, m_candleFibo50[index], initTime);
    wtsPrintTrend("Fibo60", index, m_candleFibo60[index], initTime);

    return (0);
}

int wtsGetPeriodPrev(int period) 
{
    int prev = PERIOD_H1;
    
    switch (period) {
        case PERIOD_MN1: prev = PERIOD_W1; break;
        case PERIOD_W1:  prev = PERIOD_D1; break;
        case PERIOD_D1:  prev = PERIOD_H4; break;
        case PERIOD_H4:  prev = PERIOD_H1; break;
        case PERIOD_H1:  prev = PERIOD_M30; break;
        case PERIOD_M15:  prev = PERIOD_M5; break;
        case PERIOD_M5:  prev = PERIOD_M5; break;
    }
    
    return (prev);
}

void wtsPrintLT(int index)
{
    int period = wtsGetPeriodByIndex(index);
    double lta = 0;
    double bottom = 0;
    double bottomTime = 0;
    double bottomShift = 0;
    double top = 0;
    double topTime = 0;
    double topShift = 0;
    string comment = "";

    switch (m_normalState[index]) {
    case ST_UP:
        
        if (m_candleBottom[index] != m_candleBottomPrev[index]) {
            bottom = m_candleBottom[index];
            bottomTime = m_candleBottomTime[index];
            bottomShift = m_candleBottomShift[index] - 1;
            wtsPrintTrendFull("LT", index, m_candleBottomPrev[index], m_candleBottomTimePrev[index], bottom, bottomTime, Gold);
        } else {
            bottom = iLow(Symbol(), period, m_candleLowestRetration[index]);
            bottomTime = iTime(Symbol(), period, m_candleLowestRetration[index]);
            bottomShift = m_candleLowestRetration[index] - 1;
            wtsPrintTrendFull("LT", index, m_candleBottomPrev[index], m_candleBottomTimePrev[index], bottom, bottomTime, Blue);
        }
        
        if ((m_candleBottomShiftPrev[index] - bottomShift) > 0) {
            lta = ((bottom - m_candleBottomPrev[index]) / (m_candleBottomShiftPrev[index] - bottomShift)) * bottomShift + bottom;
        }
        
        comment = "LTA_"+wtsGetPeriodNameByIndex(index);

        wtsPrintLine("LTVAL1"+index, lta-30*m_normalTradeStartMultiply[index]*Point, Purple, comment);
        break;
        
    case ST_DOWN:
        if (m_candleTop[index] != m_candleTopPrev[index]) {
            top = m_candleTop[index];
            topTime = m_candleTopTime[index];
            topShift = m_candleTopShift[index];
            wtsPrintTrendFull("LT", index, m_candleTopPrev[index], m_candleTopTimePrev[index], top, topTime, Gold);
        } else {
            top = iHigh(Symbol(), period, m_candleHighestRetration[index]);
            topTime = iTime(Symbol(), period, m_candleHighestRetration[index]);
            topShift = m_candleHighestRetration[index];
            wtsPrintTrendFull("LT", index, m_candleTopPrev[index], m_candleTopTimePrev[index], top, topTime, Blue);
        }
        
        if ((m_candleTopShiftPrev[index] - topShift) > 0) {
            lta = top - ((m_candleTopPrev[index] - top) / (m_candleTopShiftPrev[index] - topShift)) * topShift;
        }
        
        comment = "LTB_"+wtsGetPeriodNameByIndex(index);

        wtsPrintLine("LTVAL1"+index, lta+30*m_normalTradeStartMultiply[index]*Point, Purple, comment);

        break;
        
    default:
        ObjectDelete("LT");
        ObjectDelete("LTVAL1"+index);
        break;
    }
}

