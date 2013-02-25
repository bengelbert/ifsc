//+------------------------------------------------------------------+
//|                                                        Ninja.mq4 |
//|                        Copyright 2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#property copyright "Copyright 2013, MetaQuotes Software Corp."
#property link      "http://www.metaquotes.net"

#define ST_WAIT 0
#define ST_BUY  1
#define ST_SELL 2

#define PIPS    100000

int g_state = ST_WAIT;

bool flag_long = false; 
bool flag_short = false;

double g_long_start = 0;
double g_long_stop = 0;
double g_long_candle_high = 0;

double g_short_start = 0;
double g_short_stop = 0;
double g_short_candle_low = 999;

double g_patrimonio = 100;
double g_percent_risk = 0.05;
double g_loss = 0;
double g_volume = 0;
double g_spread = 0;

string str_start_long = "start_long";
string str_start_short = "start_short";
string str_stop_long = "stop_long";
string str_stop_short = "stop_short";


//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
  {
//----
    g_state = ST_WAIT;
    flag_long = false; 
    flag_short = false;
    
    
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
int deinit()
  {
//----
    ObjectsDeleteAll();
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
int start()
{
    double spread = 0;
    double lucro = 0;
    double per_lucro = 0;
    double risk = 0;
    double candle_high = 0;
    double candle_low = 0;
    double candle_close = 0;
    double candle_open = 0;
    double l_price = 0;
    double h_price = 0;
 
    ObjectDelete("linedown");
    ObjectDelete("lineup");
    
    spread = Ask - Bid;
    //spread = 0;

    candle_low = iLow(Symbol(), Period(), 1);
    candle_high = iHigh(Symbol(), Period(), 1);
    candle_open = iOpen(Symbol(), Period(), 1);
    candle_close = iClose(Symbol(), Period(), 1);
    
    l_price = candle_low - spread;// - 0.0001;
    h_price = candle_high + spread;// + 0.0001;

    if (flag_long == false && flag_short == false) {

        ObjectCreate("linedown", OBJ_HLINE, 0, 0, l_price);
        ObjectCreate("lineup", OBJ_HLINE, 0, 0, h_price);
        
        if (is_high_candle()) {
            ObjectSet("lineup", OBJPROP_COLOR, Blue);
            ObjectSet("linedown", OBJPROP_COLOR, Red);
        } else if (is_low_candle()) {
            ObjectSet("lineup", OBJPROP_COLOR, Red);
            ObjectSet("linedown", OBJPROP_COLOR, Blue);
        } else {
            ObjectSet("lineup", OBJPROP_COLOR, Orange);
            ObjectSet("linedown", OBJPROP_COLOR, Orange);
        }

        if (Bid >= h_price) {
            g_loss = g_patrimonio * g_percent_risk;
            g_long_start = Bid + spread;
            g_long_stop = l_price;
            g_long_candle_high = candle_high;
            
            g_spread = spread;
            
            risk = (g_long_start - g_long_stop - spread) * PIPS;
            
            g_volume = g_loss / risk;

            ObjectCreate("start" + g_long_start, OBJ_TEXT, 0, Time[0], g_long_start+0.0005);
            ObjectSetText("start" + g_long_start, 
                "LONG START: Patrimonio: " + g_patrimonio + 
                " Volume: " + g_volume, 8, "Times New Roman", White);
        
            ObjectCreate(str_start_long, OBJ_HLINE, 0, 0, g_long_start);
            ObjectSet(str_start_long, OBJPROP_COLOR, Blue);
            ObjectSet(str_start_long, OBJPROP_STYLE, STYLE_DOT);

            ObjectCreate(str_stop_long, OBJ_HLINE, 0, 0, g_long_stop);
            ObjectSet(str_stop_long, OBJPROP_COLOR, Red);
            ObjectSet(str_stop_long, OBJPROP_STYLE, STYLE_DOT);

            flag_long = true;
        }

        if (Bid <= l_price) {
            g_loss = g_patrimonio * g_percent_risk;
            g_short_start = Bid - spread;
            g_short_stop = h_price;
            g_short_candle_low = candle_low;
            
            g_spread = spread;
            
            risk = (g_short_stop - g_short_start - spread) * PIPS;
            
            g_volume = g_loss / risk;

            ObjectCreate("start" + g_short_start, OBJ_TEXT, 0, Time[0], g_short_start-0.0005);
            ObjectSetText("start" + g_short_start, 
                "SHORT START: Patrimonio: " + g_patrimonio + 
                " Volume: " + g_volume, 8, "Times New Roman", White);
            
            ObjectCreate(str_start_short, OBJ_HLINE, 0, 0, g_short_start);
            ObjectSet(str_start_short, OBJPROP_COLOR, Blue);
            ObjectSet(str_start_short, OBJPROP_STYLE, STYLE_DOT);

            ObjectCreate(str_stop_short, OBJ_HLINE, 0, 0, g_short_stop);
            ObjectSet(str_stop_short, OBJPROP_COLOR, Red);
            ObjectSet(str_stop_short, OBJPROP_STYLE, STYLE_DOT);

            flag_short = true;
        }
    }

    /*
     * ST_BUY
     */
    if (flag_long == true) {

        if (candle_close > g_long_candle_high && is_high_candle()) {

            g_long_stop = l_price;
            
            ObjectDelete(str_stop_long);
         
            ObjectCreate(str_stop_long, OBJ_HLINE, 0, 0, l_price);
            ObjectSet(str_stop_long, OBJPROP_COLOR, Red);
            ObjectSet(str_stop_long, OBJPROP_STYLE, STYLE_DOT);
        }
    
        if (candle_high > g_long_candle_high) {
            g_long_candle_high = candle_high;
        }
    
        if (Bid <= g_long_stop) {
            lucro = (g_long_stop - (g_long_start + g_spread)) * PIPS * g_volume;
            per_lucro = lucro / g_patrimonio;
            g_patrimonio += lucro;

            ObjectCreate("start" + g_long_stop, OBJ_TEXT, 0, Time[0], g_long_stop-0.0005);
            ObjectSetText("start" + g_long_stop, 
                "LONG STOP: Patrimonio: " + g_patrimonio + 
                " Lucro/Perda: " + lucro +
                " Rent: " + per_lucro + "%", 8, "Times New Roman", White);
        
            ObjectDelete(str_start_long);
            ObjectDelete(str_stop_long);
            
            g_long_candle_high = 0;
            
            flag_long = false;
        }
    } 
    
    /*
     * ST_SELL
     */
    if (flag_short == true) {

        if (candle_close < g_short_candle_low && is_low_candle()) {
            
            g_short_stop = h_price;
            
            ObjectDelete(str_stop_short);
          
            ObjectCreate(str_stop_short, OBJ_HLINE, 0, 0, h_price);
            ObjectSet(str_stop_short, OBJPROP_COLOR, Red);
            ObjectSet(str_stop_short, OBJPROP_STYLE, STYLE_DOT);
        }
    
        if (candle_low < g_short_candle_low) {
            g_short_candle_low = candle_low;  
        }  

        if (Bid >= g_short_stop) {
            lucro = ((g_short_start - g_spread) - g_short_stop) * PIPS * g_volume;
            per_lucro = lucro / g_patrimonio;
            g_patrimonio += lucro;

            ObjectCreate("start" + g_short_stop, OBJ_TEXT, 0, Time[0], g_short_stop+0.0005);
            ObjectSetText("start" + g_short_stop, 
                "SHORT STOP: Patrimonio: " + g_patrimonio + 
                " Lucro/Perda: " + lucro + 
                " Rent: " + per_lucro + "%", 8, "Times New Roman", White);
            ObjectDelete(str_start_short);
            ObjectDelete(str_stop_short);

            g_short_candle_low = 999;

            flag_short = false;
        }
    }

    return(0);
}

/**********************************************************/

bool is_high_candle()
{
    bool ret = false;
    double candle_close = 0;
    double candle_open = 0;
    
    candle_open = iOpen(Symbol(), Period(), 1);
    candle_close = iClose(Symbol(), Period(), 1);

    if (candle_close > candle_open) {
        ret = true;
    }
    
    return (ret);
}

/**********************************************************/

bool is_low_candle()
{
    bool ret = false;
    double candle_close = 0;
    double candle_open = 0;
    
    candle_open = iOpen(Symbol(), Period(), 1);
    candle_close = iClose(Symbol(), Period(), 1);

    if (candle_close < candle_open) {
        ret = true;
    }
    
    return (ret);
}
//+------------------------------------------------------------------+