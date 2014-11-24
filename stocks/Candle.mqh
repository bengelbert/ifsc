//+------------------------------------------------------------------+
//|                                                       Candle.mqh |
//|                        Copyright 2014, MetaQuotes Software Corp. |
//|                                              http://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2014, MetaQuotes Software Corp."
#property link      "http://www.mql5.com"
#property version   "1.00"
#property strict
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
class Candle
{
private:
    int mTimeFrame;
    int mTotalCandles;
    
public:
    Candle();
    ~Candle();

    double getVariance(int period, int shift=0);
    double getEMA(int period, int shift=0)  { return(NormalizeDouble(iMA(Symbol(), getTimeFrame(), period, 0, MODE_EMA, PRICE_CLOSE, shift), Digits)); };
    double getSMA(int period, int shift=0)  { return(NormalizeDouble(iMA(Symbol(), getTimeFrame(), period, 0, MODE_SMA, PRICE_CLOSE, shift), Digits)); };
    int    getAverageCandleSize();    
    double getClose(int shift=0) { return(NormalizeDouble(iClose(Symbol(), getTimeFrame(), shift), Digits)); };
    double getLow(int shift=0) { return(NormalizeDouble(iLow(Symbol(), getTimeFrame(), shift), Digits)); };
    double getLowest(int shift=0) { return(NormalizeDouble(iLow(Symbol(), getTimeFrame(), iLowest(Symbol(), getTimeFrame(), MODE_LOW, shift, 0)), Digits)); };
    double getHigh(int shift=0) { return(NormalizeDouble(iHigh(Symbol(), getTimeFrame(), shift), Digits)); };
    double getHighest(int shift=0) { return(NormalizeDouble(iHigh(Symbol(), getTimeFrame(), iHighest(Symbol(), getTimeFrame(), MODE_HIGH, shift, 0)), Digits)); };
    int    getTimeFrame() { return (mTimeFrame); };
    int    getTotalCandles() { return (mTotalCandles);};
    void   setTimeFrame(int timeframe) { mTimeFrame = timeframe; };
    void   setTotalCandles(int total) { mTotalCandles = total; };
};
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
Candle::Candle()
{
    mTimeFrame = PERIOD_H1;
    mTotalCandles = 500;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
Candle::~Candle()
{
}
//+------------------------------------------------------------------+

int Candle::getAverageCandleSize(void)
{
    int i;
    double size = 0;
    
    for (i = 0; i < getTotalCandles(); i++)
    {
        size += (getHigh(i) - getLow(i)) / Point;
    }
    
    size /= getTotalCandles();
    
    return ((int) size);
} 

//+------------------------------------------------------------------+

double Candle::getVariance(int period, int shift=0)
{
    double ma = 0;
    double variance = 0;

    ma = getSMA(period, shift);

    for (int i = 0; i < period; i++)
    {
        variance += MathPow((getClose(i+shift)/Point - ma/Point), 2);
    }

    variance /= period;
    variance *= Point;

    return (variance);
}