//+------------------------------------------------------------------+
//|                                                        Trade.mqh |
//|                        Copyright 2015, MetaQuotes Software Corp. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2015, MetaQuotes Software Corp."
#property link      "https://www.mql5.com"
#property version   "1.00"
#property strict
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
class Trade
  {
private:
   double mAccountBalance;

public:
          Trade();
          ~Trade();
   void   setAccountBalance(double balance);
   double getAccountBalance(void);
   double getMilestoneGrowth(void);
   double getMarginUsage(void);
   
};
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
Trade::Trade()
{
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
Trade::~Trade()
{
}

/********************************************************************/

double Trade::getAccountBalance(void)
{
   return (mAccountBalance);
}

/********************************************************************/

void Trade::setAccountBalance(double balance)
{
   if (balance == 0)
   {
      mAccountBalance = 1;
   }
   else
   {
      mAccountBalance = balance;
   }
}

/********************************************************************/

double Trade::getMarginUsage(void)
{
   double balance = getAccountBalance();
   double margin_usage = 0.003;
   
   if (balance < 20000)
   {
      margin_usage = 0.003;
   }
   else if (balance >= 20000 && balance < 50000)
   {
      margin_usage = 0.0023;
   }
   else if (balance >= 50000 && balance < 100000)
   {
      margin_usage = 0.0015;
   }
   else if (balance >= 100000 && balance < 500000)
   {
      margin_usage = 0.0011;   
   }
   else if (balance >= 500000 && balance < 1000000)
   {
      margin_usage = 0.0008;   
   }
   else if (balance >= 1000000 && balance < 2000000)
   {
      margin_usage = 0.0004;   
   }
   else if (balance >= 2000000 && balance < 4000000)
   {
      margin_usage = 0.0002;   
   }
   else if (balance >= 4000000)
   {
      margin_usage = 0.0001;   
   }
   else
   {
      margin_usage = 0.003;
   }
   
   return (margin_usage);
}

/********************************************************************/

double Trade::getMilestoneGrowth(void)
{
   return (getMarginUsage() * 2);
}
