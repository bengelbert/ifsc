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
extern double sl = 85;
extern double risk = 0.05;
extern int MagicNumber = 888;

static double lots = 1;
static int hist_period = PERIOD_M1;
double block = false;
static double time_block = 0;

static double max_candle = 0;
static double min_candle = 999;

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
    max_candle = 0;
    min_candle = 999;
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
int deinit()
  {
    ObjectDelete("label_perceptron");
    ObjectDelete("linedown");
    ObjectDelete("lineup");
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
int start()
  {
    int j ;
    double med_shadow_max = 0;
    double med_shadow_min = 0;
    double close = 0;
    double open = 0;
    double candle_low = 0;
    double candle_high = 0;
    double candle_open = 0;
    double candle_close = 0;
    double l_price = 0;
    double h_price = 0;
    double med_volume = 0;

   int spread = 3;

//----
   if(IsTradeAllowed()) 
     {
       RefreshRates();
       spread = MarketInfo(Symbol(), MODE_SPREAD);
     } 
   else 
     {
       return(0);
     }
     
    if (time_block != iTime(Symbol(), hist_period, 0)) {
        block = false;
    }
    
    int ticket = -1;

    
   
   //media da sombra maxima   
    med_shadow_max = 0;
    for (j = 1; j <= 21; j++) {
      close = iClose(Symbol(), hist_period, j);
      open = iOpen(Symbol(), hist_period, j);
      
      if (close > open) {
         med_shadow_max += iHigh(Symbol(), hist_period, j) - close;
      } else {
         med_shadow_max += iHigh(Symbol(), hist_period, j) - open;
      }
    }
    
    med_shadow_max /= 21;
    med_shadow_max = NormalizeDouble(med_shadow_max*2, 5);

   //media da sombra minima
    med_shadow_min = 0;
    for (j = 1; j <= 21; j++) {
      close = iClose(Symbol(), hist_period, j);
      open = iOpen(Symbol(), hist_period, j);
      
      if (close > open) {
         med_shadow_min += open - iLow(Symbol(), hist_period, j);
      } else {
         med_shadow_min += close - iLow(Symbol(), hist_period, j);
      }
    }
    
    med_shadow_min /= 21;
    med_shadow_min = NormalizeDouble(med_shadow_min*2, 5);

   //media do volume
    med_volume = 0;
    for (j = 1; j <= 21; j++) {
      med_volume += iVolume(Symbol(), hist_period, j);
    }
    
    med_volume /= 21;
    med_volume = NormalizeDouble(med_volume, 5);

    
    ObjectSetText("label_perceptron", "p(): " + perceptron() + 
                  " Per: " + hist_period + 
                  " Max: " + med_shadow_max + 
                  " Min: " + med_shadow_min, 8, "Arial", White);
                  
    ObjectSetText("label_perceptron2", "Vol: " + med_volume + 
                  " MaxCdl: " + max_candle +
                  " MinCdl: " + min_candle, 8, "Arial", White);

    candle_low = iLow(Symbol(), hist_period, 1);
    candle_high = iHigh(Symbol(), hist_period, 1);
    candle_open = iOpen(Symbol(), hist_period, 1);
    candle_close = iClose(Symbol(), hist_period, 1);

    //l_price = candle_low - spread * Point;// - 0.0001;
    //h_price = candle_high + spread * Point;// + 0.0001;

    l_price = candle_low - med_shadow_min;// - 0.0001;
    h_price = candle_high + med_shadow_max;// + 0.0001;

    ObjectDelete("linedown");
    ObjectDelete("lineup");
    ObjectCreate("linedown", OBJ_HLINE, 0, 0, l_price);
    ObjectCreate("lineup", OBJ_HLINE, 0, 0, h_price);
    
    if (perceptron() > 0) {
        ObjectSet("lineup", OBJPROP_COLOR, Blue);
        ObjectSet("linedown", OBJPROP_COLOR, Red);
    } else {
        ObjectSet("lineup", OBJPROP_COLOR, Red);
        ObjectSet("linedown", OBJPROP_COLOR, Blue);
    }    
       
// check for opened position
    int total = OrdersTotal();   

//----
   for(int i = 0; i < total; i++) 
     {
       OrderSelect(i, SELECT_BY_POS, MODE_TRADES); 
       // check for symbol & magic number
       if(OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) 
         {
           int prevticket = OrderTicket();
           block = true;
           time_block = iTime(Symbol(), hist_period, 0);
           
           // long position is opened
           if(OrderType() == OP_BUY) 
             {
               // check profit 
               if(Bid > (OrderStopLoss() + (sl * 2  + spread) * Point)) 
                 {               
                   if(perceptron() < 0 && (Ask <= l_price)) 
                     { // reverse
                       RefreshRates();
                       ticket = OrderSend(Symbol(), OP_SELL, lots * 2, Bid, 3, 
                                          Ask + sl * Point, 0, "AI", MagicNumber, 0, Red); 
                       Sleep(5000);
                       //----
                       if(ticket >= 0) 
                           OrderCloseBy(ticket, prevticket, Blue);   
                     } else { // trailing stop
                       if(!OrderModify(OrderTicket(), OrderOpenPrice(), Bid - sl * Point, 
                          0, 0, Blue)) 
                         {
                           Sleep(5000);
                         } else {
                           period_change_down();
                         }
                     }
                 } else if (is_high_close_candle()) { 
                    RefreshRates();
                    if(!OrderModify(OrderTicket(), OrderOpenPrice(), candle_low - (med_shadow_min * 2), 
                      0, 0, Blue)) 
                     {
                       Sleep(5000);
                     }
                 }  
               // short position is opened
             } 
           else 
             {
               // check profit 
               if(Ask < (OrderStopLoss() - (sl * 2 + spread) * Point)) 
                 {
                   if(perceptron() > 0 && (Bid >= h_price)) 
                     { // reverse
                       RefreshRates();
                       ticket = OrderSend(Symbol(), OP_BUY, lots * 2, Ask, 3, 
                                          Bid - sl * Point, 0, "AI", MagicNumber, 0, Blue); 
                       Sleep(5000);
                       //----
                       if(ticket >= 0) 
                           OrderCloseBy(ticket, prevticket, Blue);   
                     } 
                   else 
                     { // trailing stop
  
                       if(!OrderModify(OrderTicket(), OrderOpenPrice(), Ask + sl * Point, 
                          0, 0, Blue)) 
                         {
                           Sleep(5000);
                         } else {
                           period_change_down();
                         }
                     }
                  } else if (is_low_close_candle()) { 
                     RefreshRates();
                     if(!OrderModify(OrderTicket(), OrderOpenPrice(), candle_high + (med_shadow_max * 2), 
                       0, 0, Blue)) 
                     {
                        Sleep(5000);
                     }
                 }  
             }
           // exit
           return(0);
         }
     }
// check for long or short position possibility
   int err;
    
    
   if((perceptron() > 0) && (Bid >= h_price) && (block == false) && (iVolume(Symbol(), hist_period, 0) >= med_volume)) 
     { //long
       lots = NormalizeDouble((AccountBalance() * risk) / (sl + spread), 2);
       
       RefreshRates();
       ticket = OrderSend(Symbol(), OP_BUY, lots, Ask, 3, Bid - sl * Point, 0, "AI", 
                          MagicNumber, 0, Blue); 
       //----
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(5000);
        } else {
            block = true;
            time_block = iTime(Symbol(), hist_period, 0);
            period_change_up();
            max_candle = candle_high;
        }
        
     } else if ((perceptron() < 0) && (Ask <= l_price) && (block == false) && (iVolume(Symbol(), hist_period, 0) >= med_volume)) { 
        // short
       lots = NormalizeDouble((AccountBalance() * risk) / (sl + spread), 2);
       
       RefreshRates();
       ticket = OrderSend(Symbol(), OP_SELL, lots, Bid, 3, Ask + sl * Point, 0, "AI", 
                          MagicNumber, 0, Red); 
        if(ticket < 0) {
            err=GetLastError();
            Print("error(",err,"): ",ErrorDescription(err));
            Sleep(5000);
        } else {
            block = true;
            time_block = iTime(Symbol(), hist_period, 0);
            period_change_up();
            min_candle = candle_low;
        }
    }
     
//--- exit
   return(0);
  }
//+------------------------------------------------------------------+
//| The PERCEPTRON - a perceiving and recognizing function           |
//+------------------------------------------------------------------+
double perceptron() 
  {
   double w1 = x1 - 100;
   double w2 = x2 - 100;
   double w3 = x3 - 100;
   double w4 = x4 - 100;
   double a1 = iAC(Symbol(), hist_period, 0);
   double a2 = iAC(Symbol(), hist_period, 7);
   double a3 = iAC(Symbol(), hist_period, 14);
   double a4 = iAC(Symbol(), hist_period, 21);
   return(w1 * a1 + w2 * a2 + w3 * a3 + w4 * a4);
  }
//+------------------------------------------------------------------+

void period_change_up()
{
   if (hist_period == PERIOD_M1) {
      hist_period = PERIOD_M5;
   } else if (hist_period == PERIOD_M5) {
         hist_period = PERIOD_M15;
   } else if (hist_period == PERIOD_M15) {
      hist_period = PERIOD_M30;
   } else if (hist_period == PERIOD_M30) {
      hist_period = PERIOD_H1;
   } else if (hist_period == PERIOD_H1) {
      hist_period = PERIOD_H4;
   } else {
      hist_period = PERIOD_D1;
   }
}

void period_change_down()
{
   if (hist_period == PERIOD_D1) {
      hist_period = PERIOD_H4;
   } else if (hist_period == PERIOD_H4) {
         hist_period = PERIOD_H1;
   } else if (hist_period == PERIOD_H1) {
      hist_period = PERIOD_M30;
   } else if (hist_period == PERIOD_M30) {
      hist_period = PERIOD_M15;
   } else if (hist_period == PERIOD_M15) {
      hist_period = PERIOD_M5;
   } else {
      hist_period = PERIOD_M1;
   }
}

bool is_high_close_candle()
{
    bool ret = false;

    double candle_open = 0;
    double candle_close = 0;
    double candle_high = 0;
    
    candle_open = iOpen(Symbol(), hist_period, 1);
    candle_close = iClose(Symbol(), hist_period, 1);
    candle_high = iHigh(Symbol(), hist_period, 1);
    
    if (candle_close > candle_open && candle_close > max_candle) {
        ret = true;
        max_candle = candle_high;       
    }
    
    return (ret);
}

bool is_low_close_candle()
{
    bool ret = false;

    double candle_open = 0;
    double candle_close = 0;
    double candle_low = 0;

    candle_open = iOpen(Symbol(), hist_period, 1);
    candle_close = iClose(Symbol(), hist_period, 1);
    candle_low = iLow(Symbol(), hist_period, 1);
    
    if (candle_close < candle_open && candle_close < min_candle) {
        ret = true;
        min_candle = candle_low;
    }
    
    return (ret);
}
