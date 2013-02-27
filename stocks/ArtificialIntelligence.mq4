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
extern double tradeRisk = 0.0165;
extern int MagicNumber = 888;

static double tradeLots = 1;
static int decisionPeriod = PERIOD_M1;

static int tradePeriod = PERIOD_M1;
static double tradeBlock = false;
static double tradeBlockTime = 0;

static int prevticket;
double decisionVolMed = 0;


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
    double close = 0;
    double open = 0;
    double candle_low = 0;
    double candle_high = 0;
    double candle_open = 0;
    double candle_close = 0;

    int err;
    int spread = 3;

//----
    if(IsTradeAllowed()) {
       RefreshRates();
       spread = MarketInfo(Symbol(), MODE_SPREAD);
    } else {
       return(0);
    }
     
    if (tradeBlockTime != iTime(Symbol(), decisionPeriod, 0)) {
        tradeBlock = false;
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

    candle_low = iLow(Symbol(), decisionPeriod, 1);
    candle_high = iHigh(Symbol(), decisionPeriod, 1);
    candle_open = iOpen(Symbol(), decisionPeriod, 1);
    candle_close = iClose(Symbol(), decisionPeriod, 1);

    string str_trade = "";
    
    ObjectSetText("label_perceptron", 
                  "tradeP(" + tradePeriod + "): " + DoubleToStr(perceptron(tradePeriod), 5),
                  8, "Arial", White);
                  
    ObjectSetText("label_perceptron2",
                  "decisionP(" + decisionPeriod + "): " + DoubleToStr(perceptron(decisionPeriod), 5) + 
                  " decisionVol(" + decisionPeriod + "): " + DoubleToStr(iVolume(Symbol(), decisionPeriod, 1), 0) + 
                  " decisionVolMed: " + DoubleToStr(decisionVolMed, 0), 
                  8, "Arial", White);

// check for opened position
    int total = OrdersTotal();   
    double newStop = 0;
//----
    for(int i = 0; i < total; i++) {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES); 
        // check for symbol & magic number
        if(OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
            prevticket = OrderTicket();
            tradeBlock = true;
            tradeBlockTime = iTime(Symbol(), decisionPeriod, 0);
           
            // long position is opened
            if(OrderType() == OP_BUY) {
                // check profit 
                if(Bid > (OrderStopLoss() + (tradeStopLoss * getMultiplier() * 2  + spread) * Point)) {               
                    
                    RefreshRates();
                    if (perceptron(tradePeriod) < 0) {
                        newStop = Bid - spread * Point;
                    } else {
                        newStop = Bid - tradeStopLoss * getMultiplier() * Point;
                    }

                    if(!OrderModify(OrderTicket(), OrderOpenPrice(), newStop, 
                                    0, 0, Blue)) {
                        Sleep(10000);
                    } else {
                        period_change_down();
                        tradeBlockTime = iTime(Symbol(), decisionPeriod, 0);
                    }
                }  
                // short position is opened
            } else {
                // check profit 
                if(Ask < (OrderStopLoss() - (tradeStopLoss * getMultiplier() * 2 + spread) * Point)) {
                   
                    RefreshRates();
                    if (perceptron(tradePeriod) > 0) {
                        newStop = Ask + spread * Point;
                    } else {
                        newStop = Ask + tradeStopLoss * getMultiplier() * Point;
                    }
                   
                    if(!OrderModify(OrderTicket(), OrderOpenPrice(), newStop, 
                                    0, 0, Blue)) {
                        Sleep(10000);
                    } else {
                        period_change_down();
                        tradeBlockTime = iTime(Symbol(), decisionPeriod, 0);
                    }
                }  
            }
            // exit
            return(0);
        }
    }
// check for long or short position possibility
    
   if (isLong()) { //long
       tradeLots = NormalizeDouble((AccountBalance() * tradeRisk) / (tradeStopLoss * getMultiplier() + spread), 2);
       
       if (tradeLots < 0.01) tradeLots = 0.01;
       
       RefreshRates();
       ticket = OrderSend(Symbol(), OP_BUY, tradeLots, Ask, 3, Bid - tradeStopLoss * Point, 0, "AI", 
                          MagicNumber, 0, Blue); 
       //----
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(10000);
        } else {
            tradeBlock = true;
            tradeBlockTime = iTime(Symbol(), decisionPeriod, 0);
            tradePeriod = decisionPeriod;
            period_change_up();
        }

     } else if (isShort()) { 
        // short
        
       tradeLots = NormalizeDouble((AccountBalance() * tradeRisk) / (tradeStopLoss * getMultiplier() + spread), 2);
       
       if (tradeLots < 0.01) tradeLots = 0.01;
       
       RefreshRates();
       ticket = OrderSend(Symbol(), OP_SELL, tradeLots, Bid, 3, Ask + tradeStopLoss * Point, 0, "AI", 
                          MagicNumber, 0, Red); 
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(10000);
        } else {
            tradeBlock = true;
            tradeBlockTime = iTime(Symbol(), decisionPeriod, 0);
            tradePeriod = decisionPeriod;
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
   } else {
      decisionPeriod = PERIOD_D1;
   }
}

void period_change_down()
{
   if (decisionPeriod == PERIOD_D1) {
      decisionPeriod = PERIOD_H4;
   } else if (decisionPeriod == PERIOD_H4) {
      decisionPeriod = PERIOD_H1;
   } else if (decisionPeriod == PERIOD_H1) {
      decisionPeriod = PERIOD_M30;
   } else if (decisionPeriod == PERIOD_M30) {
      decisionPeriod = PERIOD_M15;
   } else if (decisionPeriod == PERIOD_M15) {
      decisionPeriod = PERIOD_M5;
   } else {
      decisionPeriod = PERIOD_M1;
   }
}

double getMultiplier()
{
    double ret = 1;
   
    if (decisionPeriod == PERIOD_M1) {
        ret = 1.0;
    } else if (decisionPeriod == PERIOD_M5) {
        ret = 1.5;
    } else if (decisionPeriod == PERIOD_M15) {
        ret = 2.0;
    } else if (decisionPeriod == PERIOD_M30) {
        ret = 2.5;
    } else if (decisionPeriod == PERIOD_H1) {
        ret = 3.0;
    } else if (decisionPeriod == PERIOD_H4) {
        ret = 3.5;
    } else {
        ret = 4.0;
    }
   
   return (ret);
}

bool isLong()
{
    int ret = false;
    
    if ((perceptron(decisionPeriod) > 0) && 
        (tradeBlock == false) && 
        (iVolume(Symbol(), decisionPeriod, 1) >= decisionVolMed)) {
        ret = true;        
    }
    
    return (ret);
}

bool isShort()
{
    int ret = false;
    
    if ((perceptron(decisionPeriod) < 0) && 
        (tradeBlock == false) && 
        (iVolume(Symbol(), decisionPeriod, 1) >= decisionVolMed)) {
        ret = true;        
    }
    
    return (ret);
}