//+------------------------------------------------------------------+
//|                                                           wt.mq4 |
//|                        Copyright 2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#property copyright "Copyright 2013, MetaQuotes Software Corp."
#property link      "http://www.metaquotes.net"

#define FIBO_NAME   "fibo"
#define INFO_NAME   "info"
#define TREND_START "trend_start"
#define TREND_STOP  "trend_stop"

static int gModeSpread = 0;

static bool gBuySignal = false;
static bool gSellSignal = false;


static int gPivotLen = 0;
static double gPivotEnd = 0;
static double gPivotInit = 0;


static string gInfoDirection = "---";

int init()
{
    double low = 0;
    double high = 0;

    low = iLow(Symbol(), 0, 0);
    high = iHigh(Symbol(), 0, 0);

    ObjectDelete(FIBO_NAME);
    ObjectCreate(FIBO_NAME, OBJ_FIBO, 0, TimeCurrent(), high, TimeCurrent(), low);
    ObjectSet(FIBO_NAME, OBJPROP_LEVELSTYLE, STYLE_DOT);
    ObjectSet(FIBO_NAME, OBJPROP_FIBOLEVELS, 5);
    ObjectSet(FIBO_NAME, OBJPROP_FIRSTLEVEL+1, 0.4);
    ObjectSet(FIBO_NAME, OBJPROP_FIRSTLEVEL+2, 0.5);
    ObjectSet(FIBO_NAME, OBJPROP_FIRSTLEVEL+3, 0.67);
    ObjectSet(FIBO_NAME, OBJPROP_FIRSTLEVEL+4, 1.0);

    ObjectDelete(TREND_START);
    ObjectCreate(TREND_START, OBJ_TREND, 0, TimeCurrent(), high, TimeCurrent(), high);
    ObjectSet(TREND_START, OBJPROP_COLOR, Green);

    ObjectDelete(TREND_STOP);
    ObjectCreate(TREND_STOP, OBJ_TREND, 0, TimeCurrent(), low, TimeCurrent(), low);
    ObjectSet(TREND_STOP, OBJPROP_COLOR, Red);
    
    ObjectCreate(INFO_NAME, OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet(INFO_NAME, OBJPROP_CORNER, 0); // Reference corner
    ObjectSet(INFO_NAME, OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet(INFO_NAME, OBJPROP_YDISTANCE, 15); // Y coordinate

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
    wtGetMarketInfo();
    wtCheckOpenOrders();
    wtCheckDirection();
    wtDecisionCriteria();
    wtEnterMarket();
    wtPrintInfo();
    
    return(0);
}

/*****/

int wtCheckDirection()
{
    datetime timePivotEnd = 0;
    datetime timePivotInit = 0;
    int barPivotStart = 0;
    
    timePivotEnd = ObjectGet(FIBO_NAME, OBJPROP_TIME1);
    timePivotInit = ObjectGet(FIBO_NAME, OBJPROP_TIME2);
    
    if (timePivotEnd < timePivotInit) {
        gBuySignal = true;
    } else {
        gBuySignal = false;
    }
    
    if (timePivotInit < timePivotEnd) {
        gSellSignal = true;
    } else {
        gSellSignal = false;
    }
    
    if (gBuySignal == true) {
        
        gPivotEnd = ObjectGet(FIBO_NAME, OBJPROP_PRICE2);
        gPivotInit = ObjectGet(FIBO_NAME, OBJPROP_PRICE1);
        gPivotLen = (gPivotEnd - gPivotInit) / Point;
        gInfoDirection = "Buy";
                
        ObjectSet(TREND_START, OBJPROP_TIME1, timePivotEnd);
        ObjectSet(TREND_START, OBJPROP_PRICE1, gPivotEnd);
        ObjectSet(TREND_STOP, OBJPROP_TIME1, timePivotInit);
        ObjectSet(TREND_STOP, OBJPROP_PRICE1, gPivotInit);
        
        
        
    } else if (gSellSignal == true) {
        
        gPivotEnd = ObjectGet(FIBO_NAME, OBJPROP_PRICE1);
        gPivotInit = ObjectGet(FIBO_NAME, OBJPROP_PRICE2);
        gPivotLen = (gPivotInit - gPivotEnd) / Point;
        gInfoDirection = "Sell";
      
        barPivotStart = iHighest(Symbol(), 0, MODE_HIGH, iBarShift(Symbol(), 0, timePivotEnd, 0)-1, 1);
        
        Print("bar", barPivotStart);
        Print("ibar", iBarShift(Symbol(), 0, timePivotEnd, 0));
      
        ObjectSet(TREND_START, OBJPROP_TIME1, timePivotEnd);
        ObjectSet(TREND_START, OBJPROP_PRICE1, gPivotEnd);
        ObjectSet(TREND_START, OBJPROP_TIME2, iTime(Symbol(), 0, barPivotStart));
        ObjectSet(TREND_START, OBJPROP_PRICE2, iLow(Symbol(), 0, barPivotStart));
        
        ObjectSet(TREND_STOP, OBJPROP_TIME1, timePivotInit);
        ObjectSet(TREND_STOP, OBJPROP_PRICE1, gPivotInit);
        ObjectSet(TREND_STOP, OBJPROP_TIME2, iTime(Symbol(), 0, barPivotStart));
        ObjectSet(TREND_STOP, OBJPROP_PRICE2, iHigh(Symbol(), 0, barPivotStart));
        
    } else {
        gPivotEnd = 0;
        gPivotLen = 0;
        gPivotInit = 0;
        gInfoDirection = "---";
    }
}

int wtCheckOpenOrders()
{
    int i;
    
    for(i = 0; i < OrdersTotal(); i++) {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES); 

        if(OrderSymbol() == Symbol()) {
            if (OrderType() == OP_BUY) {
                //gblLongTradeBlock = true;
            }
        }
    }
}

int wtDecisionCriteria()
{
    if (gBuySignal == true) {
        
    }
}

int wtEnterMarket()
{
    //wtPrintLine("Start", value, int colored, string comment)
}

int wtGetMarketInfo()
{
    gModeSpread = MarketInfo(Symbol(), MODE_SPREAD);
}

double wtGetRisk(int timeframe)
{
    double risk = 0.005;

    switch (timeframe) {
    case PERIOD_M30: risk = 0.005; break;
    case PERIOD_H1: risk = 0.01; break;
    case PERIOD_H4: risk = 0.02; break;
    case PERIOD_D1: risk = 0.04; break;
    case PERIOD_W1: risk = 0.08; break;
    case PERIOD_MN1: risk = 0.16; break;
    }
    
    return (risk);
}

int wtGetTimeframe(string timeframeName)
{
    int timeframe = PERIOD_D1;

    if (StringLen(timeframeName) > 0) {
        if (StringFind(timeframeName, "M30") == 0) {
            timeframe = PERIOD_M30;
        }

        if (StringFind(timeframeName, "H1") == 0) {
            timeframe = PERIOD_H1;
        }        
    }
    
    return (timeframe);
}

int wtPrintInfo()
{
    ObjectSetText(INFO_NAME,
        ""+gInfoDirection+": "+
        "Len("+gPivotLen+")",
        7, "Arial", White);
}

int wtPrintLine(string name, double value, int colored, string comment)
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

