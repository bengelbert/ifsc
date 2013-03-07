//+------------------------------------------------------------------+
//|                                                          wts.mq4 |
//|                        Copyright 2013, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#property copyright "Copyright 2013, MetaQuotes Software Corp."
#property link      "http://www.metaquotes.net"
#property indicator_chart_window

double zzBottom[5] = {0,0,0,0,0};
double zzTop[5] = {0,0,0,0,0};

//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
{
    int i;
    
    wtsCalcZigzag("MN1", PERIOD_MN1, 0);
    wtsCalcZigzag("W1", PERIOD_W1, 1);
    wtsCalcZigzag("D1", PERIOD_D1, 2);
    wtsCalcZigzag("H4", PERIOD_H4, 3);
    wtsCalcZigzag("H1", PERIOD_H1, 4);

//    iCustom(Symbol(), 0, "ZUP_v64", 0, 0);
    
    for (i = 0; i < 5; i++) {
        ObjectDelete("top_"+getPeriodByIndex(i));
        ObjectDelete("bottom_"+getPeriodByIndex(i));

        ObjectCreate("top_"+getPeriodByIndex(i), OBJ_HLINE, 0, 0, zzTop[i]);
        ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold);

        ObjectCreate("bottom_"+getPeriodByIndex(i), OBJ_HLINE, 0, 0, zzBottom[i]);
        ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold); 

        ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_WIDTH, 5-i);
        ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_WIDTH, 5-i);
    }
      
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

void wtsCalcZigzag(string strPeriod, int period, int index)
{
    int i;
    double zz = 0;
    int zzTotal = 0;
    double zzBuffer[100];

    for (i = WindowBarsPerChart(); i >= 0; i--) {
        zz = iCustom(Symbol(), period, "ZUP_v64", 0, i);
        if (zz != 0) {
            zzBuffer[zzTotal] = zz;
            zzTotal++;
        }
    } 
    
    if (zzBuffer[0] > zzBuffer[1]) {
        zzTop[index] = zzBuffer[0];
        zzBottom[index] = zzBuffer[1];
    } else {
        zzTop[index] = zzBuffer[1];
        zzBottom[index] = zzBuffer[0];
    }
    
    for (i = 2; i < zzTotal; i++) {
        
        if (zzBuffer[i] > zzTop[index]) {
            zzBottom[index] = zzBuffer[i-1];
            zzTop[index] = zzBuffer[i];
        } 
        
        if (zzBuffer[i] < zzBottom[index]) {
            zzTop[index] = zzBuffer[i-1];
            zzBottom[index] = zzBuffer[i];
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


