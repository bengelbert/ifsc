//+------------------------------------------------------------------+
//|                                       ArtificialIntelligence.mq4 |
//|                               Copyright © 2006, Yury V. Reshetov |
//|                                         http://reshetov.xnet.uz/ |
//+------------------------------------------------------------------+
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
static double lots = 1;
extern int MagicNumber = 888;

//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
  {
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
int deinit()
  {
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
int start()
  {
    double candle_low = 0;
    double candle_high = 0;
    double candle_open = 0;
    double candle_close = 0;
    double l_price = 0;
    double h_price = 0;

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
     
    int ticket = -1;
   
    candle_low = iLow(Symbol(), Period(), 1);
    candle_high = iHigh(Symbol(), Period(), 1);
    candle_open = iOpen(Symbol(), Period(), 1);
    candle_close = iClose(Symbol(), Period(), 1);
    
    l_price = candle_low - spread * Point;// - 0.0001;
    h_price = candle_high + spread * Point;// + 0.0001;

    ObjectDelete("linedown");
    ObjectDelete("lineup");
    ObjectCreate("linedown", OBJ_HLINE, 0, 0, l_price);
    ObjectCreate("lineup", OBJ_HLINE, 0, 0, h_price);
    ObjectSet("lineup", OBJPROP_COLOR, Red);
    ObjectSet("linedown", OBJPROP_COLOR, Red);
   
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
           
           // long position is opened
           if(OrderType() == OP_BUY) 
             {
               // check profit 
               if(Bid > (OrderStopLoss() + (sl * 2  + spread) * Point)) 
                 {               
                   if(perceptron() < 0 && (Ask <= l_price)) 
                     { // reverse
                       ticket = OrderSend(Symbol(), OP_SELL, lots * 2, Bid, 3, 
                                          Ask + sl * Point, 0, "AI", MagicNumber, 0, Red); 
                       Sleep(30000);
                       //----
                       if(ticket >= 0) 
                           OrderCloseBy(ticket, prevticket, Blue);   
                     } 
                   else 
                     { // trailing stop
                       if(!OrderModify(OrderTicket(), OrderOpenPrice(), Bid - sl * Point, 
                          0, 0, Blue)) 
                         {
                           Sleep(30000);
                         }
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
                       ticket = OrderSend(Symbol(), OP_BUY, lots * 2, Ask, 3, 
                                          Bid - sl * Point, 0, "AI", MagicNumber, 0, Blue); 
                       Sleep(30000);
                       //----
                       if(ticket >= 0) 
                           OrderCloseBy(ticket, prevticket, Blue);   
                     } 
                   else 
                     { // trailing stop
                       if(!OrderModify(OrderTicket(), OrderOpenPrice(), Ask + sl * Point, 
                          0, 0, Blue)) 
                         {
                           Sleep(30000);
                         }
                     }
                 }  
             }
           // exit
           return(0);
         }
     }
// check for long or short position possibility
   if((perceptron() > 0) && (Bid >= h_price)) 
     { //long
       lots = NormalizeDouble((AccountBalance() * risk) / (sl + spread), 0);
       ticket = OrderSend(Symbol(), OP_BUY, lots, Ask, 3, Bid - sl * Point, 0, "AI", 
                          MagicNumber, 0, Blue); 
       //----
       if(ticket < 0) 
         {
           Sleep(30000);
         }
     } else if ((perceptron() < 0) && (Ask <= l_price)) { 
        // short
       lots = NormalizeDouble((AccountBalance() * risk) / (sl + spread), 0);
       ticket = OrderSend(Symbol(), OP_SELL, lots, Bid, 3, Ask + sl * Point, 0, "AI", 
                          MagicNumber, 0, Red); 
       if(ticket < 0) 
         {
           Sleep(30000);
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
   double a1 = iAC(Symbol(), 0, 0);
   double a2 = iAC(Symbol(), 0, 7);
   double a3 = iAC(Symbol(), 0, 14);
   double a4 = iAC(Symbol(), 0, 21);
   return(w1 * a1 + w2 * a2 + w3 * a3 + w4 * a4);
  }
//+------------------------------------------------------------------+


