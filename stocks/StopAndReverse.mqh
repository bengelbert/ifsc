//+------------------------------------------------------------------+
//|                                               StopAndReverse.mqh |
//|                        Copyright 2014, MetaQuotes Software Corp. |
//|                                              http://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2014, MetaQuotes Software Corp."
#property link      "http://www.mql5.com"
#property version   "1.00"
#property strict

#include "TradeBase.mqh"
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
class StopAndReverse : public TradeBase
{
private:
    
public:
    StopAndReverse();
    ~StopAndReverse();
    
    int findReverse();
    bool checkReverse(int reverse);
    double getSAR(int shift=0);
    void onCloseAllBuy();
    void onCloseAllSell();
    int onTick(void);
};
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
StopAndReverse::StopAndReverse()
{
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
StopAndReverse::~StopAndReverse()
{
}
//+------------------------------------------------------------------+

int StopAndReverse::onTick(void)
{
    int reverse;

    reverse = findReverse();

    if (getTimeFrame() == Period())
        Comment("lots: ", DoubleToStr(getLots(), 2),"\n",
                "AverageCandleSize: ", getAverageCandleSize(),"\n",
                "Spread: ", getSpread(),"\n",
                "div: ", DoubleToStr((double) getAverageCandleSize()/ (double)getSpread(), 2));

    return (0);
}

//+------------------------------------------------------------------+

int StopAndReverse::findReverse(void)
{
    int i;
    double sar_n0;
    double sar_n1;
    double close_n0;
    double close_n1;
    int ticket; 
    int loss;
    
    for (i = 0; i < 1; i++)
    {
        sar_n0 = getSAR(i);
        sar_n1 = getSAR(i+1);
        close_n0 = getClose(i);
        close_n1 = getClose(i+1);
        
        setTradeType(TRADEBASE_WAIT);

        loss = (int) (MathAbs(sar_n0 - close_n0) / Point) + getSpread();
            
        setLots(loss);
        
        if (close_n0 < sar_n0 && close_n1 > sar_n1)
        {
            onCloseAllBuy();
            
            if (getTotalOrders() == 0 && getLots() != 0 && NormalizeDouble((double)getAverageCandleSize()/(double)getSpread(), 0) > 3.0)
                ticket = OrderSend(Symbol(), OP_SELL, getLots(), Bid, 3, 0, 0, getName(), TRADEBASE_MAGIC, 0, Green);
                
            setTradeType(TRADEBASE_SELL);
            break;
        }
            
        if (close_n0 > sar_n0 && close_n1 < sar_n1)
        {
            onCloseAllSell();
            
            if (getTotalOrders() == 0 && getLots() != 0 && NormalizeDouble((double)getAverageCandleSize()/(double)getSpread(), 0) > 3.0)
                ticket = OrderSend(Symbol(), OP_BUY, getLots(), Ask, 3, 0, 0, getName(), TRADEBASE_MAGIC, 0, Green);
                
            setTradeType(TRADEBASE_BUY);
            break;
        }
    }
    
    return (i);
}

//+------------------------------------------------------------------+

bool StopAndReverse::checkReverse(int reverse)
{
    bool rc = false;
    double high = getHigh(reverse) + (getAverageCandleSize()/3) * Point;
    double highest = getHighest(reverse);
    double low = getLow(reverse) - (getAverageCandleSize()/3) * Point;
    double lowest = getLowest(reverse);
    
    if (highest < high && lowest > low)
        rc = true;
    
    return (rc);
}

//+------------------------------------------------------------------+

double StopAndReverse::getSAR(int shift)
{
    return (NormalizeDouble(iSAR(Symbol(), getTimeFrame(), 0.02, 0.2, shift), Digits));
}

void StopAndReverse::onCloseAllBuy(void)
{
    int i;
    
    for (i = 0; i < OrdersTotal(); i++) 
    {
        if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false) return;

        if (OrderSymbol() == Symbol() && OrderMagicNumber() == TRADEBASE_MAGIC) 
        {
            if (OrderComment() == getName()) 
            {
                if (OrderType() == OP_BUY)  
                    if (OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 5, Yellow) == false) return;

                if (OrderType()== OP_BUYSTOP)  
                    if (OrderDelete(OrderTicket(), White) == false) return;
            }
        }
    }

    return; 
}

void StopAndReverse::onCloseAllSell(void)
{
    int i;
    
    for (i = 0; i < OrdersTotal(); i++) 
    {
        if (OrderSelect(i, SELECT_BY_POS, MODE_TRADES) == false) return;

        if (OrderSymbol() == Symbol() && OrderMagicNumber() == TRADEBASE_MAGIC) 
        {
            if (OrderComment() == getName()) 
            {
                if (OrderType() == OP_SELL)  
                    if (OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Yellow) == false) return;

                if (OrderType()== OP_SELLSTOP)  
                    if (OrderDelete(OrderTicket(), White) == false) return;
            }
        }
    }

    return; 
}
