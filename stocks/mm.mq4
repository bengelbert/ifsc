//+------------------------------------------------------------------+
//|                                                           mm.mq4 |
//|                        Copyright 2014, MetaQuotes Software Corp. |
//|                                              http://www.mql5.com |
//+------------------------------------------------------------------+
#include <stderror.mqh>
#include <stdlib.mqh>

#property copyright "Copyright 2014, MetaQuotes Software Corp."
#property link      "http://www.mql5.com"
#property version   "1.00"
#property strict

#define MM_TOTAL_PERIODS    5
#define MM_TOTAL_MEDIA      9
#define MM_TOTAL_SHIFT      24
#define MM_MAX_GAIN_FACTOR  5

static int mmMagicNumber = 8855;
static int mmStartBar[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static int mmShift[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static int mmStopLoss[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static int mmTimeFrame[MM_TOTAL_PERIODS] = {PERIOD_H1,PERIOD_H4,PERIOD_D1,PERIOD_W1,PERIOD_MN1};

static bool mmMarketInside[MM_TOTAL_PERIODS] = {true,true,true,true,true};

static double mmLots[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static double mmStart[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static double mmStop[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static double mmIA1[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static double mmIA2[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static double mmTrendStopLow[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static double mmTrendStopHigh[MM_TOTAL_PERIODS] = {0,0,0,0,0};
static double mmRisk[MM_TOTAL_PERIODS] = {0.01, 0.02, 0.04, 0.08, 0.16};
static double mmGainMax[MM_TOTAL_PERIODS] = {0,0,0,0,0};

static string mmTrend[MM_TOTAL_PERIODS] = {"--","--","--","--","--"};
static string mmTimeFrameName[MM_TOTAL_PERIODS] = {"H1","H4","D1","W1","MN1"};

//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
{
    //ObjectsDeleteAll();

    ObjectCreate("result", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("result", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("result", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("result", OBJPROP_YDISTANCE, 15); // Y coordinate
    
    mm_start();
        
    return(0);
}
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
  {
//---
   
  }
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
{
    mm_start();
    
    return;
}
//+------------------------------------------------------------------+

double mm_calc_variance(int index, double med, int shift)
{
    int i;
    double var = 0;
    double tmp = 0;
    
    for (i = 0; i < MM_TOTAL_MEDIA; i++) {
        tmp = (iClose(Symbol(), mmTimeFrame[index], i+shift) - med) / Point;
        var += tmp * tmp;
    }
    
    var /= MM_TOTAL_MEDIA;
    var *= Point;
    
    return (var);
}

//+------------------------------------------------------------------+

static int start_time = 0;

void mm_start()
{
    int index = 0;
    int shift = 0;
    
    for (index = 0; index < MM_TOTAL_PERIODS; index++) {
        
        mm_calc_ia(index, 0);
    
        if (mmTimeFrame[index] == Period()) {
            ObjectSetText("result",
                          ""+DoubleToStr(mmIA2[index],2)+"::"+DoubleToStr(mmIA1[index], 2)+":::"+DoubleToStr(mmGainMax[index], 2),
                          7, "Arial", White);
        }
    
        for (shift = 0; shift < MM_TOTAL_SHIFT; shift++) {
            if (mmStart[index] == 0) {
                mm_calc_ia(index, shift+1);
            }
        }
        
        if (mmMarketInside[index] == false) {
            mm_market_enter(index);        
        } else {
            mm_market_exit(index);
        }
    }
    
    return;    
}

//+------------------------------------------------------------------+

int mm_calc_ia(int index, int shift)
{
    int lowest = 0;
    int highest = 0;
    int spread = MarketInfo(Symbol(), MODE_SPREAD);
    double med_1 = 0;
    double med_2 = 0;
    double med_3 = 0;
    double var_1 = 0;
    double var_2 = 0;
    double var_3 = 0;

    med_3 = iMA(Symbol(), mmTimeFrame[index], MM_TOTAL_MEDIA, 0, MODE_SMA, PRICE_CLOSE, 3+shift);
    med_2 = iMA(Symbol(), mmTimeFrame[index], MM_TOTAL_MEDIA, 0, MODE_SMA, PRICE_CLOSE, 2+shift);
    med_1 = iMA(Symbol(), mmTimeFrame[index], MM_TOTAL_MEDIA, 0, MODE_SMA, PRICE_CLOSE, 1+shift);
    
    var_3 = mm_calc_variance(index, med_3, 3+shift);
    var_2 = mm_calc_variance(index, med_2, 2+shift);
    var_1 = mm_calc_variance(index, med_1, 1+shift);

    mmIA2[index] = var_2 / var_3 * 100;
    mmIA1[index] = var_1 / var_2 * 100;

    mmStart[index] = 0;
    mmStop[index] = 0;
    mmTrend[index] = "--";

    //if (mmStartBar[index] != iBarShift(Symbol(), mmTimeFrame[index], iTime(Symbol(), mmTimeFrame[index], 0+shift))) {
    if (mmIA2[index] > 0 && mmIA1[index] > 0) {
        if (mmIA1[index] >= 200) {
            //mmStartBar[index] = iBarShift(Symbol(), mmTimeFrame[index], iTime(Symbol(), mmTimeFrame[index], 0+shift));
            
            if (mm_is_high_clande(mmTimeFrame[index], 1+shift)) {
                mmStart[index] = NormalizeDouble(iHigh(Symbol(), mmTimeFrame[index], 1+shift) + 30 * Point, 4);
                mmStop[index] = NormalizeDouble(iLow(Symbol(), mmTimeFrame[index], 1+shift) - 30 * Point, 4);
                mmStopLoss[index] = (mmStart[index] - mmStop[index]) / Point;
                mmTrend[index] = "up";
                
                highest = iHighest(Symbol(), mmTimeFrame[index], MODE_HIGH, shift+1, 1);
                
                if (NormalizeDouble(iHigh(Symbol(), mmTimeFrame[index], highest), 4) >= mmStart[index] || mm_next_period_is_down(index) || index == (MM_TOTAL_PERIODS-1)) {
                    mmStart[index] = 0;
                    mmStop[index] = 0;
                    mmTrend[index] = "--";
                }
                
            } else {
                mmStart[index] = NormalizeDouble(iLow(Symbol(), mmTimeFrame[index], 1+shift) - (30 - spread) * Point, 4);
                mmStop[index] = NormalizeDouble(iHigh(Symbol(), mmTimeFrame[index], 1+shift) + (30 + spread) * Point, 4);
                mmStopLoss[index] = (mmStop[index] - mmStart[index]) / Point;
                mmTrend[index] = "dw";

                lowest = iLowest(Symbol(), mmTimeFrame[index], MODE_LOW, shift+1, 1);
                
                if ((NormalizeDouble(iLow(Symbol(), mmTimeFrame[index], lowest), 4) + spread * Point) <= mmStart[index] || mm_next_period_is_up(index) || index == (MM_TOTAL_PERIODS-1)) {
                    mmStart[index] = 0;
                    mmStop[index] = 0;
                    mmTrend[index] = "--";
                }
            }
        }
        
        if (mmMarketInside[index] == false) {
            mmLots[index] = NormalizeDouble((AccountBalance() * mmRisk[index]) / (mmStopLoss[index] + spread), 2);
            mm_print_trade_line("Start"+index, mmStart[index], Green, mmTimeFrameName[index]+"("+mmTrend[index]+":"+(shift+1)+"): "+DoubleToStr(mmLots[index], 2)+" "+DoubleToStr(mmIA2[index], 2)+"::"+DoubleToStr(mmIA1[index], 2));
        } else {
            mmLots[index] = 0;
            mm_print_trade_line("Start"+index, mmStart[index], Green, mmTimeFrameName[index]+"("+mmTrend[index]+":"+(shift+1)+"): "+DoubleToStr(mmLots[index], 2)+" "+DoubleToStr(mmIA2[index], 2)+"::"+DoubleToStr(mmIA1[index], 2));
            //mm_print_trade_line("Start"+index, 0, Green, "");
        }
    }
/*
    if (mmIA1[index] <= 50) {
        if (mmTrend[index] == "up") {
            mmTrendStopLow[index] = NormalizeDouble(iLow(Symbol(), mmTimeFrame[index], 1+shift) - 30 * Point, 4);
            mmTrendStopHigh[index] = NormalizeDouble(iHigh(Symbol(), mmTimeFrame[index], 1+shift) + 30 * Point, 4);
        } else {
            mmTrendStopLow[index] = NormalizeDouble(iLow(Symbol(), mmTimeFrame[index], 1+shift) - (30 - spread) * Point, 4);
            mmTrendStopHigh[index] = NormalizeDouble(iHigh(Symbol(), mmTimeFrame[index], 1+shift) + (30 + spread) * Point, 4);
        }
    } else {
        mmTrendStopLow[index] = 0;
        mmTrendStopHigh[index] = 0;
    }
*/
    return (0);    
}

//+------------------------------------------------------------------+

bool mm_next_period_is_down(int index)
{
    bool rc = false;
    double med9 = 0;
    double med21 = 0;
    
    if (index < (MM_TOTAL_PERIODS - 1)) {
        med9 = iMA(Symbol(), mmTimeFrame[index+1], 9, 0, MODE_EMA, PRICE_CLOSE, 1);
        med21 = iMA(Symbol(), mmTimeFrame[index+1], 21, 0, MODE_SMA, PRICE_CLOSE, 1);
        
        if (med21 >= med9) {
            rc = true;
        } else {
            rc = false;
        }
    }
    
    return (rc);
}

//+------------------------------------------------------------------+

bool mm_next_period_is_up(int index)
{
    bool rc = false;
    double med9 = 0;
    double med21 = 0;
    
    if (index < (MM_TOTAL_PERIODS - 1)) {
        med9 = iMA(Symbol(), mmTimeFrame[index+1], 9, 0, MODE_EMA, PRICE_CLOSE, 1);
        med21 = iMA(Symbol(), mmTimeFrame[index+1], 21, 0, MODE_SMA, PRICE_CLOSE, 1);
        
        if (med21 <= med9) {
            rc = true;
        } else {
            rc = false;
        }
    }
    
    return (rc);
}

//+------------------------------------------------------------------+

void mm_market_enter(int index)
{
    int err;
    int ticket = -1;
    double ask = 0;
    double bid = 0;
    
    RefreshRates();
    
    ask = NormalizeDouble(Ask, 4);
    bid = NormalizeDouble(Bid, 4);
    
    if (mmTrend[index] == "up" && mmStart[index] != 0 && mmLots[index] != 0) {
        if (bid == mmStart[index]) {
            ticket = OrderSend(Symbol(), OP_BUY, mmLots[index], Ask, 3, mmStop[index], 0, "["+index+"]", mmMagicNumber, 0, Gray);
            if (ticket < 0) {
                err = GetLastError();
                Print("error(", err, "): ", ErrorDescription(err));
                Sleep(1000);
            } else {
                mmMarketInside[index] = true;
            }
        }
    } else if (mmTrend[index] == "dw" && mmStart[index] != 0 && mmLots[index] != 0) {
        if (ask == mmStart[index]) {
            ticket = OrderSend(Symbol(), OP_SELL, mmLots[index], Bid, 3, mmStop[index], 0, "["+index+"]", mmMagicNumber, 0, Gray);
            if (ticket < 0) {
                err = GetLastError();
                Print("error(", err, "): ", ErrorDescription(err));
                Sleep(1000);
            } else {
                mmMarketInside[index] = true;
            }
        }
    }
}

//+------------------------------------------------------------------+

void mm_market_exit(int index)
{
    int i;
    int shift = 0;
    int str_open = 0;
    int str_close = 0;
    double stop = 0;
    int spread = MarketInfo(Symbol(), MODE_SPREAD);
    
    mmMarketInside[index] = false;
    
    mmGainMax[index] = 0;
    
    for (i = 0; i < OrdersTotal(); i++) {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
        // check for symbol & magic number
        //if(OrderSymbol() == Symbol() && OrderMagicNumber() == zzMagicNumber) {
        if (OrderSymbol() == Symbol()) {

            RefreshRates();

            str_open = StringFind(OrderComment(), "[", 0);

            str_open += 1;
            str_close = StringFind(OrderComment(), "]", str_open);
            index = StrToInteger(StringSubstr(OrderComment(), str_open, str_close - str_open));

            mmMarketInside[index] = true;

            if (iBarShift(Symbol(), mmTimeFrame[index], OrderOpenTime()) != 0) {

                mmStart[index] = 0;
                mm_calc_ia(index, 0);

                for (shift = 0; shift < MM_TOTAL_SHIFT; shift++) {
                    if (mmStart[index] == 0) {
                        mm_calc_ia(index, shift+1);
                    }
                }

                mmGainMax[index] = AccountBalance() * mmRisk[index] * MM_MAX_GAIN_FACTOR;

                // long position is opened
                if (OrderType() == OP_BUY) {
               
                    //mmGainMax[index] = AccountBalance() * mmRisk[index] * MM_MAX_GAIN_FACTOR;
               
                    RefreshRates();
                    
                    if (OrderProfit() >= mmGainMax[index]) {
                        if (!OrderClose(OrderTicket(), OrderLots(), Bid, 3, Orange)) {
                            Sleep(10000);
                        }
                    } else if (mmTrend[index] == "dw" && mmStart[index] != 0 && mmStart[index] > OrderStopLoss() && mm_next_period_is_down(index)) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), mmStart[index] - spread * Point, 0, 0, Yellow)) {
                            Sleep(10000);
                        }
                    } else if (mmTrendStopLow[index] != 0 && OrderStopLoss() < mmTrendStopLow[index]) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), mmTrendStopLow[index], 0, 0, Pink)) {
                            Sleep(10000);
                        }
                    } else if (OrderStopLoss() < mmStop[index] && mmStop[index] != 0) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), mmStop[index], 0, 0, Blue)) {
                            Sleep(10000);
                        }
                    }
                } else if (OrderType() == OP_SELL) {
                    
                    if (OrderProfit() >= mmGainMax[index]) {
                        if (!OrderClose(OrderTicket(), OrderLots(), Ask, 3, Orange)) {
                            Sleep(10000);
                        }
                    } else if (mmTrend[index] == "up" && mmStart[index] != 0 && mmStart[index] < OrderStopLoss() && mm_next_period_is_up(index)) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), mmStart[index] + spread * Point, 0, 0, Yellow)) {
                            Sleep(10000);
                        }
                    } else if (mmTrendStopHigh[index] != 0 && OrderStopLoss() > mmTrendStopHigh[index]) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), mmTrendStopHigh[index], 0, 0, Pink)) {
                            Sleep(10000);
                        }
                    } else if (OrderStopLoss() > mmStop[index] && mmStop[index] != 0) {
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), mmStop[index], 0, 0, Blue)) {
                            Sleep(10000);
                        }
                    }
                }
            }
        }
    }    
}

//+------------------------------------------------------------------+

int mm_print_trade_line(string name, double value, int colored, string comment)
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

//+------------------------------------------------------------------+

bool mm_is_high_clande(int timeframe, int shift)
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

//+------------------------------------------------------------------+

