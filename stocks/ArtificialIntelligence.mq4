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
extern int    x1 = 135;
extern int    x2 = 127;
extern int    x3 = 16;
extern int    x4 = 93;
// StopLoss level
extern double tradeStopLoss = 85;
extern double tradeRisk = 0.02;
extern int MagicNumber = 888;

static double tradeLots = 1;

extern int decisionPeriod = PERIOD_H1;
static double decisionVolMed = 0;

static double optVolMed = 0;

extern int tradePeriod = PERIOD_H1;

static int prevticket;


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
     
    int ticket = -1;
   
   //media do volume
    decisionVolMed = 0;
    for (j = 1; j <= 21; j++) {
        decisionVolMed += iVolume(Symbol(), decisionPeriod, j);
    }

    decisionVolMed /= 21;
    decisionVolMed *= 0.95;
    decisionVolMed = NormalizeDouble(decisionVolMed, 5);
    
   //media do volume opt
    optVolMed = 0;
    for (j = 1; j <= 21; j++) {
        optVolMed += iVolume(Symbol(), getOptPeriod(), j);
    }

    optVolMed /= 21;
    optVolMed *= 0.95;
    optVolMed = NormalizeDouble(optVolMed, 5);
    
    ObjectSetText("label_perceptron",
                  "optP(" + getOptPeriod() + "): " + DoubleToStr(perceptron(getOptPeriod()), 5) + 
                  " optVol(" + getOptPeriod() + "): " + DoubleToStr(iVolume(Symbol(), getOptPeriod(), 0), 0) + 
                  " optVolMed: " + DoubleToStr(optVolMed, 0), 
                  8, "Arial", White);

    ObjectSetText("label_perceptron2",
                  "decP(" + decisionPeriod + "): " + DoubleToStr(perceptron(decisionPeriod), 5) + 
                  " decVol(" + decisionPeriod + "): " + DoubleToStr(iVolume(Symbol(), decisionPeriod, 0), 0) + 
                  " decVolMed: " + DoubleToStr(decisionVolMed, 0), 
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
                ObjectSetText("label_perceptron3", 
                              "tradeP(" + tradePeriod + "): " + DoubleToStr(perceptron(tradePeriod), 5) +
                              " tradeProt: " + DoubleToStr((tradeStopLoss * getMultiplier(tradePeriod) * 2  + spread), 0) +
                              " tradeToProt: " + DoubleToStr((Bid - OrderStopLoss()) / Point, 0),
                              8, "Arial", White);
                
                if(Bid > (OrderStopLoss() + (tradeStopLoss * getMultiplier(tradePeriod) * 2  + spread) * Point)) {               
                    
                    RefreshRates();
                    if (perceptron(tradePeriod) < 0) {
                        newStop = Bid - spread * Point;
                    } else {
                        newStop = Bid - tradeStopLoss * getMultiplier(tradePeriod) * Point;
                    }

                    if(!OrderModify(OrderTicket(), OrderOpenPrice(), newStop, 
                                    0, 0, Blue)) {
                        Sleep(10000);
                    } else {
                        period_change_down();
                    }
                } else if ((perceptron(tradePeriod) < 0) && (perceptron(decisionPeriod) < 0)) {
                    
                    candleLow = iLow(Symbol(), decisionPeriod, 0);
                    candleLowPrev = iLow(Symbol(), decisionPeriod, 1);
                    
                    if (candleLowPrev <= candleLow) {
                        newStop = candleLowPrev - spread * Point;
                    } else {
                        newStop = candleLow - spread * Point;
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
                ObjectSetText("label_perceptron3", 
                              "tradeP(" + tradePeriod + "): " + DoubleToStr(perceptron(tradePeriod), 5) +
                              " tradeProt: " + DoubleToStr((tradeStopLoss * getMultiplier(tradePeriod) * 2  + spread), 0) +
                              " tradeToProt: " + DoubleToStr((OrderStopLoss() - Ask) / Point, 0),
                              8, "Arial", White);

                // check profit 
                if(Ask < (OrderStopLoss() - (tradeStopLoss * getMultiplier(tradePeriod) * 2 + spread) * Point)) {
                   
                    RefreshRates();
                    if (perceptron(tradePeriod) > 0) {
                        newStop = Ask + spread * Point;
                    } else {
                        newStop = Ask + tradeStopLoss * getMultiplier(tradePeriod) * Point;
                    }
                    
                    Print("newStop: " + newStop);
                   
                    if(!OrderModify(OrderTicket(), OrderOpenPrice(), newStop, 
                                    0, 0, Blue)) {
                        Sleep(10000);
                    } else {
                        period_change_down();
                    }
                } else if ((perceptron(tradePeriod) > 0) && (perceptron(decisionPeriod) > 0)) {

                    candleHigh = iHigh(Symbol(), decisionPeriod, 0);
                    candleHighPrev = iHigh(Symbol(), decisionPeriod, 1);
                    
                    if (candleHighPrev >= candleHigh) {
                        newStop = candleHighPrev + spread * 2 * Point;
                    } else {
                        newStop = candleHigh + spread * 2 * Point;
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
                          "tradePer(" + tradePeriod + ") decPer(" + decisionPeriod + ")", 
                          MagicNumber, 0, Blue); 
       //----
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(10000);
        } else {
            period_change_up();
        }

     } else if (isShort()) { 
        // short
        
       tradeLots = NormalizeDouble((AccountBalance() * tradeRisk) / (tradeStopLoss * getMultiplier(tradePeriod) + spread), 2);
       
       if (tradeLots < 0.01) tradeLots = 0.01;
       
       RefreshRates();
       ticket = OrderSend(Symbol(), OP_SELL, tradeLots, Bid, 3, 
                          Ask + tradeStopLoss * getMultiplier(tradePeriod) * Point, 0, 
                          "tradePer(" + tradePeriod + ") decPer(" + decisionPeriod + ")", 
                          MagicNumber, 0, Red); 
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(10000);
        } else {
            period_change_up();
        }
    }
     
//--- exit
   return(0);
  }
//+------------------------------------------------------------------+
//| The PERCEPTRON - a perceiving and recognizing function           |
//+------------------------------------------------------------------+
double perceptron(int per) 
  {
   double w1 = x1 - 100;
   double w2 = x2 - 100;
   double w3 = x3 - 100;
   double w4 = x4 - 100;
   double a1 = iAC(Symbol(), per, 0);
   double a2 = iAC(Symbol(), per, 7);
   double a3 = iAC(Symbol(), per, 14);
   double a4 = iAC(Symbol(), per, 21);
   return(w1 * a1 + w2 * a2 + w3 * a3 + w4 * a4);
  }
//+------------------------------------------------------------------+

void period_change_up()
{
   if (decisionPeriod == PERIOD_M1) {
      decisionPeriod = PERIOD_M5;
   } else if (decisionPeriod == PERIOD_M5) {
      decisionPeriod = PERIOD_M15;
   } else if (decisionPeriod == PERIOD_M15) {
      decisionPeriod = PERIOD_M30;
   } else if (decisionPeriod == PERIOD_M30) {
      decisionPeriod = PERIOD_H1;
   } else if (decisionPeriod == PERIOD_H1) {
      decisionPeriod = PERIOD_H4;
   } else if (decisionPeriod == PERIOD_H4) {
      decisionPeriod = PERIOD_D1;
   } else if (decisionPeriod == PERIOD_D1) {
      decisionPeriod = PERIOD_W1;
   } else if (decisionPeriod == PERIOD_W1) {
      decisionPeriod = PERIOD_MN1;
   } else {
      decisionPeriod = PERIOD_H1;
   }
}

void period_change_down()
{
   if (decisionPeriod == PERIOD_MN1) {
      decisionPeriod = PERIOD_W1;
   } else if (decisionPeriod == PERIOD_W1) {
      decisionPeriod = PERIOD_D1;
   } else if (decisionPeriod == PERIOD_D1) {
      decisionPeriod = PERIOD_H4;
   } else if (decisionPeriod == PERIOD_H4) {
      decisionPeriod = PERIOD_H1;
   } else if (decisionPeriod == PERIOD_H1) {
      decisionPeriod = PERIOD_M30;
   } else if (decisionPeriod == PERIOD_M30) {
      decisionPeriod = PERIOD_M15;
   } else if (decisionPeriod == PERIOD_M15) {
      decisionPeriod = PERIOD_M5;
   } else if (decisionPeriod == PERIOD_M5) {
      decisionPeriod = PERIOD_M1;
   } else {
      decisionPeriod = PERIOD_H1;
   }
}

double getMultiplier(int per)
{
    double ret = 1;
   
    if (per == PERIOD_M1) {
        ret = 1.0;
    } else if (per == PERIOD_M5) {
        ret = 1.5;
    } else if (per == PERIOD_M15) {
        ret = 2.0;
    } else if (per == PERIOD_M30) {
        ret = 2.5;
    } else if (per == PERIOD_H1) {
        ret = 3.0;
    } else if (per == PERIOD_H4) {
        ret = 3.5;
    } else if (per == PERIOD_D1) {
        ret = 4.0;
    } else if (per == PERIOD_W1) {
        ret = 4.5;
    } else if (per == PERIOD_MN1) {
        ret = 5.0;
    } else {
        ret = 3.0;
    }
   
   return (ret);
}

bool isLong()
{
    int ret = false;
    
    if ((perceptron(decisionPeriod) > 0) && 
        (perceptron(getOptPeriod()) > 0) && 
        (iVolume(Symbol(), getOptPeriod(), 0) >= optVolMed)) {
    
        tradePeriod = getOptPeriod();
        ret = true;        
    
    } else if ((perceptron(decisionPeriod) > 0) && 
        (iVolume(Symbol(), decisionPeriod, 0) >= decisionVolMed)) {
        
        tradePeriod = decisionPeriod;
        ret = true;        
    }
    
    return (ret);
}

bool isShort()
{
    int ret = false;
    
    if ((perceptron(decisionPeriod) < 0) && 
        (perceptron(getOptPeriod()) < 0) && 
        (iVolume(Symbol(), getOptPeriod(), 0) >= optVolMed)) {
        
        tradePeriod = getOptPeriod();
        ret = true;        
        
    } else if ((perceptron(decisionPeriod) < 0) && 
        (iVolume(Symbol(), decisionPeriod, 0) >= decisionVolMed)) {
        
        tradePeriod = decisionPeriod;
        ret = true;        
    }
    
    return (ret);
}

int getOptPeriod()
{
    int ret = PERIOD_M30;
    
    if (decisionPeriod == PERIOD_MN1) {
        ret = PERIOD_W1;
    } else if (decisionPeriod == PERIOD_W1) {
        ret = PERIOD_D1;
    } else if (decisionPeriod == PERIOD_D1) {
        ret = PERIOD_H4;
    } else if (decisionPeriod == PERIOD_H4) {
        ret = PERIOD_H1;
    } else if (decisionPeriod == PERIOD_H1) {
        ret = PERIOD_M30;
    } else if (decisionPeriod == PERIOD_M30) {
        ret = PERIOD_M15;
    } else if (decisionPeriod == PERIOD_M15) {
        ret = PERIOD_M5;
    } else if (decisionPeriod == PERIOD_M5) {
        ret = PERIOD_M1;
    } else {
        ret = PERIOD_M30;
    }

    return (ret);
}


