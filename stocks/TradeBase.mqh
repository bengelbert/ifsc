//+------------------------------------------------------------------+
//|                                                    TradeBase.mqh |
//|                        Copyright 2014, MetaQuotes Software Corp. |
//|                                              http://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2014, MetaQuotes Software Corp."
#property link      "http://www.mql5.com"
#property version   "1.00"
#property strict

#include "Candle.mqh"
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
#define TRADEBASE_SELL  "SELL"
#define TRADEBASE_BUY   "BUY"
#define TRADEBASE_WAIT  "WAIT"
#define TRADEBASE_MAGIC 5555

class TradeBase : public Candle
  {
private:
    string mName;
    double mRisk;
    string mTradeType;
    double mStart;
    double mStop;
    int    mSpread;
    double mLots;
    
public:
    TradeBase();
    ~TradeBase();
    
    int    getTotalOrders();
    double getLots() { return (mLots); };
    string getName() { return (mName); };
    double getRisk() { return (mRisk); };
    int    getSpread() { return (mSpread); };
    double getStart() { return (mStart); };
    string getTradeType() { return (mTradeType); };
    int    onInit(string name, int timeframe, double risk);
    void   setStart(int shift=0);
    void   setLots(double lots);
    void   setTradeType(string type) { mTradeType = type; };
    
    virtual int onTick(void);
};
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
TradeBase::TradeBase()
{
    mName = "NULL";
    mRisk = 0;
    mSpread = (int) MarketInfo(Symbol(), MODE_SPREAD);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
TradeBase::~TradeBase()
  {
  }
//+------------------------------------------------------------------+

int TradeBase::onInit(string name,int timeframe,double risk)
{
    mName = name;
    setTimeFrame(timeframe);
    mRisk = risk;

    return (0);
}

//+------------------------------------------------------------------+

void TradeBase::setStart(int shift)
{
    if (getTradeType() == TRADEBASE_BUY)
    {
        mStart = NormalizeDouble(getHigh(shift) + (getAverageCandleSize() / 3) * Point, Digits-1);
    }
    else if (getTradeType() == TRADEBASE_SELL)
    {
        mStart = NormalizeDouble(getLow(shift) - (getAverageCandleSize() / 3) * Point, Digits-1);
    }    
    else
    {
        mStart = 0;
    }
}

int TradeBase::getTotalOrders(void)
{
    int i;
    int total_orders = 0;

    for (i = 0; i < OrdersTotal(); i++) 
    {
        if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false) return (-1);

        if (OrderSymbol() == Symbol() && OrderMagicNumber() == TRADEBASE_MAGIC) 
        {
            if (OrderComment() == getName()) 
            {
                total_orders++;
            }
        }
    }

    return (total_orders);
}

void TradeBase::setLots(double loss)
{
    double tick_value = (double) MarketInfo(Symbol(), MODE_TICKVALUE);
    int lotsize = (int) MarketInfo(Symbol(), MODE_LOTSIZE);

    mLots = NormalizeDouble((AccountBalance() * getRisk()) / (loss * tick_value * lotsize * Point), 2);
}