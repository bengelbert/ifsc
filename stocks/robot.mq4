//+------------------------------------------------------------------+
//|                                                        robot.mq4 |
//|                        Copyright 2014, MetaQuotes Software Corp. |
//|                                              http://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2014, MetaQuotes Software Corp."
#property link      "http://www.mql5.com"
#property version   "1.00"
#property strict

#include "StopAndReverse.mqh"

StopAndReverse sarM1;
StopAndReverse sarM5;
StopAndReverse sarM15;
StopAndReverse sarM30;
StopAndReverse sarH1;
StopAndReverse sarH4;
StopAndReverse sarD1;
StopAndReverse sarW1;
StopAndReverse sarMN1;
//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
{
    sarM1.onInit ("sarM1",  PERIOD_M1,  0.00003125);
    sarM5.onInit ("sarM5",  PERIOD_M5,  0.0000625 );
    sarM15.onInit("sarM15", PERIOD_M15, 0.000125  );
    sarM30.onInit("sarM30", PERIOD_M30, 0.00025   );
    sarH1.onInit ("sarH1",  PERIOD_H1,  0.0005    );
    sarH4.onInit ("sarH4",  PERIOD_H4,  0.001     );
    sarD1.onInit ("sarD1",  PERIOD_D1,  0.002     );
    sarW1.onInit ("sarW1",  PERIOD_W1,  0.004     );
    sarMN1.onInit("sarMN1", PERIOD_MN1, 0.008     );

    return(INIT_SUCCEEDED);
}
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
    Comment("Ops deu merda!!", reason);   
}
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
{
    sarM1.onTick();
    sarM5.onTick();
    sarM15.onTick();
    sarM30.onTick();
    sarH1.onTick();
    sarH4.onTick();
    sarD1.onTick();
    sarW1.onTick();
    sarMN1.onTick();
}
//+------------------------------------------------------------------+
