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

#define MAX_PERIOD  5

double zzBottom[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzBottomTime[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzBottomTimeFrame[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzTop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzTopTime[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzTopTimeFrame[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzTopBottomCount[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzState[MAX_PERIOD] = {ST_UNDEF,ST_UNDEF,ST_UNDEF,ST_UNDEF,ST_UNDEF};
double zzMedShadow[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzRisk[MAX_PERIOD] = {0.25,0.20,0.15,0.10,0.05,0.025,0.01,0.01,0.01};
double zzPerna[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zz33Start[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zz50Start[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zz60Start[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zz33Stop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zz50Stop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zz60Stop[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zzRiskGain[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zz50RiskGain[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
double zz60RiskGain[MAX_PERIOD] = {0,0,0,0,0,0,0,0};


//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
{
    int i;
    
    ObjectsDeleteAll();
    
    for (i = 0; i < MAX_PERIOD; i++) {
        ObjectDelete("top_"+getPeriodByIndex(i));
        ObjectDelete("bottom_"+getPeriodByIndex(i));    
    }
    
    ObjectCreate("label_start", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_start", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_start", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_start", OBJPROP_YDISTANCE, 15);// Y coordinate
    
    ObjectCreate("label_risk", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_risk", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_risk", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_risk", OBJPROP_YDISTANCE, 28);// Y coordinate

    ObjectCreate("label_state1", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_state1", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_state1", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_state1", OBJPROP_YDISTANCE, 41);// Y coordinate

    ObjectCreate("label_state2", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_state2", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_state2", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_state2", OBJPROP_YDISTANCE, 54);// Y coordinate

    ObjectCreate("label_state3", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_state3", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_state3", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_state3", OBJPROP_YDISTANCE, 67);// Y coordinate

    ObjectCreate("label_state4", OBJ_LABEL, 0, 0, 0);// Creating obj.
    ObjectSet("label_state4", OBJPROP_CORNER, 0);    // Reference corner
    ObjectSet("label_state4", OBJPROP_XDISTANCE, 10);// X coordinate
    ObjectSet("label_state4", OBJPROP_YDISTANCE, 80);// Y coordinate

    return(0);
}
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
int deinit()
  {
//----
   //ObjectsDeleteAll();
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
int start()
{
    int i;
    string desc;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    double swapLong = MarketInfo(Symbol(), MODE_SWAPLONG);
    double swapShort = MarketInfo(Symbol(), MODE_SWAPSHORT);
    
    calcMedShadow();
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
            ObjectSetText("top_"+getPeriodByIndex(i), desc+"_"+getPeriodNameByIndex(i), 7, "Arial", White);

            ObjectSetText("top_"+getPeriodByIndex(i-1), "", 7, "Arial", White);
            ObjectDelete("top_"+getPeriodByIndex(i-1));
        } else { 
            ObjectCreate("top_"+getPeriodByIndex(i), OBJ_TREND, 0, 0, zzTop[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_TIME1, zzTopTime[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_PRICE2, zzTop[i]);
            ObjectSet("top_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("top_"+getPeriodByIndex(i), 
                          "["+DoubleToStr(zzTop[i], 4)+"] TOP_"+getPeriodNameByIndex(i), 
                          7, "Arial", White);
        }
        
        if (i > 0 && zzBottom[i] == zzBottom[i-1]) {
            ObjectCreate("bottom_"+getPeriodByIndex(i), OBJ_TREND, 0, 0, zzBottom[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold); 
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_TIME1, zzBottomTime[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_PRICE2, zzBottom[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);

            desc = ObjectDescription("bottom_"+getPeriodByIndex(i-1));
            ObjectSetText("bottom_"+getPeriodByIndex(i), desc+"_"+getPeriodNameByIndex(i), 7, "Arial", White);

            ObjectSetText("bottom_"+getPeriodByIndex(i-1), "", 7, "Arial", White);
            ObjectDelete("bottom_"+getPeriodByIndex(i-1));
        } else { 
            ObjectCreate("bottom_"+getPeriodByIndex(i), OBJ_TREND, 0, 0, zzBottom[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_COLOR, Gold); 
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_TIME1, zzBottomTime[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_TIME2, Time[0]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_PRICE2, zzBottom[i]);
            ObjectSet("bottom_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
            ObjectSetText("bottom_"+getPeriodByIndex(i), 
                          "["+DoubleToStr(zzBottom[i], 4)+"] BOTTOM_"+getPeriodNameByIndex(i),
                          7, "Arial", White);
        }
        
        //33%
        if ((zz33Start[i] > 0) && 
            ((zzState[i] == ST_DOWN && Bid < zz33Start[i]) ||
            ((zzState[i] == ST_UP && Bid > zz33Start[i])))) {
                
                //Start
                ObjectCreate("zz33Start_"+getPeriodByIndex(i), OBJ_HLINE, 0, 0, zz33Start[i]);
            
                if (zzState[i] == ST_UP) {
                    ObjectSet("zz33Start_"+getPeriodByIndex(i), OBJPROP_COLOR, Green); 
                } else {
                    ObjectSet("zz33Start_"+getPeriodByIndex(i), OBJPROP_COLOR, Red); 
                }
                ObjectSet("zz33Start_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz33Start_"+getPeriodByIndex(i), 
                              "[33%] Start "+getPeriodNameByIndex(i),
                              7, "Arial", White);
                
                //Stop
                ObjectCreate("zz33Stop_"+getPeriodByIndex(i), OBJ_HLINE, 0, 0, zz33Stop[i]);
            
                if (zzState[i] == ST_UP) {
                    ObjectSet("zz33Stop_"+getPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                } else {
                    ObjectSet("zz33Stop_"+getPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                }
                ObjectSet("zz33Stop_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz33Stop_"+getPeriodByIndex(i), 
                              "[33%] Stop "+getPeriodNameByIndex(i),
                              7, "Arial", White);
        } else {
            ObjectDelete("zz33Start_"+getPeriodByIndex(i));
            ObjectDelete("zz33Stop_"+getPeriodByIndex(i));
        }

        //50%
        if ((zz50Start[i] > 0) && 
            ((zzState[i] == ST_DOWN && Bid < zz50Start[i]) ||
            ((zzState[i] == ST_UP && Bid > zz50Start[i])))) {
                
                ObjectCreate("zz50Start_"+getPeriodByIndex(i), OBJ_HLINE, 0, 0, zz50Start[i]);
            
                if (zzState[i] == ST_UP) {
                    ObjectSet("zz50Start_"+getPeriodByIndex(i), OBJPROP_COLOR, Green); 
                } else {
                    ObjectSet("zz50Start_"+getPeriodByIndex(i), OBJPROP_COLOR, Red); 
                }
                ObjectSet("zz50Start_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz50Start_"+getPeriodByIndex(i), 
                              "[50%] Start "+getPeriodNameByIndex(i),
                              7, "Arial", White);

                //Stop
                ObjectCreate("zz50Stop_"+getPeriodByIndex(i), OBJ_HLINE, 0, 0, zz50Stop[i]);
            
                if (zzState[i] == ST_UP) {
                    ObjectSet("zz50Stop_"+getPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                } else {
                    ObjectSet("zz50Stop_"+getPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                }
                ObjectSet("zz50Stop_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz50Stop_"+getPeriodByIndex(i), 
                              "[50%] Stop "+getPeriodNameByIndex(i),
                              7, "Arial", White);
        } else {
            ObjectDelete("zz50Start_"+getPeriodByIndex(i));
            ObjectDelete("zz50Stop_"+getPeriodByIndex(i));
        }

        //60%
        if ((zz60Start[i] > 0) && 
            ((zzState[i] == ST_DOWN && Bid < zz60Start[i]) ||
            ((zzState[i] == ST_UP && Bid > zz60Start[i])))) {
                
                ObjectCreate("zz60Start_"+getPeriodByIndex(i), OBJ_HLINE, 0, 0, zz60Start[i]);
            
                if (zzState[i] == ST_UP) {
                    ObjectSet("zz60Start_"+getPeriodByIndex(i), OBJPROP_COLOR, Green); 
                } else {
                    ObjectSet("zz60Start_"+getPeriodByIndex(i), OBJPROP_COLOR, Red); 
                }
                ObjectSet("zz60Start_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz60Start_"+getPeriodByIndex(i), 
                              "[60%] Start "+getPeriodNameByIndex(i),
                              7, "Arial", White);

                //Stop
                ObjectCreate("zz60Stop_"+getPeriodByIndex(i), OBJ_HLINE, 0, 0, zz60Stop[i]);
            
                if (zzState[i] == ST_UP) {
                    ObjectSet("zz60Stop_"+getPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                } else {
                    ObjectSet("zz60Stop_"+getPeriodByIndex(i), OBJPROP_COLOR, Blue); 
                }
                ObjectSet("zz60Stop_"+getPeriodByIndex(i), OBJPROP_STYLE, 2);
                ObjectSetText("zz60Stop_"+getPeriodByIndex(i), 
                              "[60%] Stop "+getPeriodNameByIndex(i),
                              7, "Arial", White);
        } else {
            ObjectDelete("zz60Start_"+getPeriodByIndex(i));
            ObjectDelete("zz60Stop_"+getPeriodByIndex(i));
        }
        
    }

    ObjectSetText("label_start",
                  "SPREAD: " + DoubleToStr(spread, 0) + 
                  " SWLONG: " + DoubleToStr(swapLong, 2) + 
                  " SWSHORT: " + DoubleToStr(swapShort, 2),
                  //" risk: " + DoubleToStr(getRisk(medLowShadow) * 100, 0) + " %", 
                  7, "Arial", White);

    ObjectSetText("label_risk",
                  "RISK: H1(" + DoubleToStr(getRisk(4)*100,0) + "%)" +
                  " H4(" + DoubleToStr(getRisk(3)*100,0) + "%)" +
                  " D1(" + DoubleToStr(getRisk(2)*100,0) + "%)" +
                  " W1(" + DoubleToStr(getRisk(1)*100,0) + "%)" +
                  " MN1(" + DoubleToStr(getRisk(0)*100,0) + "%)", 
                  7, "Arial", White);

    ObjectSetText("label_state1",
                  "TEND: H1(" + getStateName(zzState[4]) + ")" +
                  " H4(" + getStateName(zzState[3]) + ")" +
                  " D1(" + getStateName(zzState[2]) + ")" +
                  " W1(" + getStateName(zzState[1]) + ")" +
                  " MN1(" + getStateName(zzState[0]) + ")", 
                  7, "Arial", White);

    ObjectSetText("label_state2",
                  "RG: H1(" + DoubleToStr(zzRiskGain[4], 1) + ")" +
                  " H4(" + DoubleToStr(zzRiskGain[3], 1) + ")" +
                  " D1(" + DoubleToStr(zzRiskGain[2], 1) + ")" +
                  " W1(" + DoubleToStr(zzRiskGain[1], 1) + ")" +
                  " MN1(" + DoubleToStr(zzRiskGain[0], 1) + ")", 
                  7, "Arial", White);

    return(0);
  }
//+------------------------------------------------------------------+

bool is_high_clande(int timeframe, int shift)
{
    bool ret = false;
    double cdlOpen;
    double cdlClose;
    
    cdlOpen = iOpen(Symbol(), timeframe, shift);
    cdlClose = iClose(Symbol(), timeframe, shift);
   
    if (cdlClose > cdlOpen) {
        ret = true;
    }
   
    return (ret); 
}

double getRisk(int index)
{
    double med = 0;
    double risk = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    
    switch (zzState[index]) {
        case ST_UNDEF: risk = 0; break;
        case ST_CONG: risk = zzRisk[index]; break;
        case ST_UP: med = zzMedShadow[index]; break;
        case ST_DOWN: med = zzMedShadow[index]; break;
    }
    
    if (zzState[index] == ST_UP || zzState[index] == ST_DOWN) {
        if (med > spread * 8) {
            risk = 0.16;    
        } else if ((med > spread * 4) && (med < spread * 8)) {
            risk = 0.08;
        } else if ((med > spread * 2) && (med < spread * 4)) {
            risk = 0.04;
        } else if ((med > spread * 1) && (med < spread * 2)) {
            risk = 0.02;
        } else {
            risk = 0.01;
        } 
    }
    
    return (risk);
}

void wtsCalcZigzag(string strPeriod, int period, int index)
{
    int i;
    double zz = 0;
    int zzTotal = 0;
    int spread = MarketInfo(Symbol(), MODE_SPREAD);
    int shift[MAX_PERIOD] = {0,0,0,0,0,0,0,0};
    double zzBuffer[200];
    double zzBufferClose[200];
    double zzBufferTime[200];
    double zzShift[200];
    double pernaPerc = 0;
    double startPerc = 0;

    for (i = WindowBarsPerChart()*2; i != 0; i--) {
        zz = iCustom(Symbol(), period, "ZigZag", 0, i);
        if (zz != 0) {
            zzBuffer[zzTotal] = zz;
            zzBufferTime[zzTotal] = iTime(Symbol(), period, i);
            zzBufferClose[zzTotal] = iClose(Symbol(), period, i);
            zzShift[zzTotal] = i;
            zzTotal++;
        }
    } 
    
    if (zzBuffer[0] > zzBuffer[1]) {
        zzTop[index] = zzBuffer[0];
        zzBottom[index] = zzBuffer[1];
        zzTopTime[index] = zzBufferTime[0];
        zzBottomTime[index] = zzBufferTime[1];
        zzState[index] = ST_DOWN;
        shift[index] = zzShift[1];
    } else {
        zzTop[index] = zzBuffer[1];
        zzBottom[index] = zzBuffer[0];
        zzTopTime[index] = zzBufferTime[1];
        zzBottomTime[index] = zzBufferTime[0];
        zzState[index] = ST_UP;
        shift[index] = zzShift[0];
    }
    
    zzTopBottomCount[index] = 0;
    
    for (i = 2; i < zzTotal; i++) {
        zzTopBottomCount[index]++;
        
        if (zzBuffer[i] > zzTop[index] && zzBufferClose[i] > zzTop[index]) {
            zzTop[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i];
            zzBottom[index] = zzBuffer[i-1];
            zzBottomTime[index] = zzBufferTime[i-1];
            zzState[index] = ST_UP;
            zzTopBottomCount[index] = 0;
            shift[index] = zzShift[i];
        } else if (zzBuffer[i] > zzTop[index]) {
            zzTop[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i];
            zzBottom[index] = zzBuffer[i-1];
            zzBottomTime[index] = zzBufferTime[i-1];
            zzState[index] = ST_CONG;
            zzTopBottomCount[index] = 0;
            shift[index] = zzShift[i];
        }
        
        if (zzBuffer[i] < zzBottom[index] && zzBufferClose[i] < zzBottom[index]) {
            zzTop[index] = zzBuffer[i-1];
            zzBottom[index] = zzBuffer[i];
            zzTopTime[index] = zzBufferTime[i-1];
            zzBottomTime[index] = zzBufferTime[i];
            zzState[index] = ST_DOWN;
            zzTopBottomCount[index] = 0;
            shift[index] = zzShift[i];
        } else if (zzBuffer[i] < zzBottom[index]) {
            zzTop[index] = zzBuffer[i-1];
            zzTopTime[index] = zzBufferTime[i-1];
            zzBottom[index] = zzBuffer[i];
            zzBottomTime[index] = zzBufferTime[i];
            zzState[index] = ST_CONG;
            zzTopBottomCount[index] = 0;
            shift[index] = zzShift[i];
        }
    }

    if (zzTopBottomCount[index] > 1) {
        zzState[index] = ST_UNDEF;
    }

    zzPerna[index] = (zzTop[index] - zzBottom[index]) / Point;

    //33%    
    if (zzState[index] == ST_UP) {
        pernaPerc = zzTop[index] / zzBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        zz33Start[index] = (zzTop[index] - (zzPerna[index] * 0.33 * Point)) * (1 - startPerc);
        zz33Stop[index] = zz33Start[index] - ((2 * zzMedShadow[index]) * Point);
    } else if (zzState[index] == ST_DOWN) {
        pernaPerc = 1 - zzBottom[index] / zzTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        zz33Start[index] = (zzBottom[index] + (zzPerna[index] * 0.33 * Point)) * (1 + startPerc);
        zz33Stop[index] = zz33Start[index] + ((2 * zzMedShadow[index] + spread) * Point);
    } else {
        zz33Start[index] = 0;
        zz33Stop[index] = 0;
    }

    //50%
    if (zzState[index] == ST_UP) {
        pernaPerc = zzTop[index] / zzBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        zz50Start[index] = (zzTop[index] - (zzPerna[index] * 0.5 * Point)) * (1 - startPerc);
        zz50Stop[index] = zz50Start[index] - ((2 * zzMedShadow[index]) * Point);
    } else if (zzState[index] == ST_DOWN) {
        pernaPerc = 1 - zzBottom[index] / zzTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        zz50Start[index] = (zzBottom[index] + (zzPerna[index] * 0.5 * Point)) * (1 + startPerc);
        zz50Stop[index] = zz50Start[index] + ((2 * zzMedShadow[index] + spread) * Point);
    } else {
        zz50Start[index] = 0;
        zz50Start[index] = 0;
    }

    //60%
    if (zzState[index] == ST_UP) {
        pernaPerc = zzTop[index] / zzBottom[index] - 1;
        startPerc = pernaPerc * 0.0025 / 0.1;
        zz60Start[index] = (zzTop[index] - (zzPerna[index] * 0.6 * Point)) * (1 - startPerc);
        zz60Stop[index] = zz60Start[index] - ((2 * zzMedShadow[index]) * Point);
    } else if (zzState[index] == ST_DOWN) {
        pernaPerc = 1 - zzBottom[index] / zzTop[index];
        startPerc = pernaPerc * 0.0025 / 0.1;
        zz60Start[index] = (zzBottom[index] + (zzPerna[index] * 0.6 * Point)) * (1 + startPerc);
        zz60Stop[index] = zz60Start[index] + ((2 * zzMedShadow[index] + spread) * Point);
    } else {
        zz60Start[index] = 0;
        zz60Start[index] = 0;
    }
    
    for (i = shift[index]; i != 0; i--) {
        if ((zzState[index] == ST_UP && iLow(Symbol(), period, i) < zz60Start[index]) ||
            (zzState[index] == ST_DOWN && iHigh(Symbol(), period, i) > zz60Start[index])) {
            zz33Start[index] = 0;
            zz50Start[index] = 0;
            zz60Start[index] = 0;
            zzState[index] = ST_UNDEF;
        } else if ((zzState[index] == ST_UP && iLow(Symbol(), period, i) < zz50Start[index]) ||
            (zzState[index] == ST_DOWN && iHigh(Symbol(), period, i) > zz50Start[index])) {
            zz33Start[index] = 0;
            zz50Start[index] = 0;
        } else if ((zzState[index] == ST_UP && iLow(Symbol(), period, i) < zz33Start[index]) ||
            (zzState[index] == ST_DOWN && iHigh(Symbol(), period, i) > zz33Start[index])) {
            zz33Start[index] = 0;
        } 
    }    

    zz33Start[index] = NormalizeDouble(zz33Start[index] ,4);
    zz50Start[index] = NormalizeDouble(zz50Start[index] ,4);
    zz60Start[index] = NormalizeDouble(zz60Start[index] ,4);

    zz33Stop[index] = NormalizeDouble(zz33Stop[index] ,4);
    zz50Stop[index] = NormalizeDouble(zz50Stop[index] ,4);
    zz60Stop[index] = NormalizeDouble(zz60Stop[index] ,4);
    
    if (zz33Start[index] != 0 && zz33Stop[index] != 0) {
        if (zzState[index] == ST_UP) {
            zzRiskGain[index] = zzPerna[index] / ((zz33Start[index] - zz33Stop[index]) / Point);
        } else {
            zzRiskGain[index] = zzPerna[index] / ((zz33Stop[index] - zz33Start[index]) / Point);
        }
    } else if (zz50Start[index] != 0 && zz50Stop[index] != 0) {
        if (zzState[index] == ST_UP) {
            zzRiskGain[index] = zzPerna[index] / ((zz50Start[index] - zz50Stop[index]) / Point);
        } else {
            zzRiskGain[index] = zzPerna[index] / ((zz50Stop[index] - zz50Start[index]) / Point);
        }
    } else if (zz60Start[index] != 0 && zz60Stop[index] != 0) {
        if (zzState[index] == ST_UP) {
            zzRiskGain[index] = zzPerna[index] / ((zz60Start[index] - zz60Stop[index]) / Point);
        } else {
            zzRiskGain[index] = zzPerna[index] / ((zz60Stop[index] - zz60Start[index]) / Point);
        }
    } else {
        zzRiskGain[index] = 0;
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
        case 5: ret = PERIOD_M30; break;
        case 6: ret = PERIOD_M15; break;
        case 7: ret = PERIOD_M5; break;
        case 8: ret = PERIOD_M1; break;
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
        case 5: ret = "M30"; break;
        case 6: ret = "M15"; break;
        case 7: ret = "M5"; break;
        case 8: ret = "M1"; break;
    }

    return (ret);
}


string getStateName(int state)
{
    string name = "";
    
    switch (state) {
        case ST_UNDEF:  name = "--"; break;
        case ST_CONG:   name = "cg"; break;
        case ST_UP:     name = "up"; break;
        case ST_DOWN:   name = "dw"; break;
    }
    
    return (name);
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
        case PERIOD_M30: ret = 5; break;
        case PERIOD_M15: ret = 6; break;
        case PERIOD_M5: ret = 7; break;
        case PERIOD_M1: ret = 8; break;
    }

    return (ret);
}

void calcMedShadow()
{
    int i, j;
    int medTotal = WindowBarsPerChart();
    double medLowShadow = 0;
    double medHighShadow = 0;
    double spread = MarketInfo(Symbol(), MODE_SPREAD);
    double cdlOpenCloseLow;
    double cdlOpenCloseHigh;
    double cdlLow;
    double cdlHigh;

    for (j = 0; j < MAX_PERIOD; j++) {
        
        medLowShadow = 0;
        medHighShadow = 0;
        
        for (i = medTotal; i != 0; i--) {
    
            if (is_high_clande(getPeriodByIndex(j), i)) {
                cdlOpenCloseLow = iOpen(Symbol(), getPeriodByIndex(j), i);
                cdlOpenCloseHigh = iClose(Symbol(), getPeriodByIndex(j), i);
            } else {
                cdlOpenCloseLow = iClose(Symbol(), getPeriodByIndex(j), i);
                cdlOpenCloseHigh = iOpen(Symbol(), getPeriodByIndex(j), i);
            }
            
            cdlLow = iLow(Symbol(), getPeriodByIndex(j), i);
            cdlHigh = iHigh(Symbol(), getPeriodByIndex(j), i);
        
            medLowShadow += cdlOpenCloseLow - cdlLow;
            medHighShadow += cdlHigh - cdlOpenCloseHigh;
        }
    
        medLowShadow /= medTotal;
        medLowShadow = NormalizeDouble(medLowShadow / Point, 0);
    
        if (medLowShadow < spread) {
            medLowShadow = spread;
        }

        medHighShadow /= medTotal;
        medHighShadow = NormalizeDouble(medHighShadow / Point, 0);
    
        if (medHighShadow < spread) {
            medHighShadow = spread;
        }
        
        zzMedShadow[j] = (medLowShadow + medHighShadow) / 2;
    }
}