//+------------------------------------------------------------------+
//|                                                          wts.mq4 |
//|                        Copyright 2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#property copyright "Copyright 2013, MetaQuotes Software Corp."
#property link      "http://www.metaquotes.net"

#define ST_UNDEF    0
#define ST_CONG     1
#define ST_UP       2
#define ST_DOWN     3


double zzBottom[5] = {0,0,0,0,0};
double zzBottomTime[5] = {0,0,0,0,0};
double zzTop[5] = {0,0,0,0,0};
double zzTopTime[5] = {0,0,0,0,0};
double zzState[5] = {ST_UNDEF,ST_UNDEF,ST_UNDEF,ST_UNDEF,ST_UNDEF};

//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
{
    ObjectsDeleteAll();
    ObjectCreate("label_state", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_state", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_state", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_state", OBJPROP_YDISTANCE, 15);// Y coordinate

    int i;
    string desc;
    
    wtsCalcZigzag("MN1", PERIOD_MN1, 0);
    wtsCalcZigzag("W1", PERIOD_W1, 1);
    wtsCalcZigzag("D1", PERIOD_D1, 2);
    wtsCalcZigzag("H4", PERIOD_H4, 3);
    wtsCalcZigzag("H1", PERIOD_H1, 4);
    
    for (i = 0; i <= getIndexByPeriod(Period()); i++) {

        if (i > 0 && zzTop[i] == zzTop[i-1]) {
            ObjectCreate("top_"+getPeriodByIndex(i), OBJ_TREND, 0, 0, zzTop[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold); 
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_TIME1, zzTopTime[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_PRICE2, zzTop[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);

            desc = ObjectDescription("top_"+getPeriodByIndex(i-1));
            ObjectSetText("top_"+getPeriodByIndex(i), desc+"_"+getPeriodNameByIndex(i));

            ObjectSetText("top_"+getPeriodByIndex(i-1), "");
            ObjectDelete("top_"+getPeriodByIndex(i-1));
        } else { 
            ObjectCreate("top_"+getPeriodByIndex(i), OBJ_TREND, 0, 0, zzTop[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_TIME1, zzTopTime[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_PRICE2, zzTop[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("top_"+getPeriodByIndex(i), "TOP_"+getPeriodNameByIndex(i));
        }
        
        if (i > 0 && zzBottom[i] == zzBottom[i-1]) {
            ObjectCreate("bottom_"+getPeriodByIndex(i), OBJ_TREND, 0, 0, zzBottom[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold); 
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_TIME1, zzBottomTime[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_PRICE2, zzBottom[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);

            desc = ObjectDescription("bottom_"+getPeriodByIndex(i-1));
            ObjectSetText("bottom_"+getPeriodByIndex(i), desc+"_"+getPeriodNameByIndex(i));

            ObjectSetText("bottom_"+getPeriodByIndex(i-1), "");
            ObjectDelete("bottom_"+getPeriodByIndex(i-1));
        } else { 
            ObjectCreate("bottom_"+getPeriodByIndex(i), OBJ_TREND, 0, 0, zzBottom[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold); 
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_TIME1, zzBottomTime[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_PRICE2, zzBottom[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("bottom_"+getPeriodByIndex(i), "BOTTOM_"+getPeriodNameByIndex(i));
        }
    }
       
    ObjectSetText("label_state", "H1: "+getStateName(zzState[4]) +
                  " H4: "+getStateName(zzState[3]) + 
                  " D1: "+getStateName(zzState[2]) + 
                  " W1: "+getStateName(zzState[1]) + 
                  " MN1: "+getStateName(zzState[0]), 7, "Arial", White);
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
    
    return(0);
}
//+------------------------------------------------------------------+

void wtsCalcZigzag(string strPeriod, int period, int index)
{
    int i;
    double zz = 0;
    int zzTotal = 0;
    double zzBuffer[200];
    double zzBufferClose[200];
    double zzBufferTime[200];

    for (i = WindowBarsPerChart() * 2; i >= 0; i--) {
        zz = iCustom(Symbol(), period, "ZigZag", 0, i);
        if (zz != 0) {
            zzBuffer[zzTotal] = zz;
            zzBufferClose[zzTotal] = iClose(Symbol(), period, i);
            zzBufferTime[zzTotal] = iTime(Symbol(), period, i);
            
            zzTotal++;
        }
    } 
    
    if (zzBuffer[0] > zzBuffer[1]) {
        zzTop[index] = zzBuffer[0];
        zzBottom[index] = zzBuffer[1];
        zzTopTime[index] = zzBufferTime[0];
        zzBottomTime[index] = zzBufferTime[1];
    } else {
        zzTop[index] = zzBuffer[1];
        zzBottom[index] = zzBuffer[0];
        zzTopTime[index] = zzBufferTime[1];
        zzBottomTime[index] = zzBufferTime[0];
    }
    
    for (i = 2; i < zzTotal; i++) {
        
        if (zzBuffer[i] > zzTop[index] && zzBufferClose[i] > zzTop[index]) {
            zzTop[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i];
            zzBottom[index] = zzBuffer[i-1];
            zzBottomTime[index] = zzBufferTime[i-1];
            zzState[index] = ST_UP;
        } else if (zzBuffer[i] > zzTop[index]) {
            zzTop[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i];
            zzBottom[index] = zzBuffer[i-1];
            zzBottomTime[index] = zzBufferTime[i-1];
            zzState[index] = ST_CONG;
        }
        
        if (zzBuffer[i] < zzBottom[index] && zzBufferClose[i] < zzBottom[index]) {
            zzTop[index] = zzBuffer[i-1];
            zzBottom[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i-1];
            zzBottomTime[index] = zzBufferTime[i];
            zzState[index] = ST_DOWN;
        } else if (zzBuffer[i] < zzBottom[index]) {
            zzTop[index] = zzBuffer[i-1];
            zzTopTime[index] = zzBufferTime[i-1];
            zzBottom[index] = zzBuffer[i];
            zzBottomTime[index] = zzBufferTime[i];
            zzState[index] = ST_CONG;
        }
    }

}

int getPeriodByIndex(int index) 
{
    int ret = 0;
    
    switch (index) {
        case 0: ret = PERIOD_MN1; break;
        case 1: ret = PERIOD_W1; break;
        case 2: ret = PERIOD_D1; break;
        case 3: ret = PERIOD_H4; break;
        case 4: ret = PERIOD_H1; break;
    }

    return (ret);
}

string getPeriodNameByIndex(int index) 
{
    string ret = 0;
    
    switch (index) {
        case 0: ret = "MN1"; break;
        case 1: ret = "W1"; break;
        case 2: ret = "D1"; break;
        case 3: ret = "H4"; break;
        case 4: ret = "H1"; break;
    }

    return (ret);
}

string getStateName(int state)
{
    string ret = "";
    
    switch (state) {
        case ST_UNDEF:  ret = "UNDEF"; break;   
        case ST_CONG:  ret = "CONG"; break;
        case ST_UP:  ret = "UP"; break;
        case ST_DOWN:  ret = "DOWN"; break;
    }
    
    return (ret);
}

int getIndexByPeriod(int period) 
{
    int ret = 0;
    
    switch (period) {
        case PERIOD_MN1: ret = 0; break;
        case PERIOD_W1: ret = 1; break;
        case PERIOD_D1: ret = 2; break;
        case PERIOD_H4: ret = 3; break;
        case PERIOD_H1: ret = 4; break;
    }

    return (ret);
}


















