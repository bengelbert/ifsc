//+------------------------------------------------------------------+
//|                                                          wts.mq4 |
//|                        Copyright 2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#property copyright "Copyright 2013, MetaQuotes Software Corp."
#property link      "http://www.metaquotes.net"

extern int    x1 = 0;
extern int    x2 = 101;
extern int    x3 = 101;
extern int    x4 = 101;

extern int per = 0;
//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
  {
//----
    int i;
    bool prev = false;
    bool now = false;
    double p = 0;
    
    
    ObjectsDeleteAll();
    
    for (i = 0; i < 100; i++) {
    
        p = perceptron(per, i);
        
        if (p > 0) {
            now = true;
        } else {
            now = false;
        }
        
        if (i == 0) {
            prev = now;
        } else if (now != prev && (p > 0.001 || p < -0.001)) {
            Print("p(): " + p);
            
            if (now) {
                ObjectCreate("seta_"+i, OBJ_ARROW, 0, iTime(Symbol(), per, i), iLow(Symbol(), per, i)-0.001);
                ObjectSet("seta_"+i, OBJPROP_ARROWCODE, SYMBOL_ARROWUP);
                ObjectSet("seta_"+i, OBJPROP_COLOR, Lime);
            } else {
                ObjectCreate("seta_"+i, OBJ_ARROW, 0, iTime(Symbol(), per, i), iHigh(Symbol(), per, i)+0.001);             
                ObjectSet("seta_"+i, OBJPROP_ARROWCODE, SYMBOL_ARROWDOWN);
                ObjectSet("seta_"+i, OBJPROP_COLOR, Red);
            }
        
            prev = now;
        }
    }
   
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
int deinit()
{
//----

   
//----
   return(0);
}
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
int start()
  {
//----
   
//----
   return(0);
  }
//+------------------------------------------------------------------+

double perceptron(int timeframe, int shift) 
{
    double w1 = x1 - 100;
    double w2 = x2 - 100;
    double w3 = x3 - 100;
    double w4 = x4 - 100;
    double a1 = iAC(Symbol(), timeframe, shift+0);
    double a2 = iAC(Symbol(), timeframe, shift+7);
    double a3 = iAC(Symbol(), timeframe, shift+14);
    double a4 = iAC(Symbol(), timeframe, shift+21);
    return(w1 * a1 + w2 * a2 + w3 * a3 + w4 * a4);
}