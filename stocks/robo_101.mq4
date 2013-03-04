//+------------------------------------------------------------------+
//|                                       ArtificialIntelligence.mq4 |
//|                               Copyright © 2006, Yury V. Reshetov |
//|                                         http://reshetov.xnet.uz/ |
//+------------------------------------------------------------------+

#include <stderror.mqh>
#include <stdlib.mqh>

#property copyright "Copyright © 2006, Yury V. Reshetov ICQ:282715499  http://reshetov.xnet.uz/"
#property link      "http://reshetov.xnet.uz/"
//---- input parameters
extern int    x1 = 0;
extern int    x2 = 101;
extern int    x3 = 101;
extern int    x4 = 101;
// StopLoss level
extern double tradeStopLoss = 85;
extern double tradeRisk = 0.02;
extern int MagicNumber = 888;

extern int tradePeriod = PERIOD_M30;

static double tradeLots = 1;
static double tradeVolMed = 0;
static double optVolMed = 0;
static double decisionVolMed = 0;
static bool tradeBlock = false;
static double tradeBlockTime = 0;

//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
  {
    ObjectCreate("label_perceptron", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_perceptron", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_perceptron", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_perceptron", OBJPROP_YDISTANCE, 15);// Y coordinate
    
    ObjectCreate("label_perceptron2", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_perceptron2", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_perceptron2", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_perceptron2", OBJPROP_YDISTANCE, 30);// Y coordinate

    ObjectCreate("label_perceptron3", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_perceptron3", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_perceptron3", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_perceptron3", OBJPROP_YDISTANCE, 45);// Y coordinate

    ObjectCreate("label_perceptron4", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_perceptron4", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_perceptron4", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_perceptron4", OBJPROP_YDISTANCE, 60);// Y coordinate

   
    //getTradePeriod();
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
int deinit()
  {
    ObjectsDeleteAll();
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
int start()
{
    int j ;
    int err;
    int spread = 3;
    int ticket = -1;
    string str_trade = "";
    double candleLow = 0;
    double candleLowPrev = 0;
    double candleHigh = 0;
    double candleHighPrev = 0;

//----
    if(IsTradeAllowed()) {
       RefreshRates();
       spread = MarketInfo(Symbol(), MODE_SPREAD);
    } else {
       return(0);
    }

    if (tradeBlockTime != iTime(Symbol(), tradePeriod, 0)) {
        tradeBlock = false;
    }
        
    tradeVolMed = getVolume(tradePeriod);
    optVolMed = getVolume(getOptPeriod(tradePeriod));
    decisionVolMed = getVolume(getNextPeriod(tradePeriod));
    
    ObjectSetText("label_perceptron",
                  "optP(" + getOptPeriod(tradePeriod) + "): " + DoubleToStr(perceptron(getOptPeriod(tradePeriod), 0), 4) + 
                  " optVol(" + tradeBlock + "): " + DoubleToStr(iVolume(Symbol(), getOptPeriod(tradePeriod), 0), 0) + 
                  " optVolMed: " + DoubleToStr(optVolMed, 0), 
                  8, "Arial", White);

    ObjectSetText("label_perceptron2",
                  "trdP(" + tradePeriod + "): " + DoubleToStr(perceptron(tradePeriod, 0), 4) + 
                  " trdVol(" + tradeBlock + "): " + DoubleToStr(iVolume(Symbol(), tradePeriod, 0), 0) + 
                  " trdVolMed: " + DoubleToStr(tradeVolMed, 0), 
                  8, "Arial", White);

    ObjectSetText("label_perceptron3",
                  "decP(" + getNextPeriod(tradePeriod) + "): " + DoubleToStr(perceptron(getNextPeriod(tradePeriod), 0), 4) + 
                  " decVol(" + tradeBlock + "): " + DoubleToStr(iVolume(Symbol(), getNextPeriod(tradePeriod), 0), 0) + 
                  " decVolMed: " + DoubleToStr(decisionVolMed, 0), 
                  8, "Arial", White);

    ObjectSetText("label_perceptron4",
                  "wP(0): " + DoubleToStr(perceptron(0, 0), 4),
                  8, "Arial", White);

// check for opened position
    int total = OrdersTotal();   
    double newStop = 0;
//----
    for(int i = 0; i < total; i++) {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES); 
        // check for symbol & magic number
        if(OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
           
            // long position is opened
            if(OrderType() == OP_BUY) {
                // check profit 
                
                if (perceptron(tradePeriod, 0) < 0) {
                    
                    candleLow = iLow(Symbol(), tradePeriod, 0);
                    candleLowPrev = iLow(Symbol(), tradePeriod, 1);
                    
                    if (candleLowPrev <= candleLow && candleLowPrev < Bid) {
                        newStop = candleLowPrev - spread * Point;
                    } else if (candleLowPrev > candleLow && candleLow < Bid) {
                        newStop = candleLow - spread * Point;
                    } else {
                        newStop = Bid - spread * Point;
                    }
                                    
                    if (newStop > OrderStopLoss()) {          
                        if(!OrderModify(OrderTicket(), OrderOpenPrice(), newStop, 
                                        0, 0, Blue)) {
                            Sleep(10000);
                        } else {
                            Print("SL by (tradeP and decP) < 0");
                        }
                    }
                }

                // short position is opened
            } else {
                // check profit 
                if (perceptron(tradePeriod, 0) > 0) {
                
                    candleHigh = iHigh(Symbol(), tradePeriod, 0);
                    candleHighPrev = iHigh(Symbol(), tradePeriod, 1);
                    
                    if (candleHighPrev >= candleHigh && candleHighPrev > Ask) {
                        newStop = candleHighPrev + spread * 2 * Point;
                    } else if (candleHighPrev < candleHigh && candleHigh > Ask) {
                        newStop = candleHigh + spread * 2 * Point;
                    } else {
                        newStop = Ask + spread * 2 * Point;
                    }
                    
                    if (newStop < OrderStopLoss()) {          
                        if(!OrderModify(OrderTicket(), OrderOpenPrice(), newStop, 
                                        0, 0, Blue)) {
                            err=GetLastError();
                            Print("error(",err,"): ",ErrorDescription(err));
                            Sleep(10000);
                        } else {
                            Print("SL by (tradeP and decP) < 0");
                        }
                    }
                }  
            }
            // exit
            return(0);
        }
    }
// check for long or short position possibility
    
   if (isLong()) { //long
       tradeLots = NormalizeDouble((AccountBalance() * tradeRisk) / (tradeStopLoss * getMultiplier(tradePeriod) + spread), 2);
       
       if (tradeLots < 0.01) tradeLots = 0.01;
       
       RefreshRates();
       ticket = OrderSend(Symbol(), OP_BUY, tradeLots, Ask, 3, 
                          Bid - tradeStopLoss * getMultiplier(tradePeriod) * Point, 0, 
                          "trdPer(" + tradePeriod + ")", 
                          MagicNumber, 0, Blue); 
       //----
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(10000);
        } else {
            tradeBlockTime = iTime(Symbol(), tradePeriod, 0);            
            tradeBlock = true;
        }

     } else if (isShort()) { 
        // short
        
       tradeLots = NormalizeDouble((AccountBalance() * tradeRisk) / (tradeStopLoss * getMultiplier(tradePeriod) + spread), 2);
       
       if (tradeLots < 0.01) tradeLots = 0.01;
       
       RefreshRates();
       ticket = OrderSend(Symbol(), OP_SELL, tradeLots, Bid, 3, 
                          Ask + tradeStopLoss * getMultiplier(tradePeriod) * Point, 0, 
                          "trdPer(" + tradePeriod + ")", 
                          MagicNumber, 0, Red); 
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(10000);
        } else {
            tradeBlockTime = iTime(Symbol(), tradePeriod, 0);            
            tradeBlock = true;
        }
    }
     
//--- exit
   return(0);
  }
//+------------------------------------------------------------------+
//| The PERCEPTRON - a perceiving and recognizing function           |
//+------------------------------------------------------------------+
double perceptron(int period, int shift) 
{
    double w1 = x1 - 100;
    double w2 = x2 - 100;
    double w3 = x3 - 100;
    double w4 = x4 - 100;
    double a1 = iAC(Symbol(), period, shift + 0);
    double a2 = iAC(Symbol(), period, shift + 7);
    double a3 = iAC(Symbol(), period, shift + 14);
    double a4 = iAC(Symbol(), period, shift + 21);
    return(w1 * a1 + w2 * a2 + w3 * a3 + w4 * a4);
}
//+------------------------------------------------------------------+

double getMultiplier(int per)
{
    double ret = 1;
   
    if (per == PERIOD_M1) {
        ret = 1.0;
    } else if (per == PERIOD_M5) {
        ret = 1.0;
    } else if (per == PERIOD_M15) {
        ret = 1.0;
    } else if (per == PERIOD_M30) {
        ret = 1.5;
    } else if (per == PERIOD_H1) {
        ret = 2.0;
    } else if (per == PERIOD_H4) {
        ret = 2.5;
    } else if (per == PERIOD_D1) {
        ret = 3.0;
    } else if (per == PERIOD_W1) {
        ret = 3.5;
    } else if (per == PERIOD_MN1) {
        ret = 4.0;
    } else {
        ret = 2.0;
    }
   
   return (ret);
}

bool isLong()
{
    int ret = false;
    
    if (((perceptron(getOptPeriod(tradePeriod), 0) > 0) && 
         (perceptron(tradePeriod, 0) > 0) && 
         (perceptron(getNextPeriod(tradePeriod), 0) > 0)) &&
        ((iVolume(Symbol(), getOptPeriod(tradePeriod), 0) >= optVolMed) || 
         (iVolume(Symbol(), tradePeriod, 0) >= tradeVolMed) ||
         (iVolume(Symbol(), getNextPeriod(tradePeriod), 0) >= decisionVolMed)) &&
        (tradeBlock == false)) {

        ret = true;        
    }

    return (ret);
}

bool isShort()
{
    int ret = false;
    
    if (((perceptron(getOptPeriod(tradePeriod), 0) < 0) && 
         (perceptron(tradePeriod, 0) < 0) && 
         (perceptron(getNextPeriod(tradePeriod), 0) < 0)) &&
        ((iVolume(Symbol(), getOptPeriod(tradePeriod), 0) >= optVolMed) || 
         (iVolume(Symbol(), tradePeriod, 0) >= tradeVolMed) ||
         (iVolume(Symbol(), getNextPeriod(tradePeriod), 0) >= decisionVolMed)) &&
        (tradeBlock == false)) {
        
        ret = true;                
    }
    
    return (ret);
}

int getOptPeriod(int period)
{
    int ret = PERIOD_M30;
    
    if (period == PERIOD_MN1) {
        ret = PERIOD_W1;
    } else if (period == PERIOD_W1) {
        ret = PERIOD_D1;
    } else if (period == PERIOD_D1) {
        ret = PERIOD_H4;
    } else if (period == PERIOD_H4) {
        ret = PERIOD_H1;
    } else if (period == PERIOD_H1) {
        ret = PERIOD_M30;
    } else if (period == PERIOD_M30) {
        ret = PERIOD_M15;
    } else if (period == PERIOD_M15) {
        ret = PERIOD_M5;
    } else if (period == PERIOD_M5) {
        ret = PERIOD_M1;
    } else {
        ret = PERIOD_M30;
    }

    return (ret);
}

int getNextPeriod(int per)
{
    int ret = PERIOD_H1;
    
    if (per == PERIOD_M1) {
        ret = PERIOD_M5;
    } else if (per == PERIOD_M5) {
        ret = PERIOD_M15;
    } else if (per == PERIOD_M15) {
        ret = PERIOD_M30;
    } else if (per == PERIOD_M30) {
        ret = PERIOD_H1;
    } else if (per == PERIOD_H1) {
        ret = PERIOD_H4;
    } else if (per == PERIOD_H4) {
        ret = PERIOD_D1;
    } else if (per == PERIOD_D1) {
        ret = PERIOD_W1;
    } else if (per == PERIOD_W1) {
        ret = PERIOD_MN1;
    } else {
        ret = PERIOD_H1;
    }

    return (ret);
}

double getVolume(int period)
{
    int i;
    double volume;
    
    volume = 0;
    for (i = 0; i < 21; i++) {
        volume += iVolume(Symbol(), period, i);
    }

    volume /= 21;
    volume *= 0.95;
    volume = NormalizeDouble(volume, 0);
    
    return (volume);
}

