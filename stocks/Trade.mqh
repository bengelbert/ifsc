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
   double mMarginUsage;
   double mMarginLevel;
   double mMilestoneGrowth;

public:
          Trade();
          ~Trade();
   void   setAccountBalance(double balance);
   void   setMarginUsage(double marginusage);
   void   setMilestoneGrowth(double milestonegrowth);
   void   setMinMarginLevel(double marginlevel);
   double getAccountBalance(void);
   double getMilestoneGrowth(void);
   double getMarginUsage(void);
   double getStopGrowth(void);
   double getMinMarginLevel(void);
   
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
   if (balance <= 0)
   {
      mAccountBalance = 100;
   }
   else
   {
      mAccountBalance = balance;
   }
}

/********************************************************************/

void Trade::setMarginUsage(double marginusage)
{
   double balance = getAccountBalance();

   if (marginusage <= 0) marginusage = 0.003;
   
   if (balance < 20000)
      mMarginUsage = marginusage;
   else if (balance >= 20000 && balance < 50000)
      mMarginUsage = marginusage * 0.7667; //0.0023;
   else if (balance >= 50000 && balance < 100000)
      mMarginUsage = marginusage * 0.5000; //0.0015;
   else if (balance >= 100000 && balance < 500000)
      mMarginUsage = marginusage * 0.3667; //0.0011;   
   else if (balance >= 500000 && balance < 1000000)
      mMarginUsage = marginusage * 0.2667; //0.0008;   
   else if (balance >= 1000000 && balance < 2000000)
      mMarginUsage = marginusage * 0.1333; //0.0004;   
   else if (balance >= 2000000 && balance < 4000000)
      mMarginUsage = marginusage * 0.0667; //0.0002;   
   else if (balance >= 4000000)
      mMarginUsage = marginusage * 0.0333; //0.0001;   
   else
      mMarginUsage = marginusage;
}  

/********************************************************************/

double Trade::getMarginUsage(void)
{
   return (mMarginUsage);
}

/********************************************************************/

void Trade::setMilestoneGrowth(double milestonegrowth)
{
   double balance = getAccountBalance();

   if (milestonegrowth <= 0) milestonegrowth = 0.006;
   
   if (balance < 20000)
      mMilestoneGrowth = milestonegrowth;
   else if (balance >= 20000 && balance < 50000)
      mMilestoneGrowth = milestonegrowth * 0.7667; //0.0023;
   else if (balance >= 50000 && balance < 100000)
      mMilestoneGrowth = milestonegrowth * 0.5000; //0.0015;
   else if (balance >= 100000 && balance < 500000)
      mMilestoneGrowth = milestonegrowth * 0.3667; //0.0011;   
   else if (balance >= 500000 && balance < 1000000)
      mMilestoneGrowth = milestonegrowth * 0.2667; //0.0008;   
   else if (balance >= 1000000 && balance < 2000000)
      mMilestoneGrowth = milestonegrowth * 0.1333; //0.0004;   
   else if (balance >= 2000000 && balance < 4000000)
      mMilestoneGrowth = milestonegrowth * 0.0667; //0.0002;   
   else if (balance >= 4000000)
      mMilestoneGrowth = milestonegrowth * 0.0333; //0.0001;   
   else
      mMilestoneGrowth = milestonegrowth;
}  

/********************************************************************/

double Trade::getMilestoneGrowth(void)
{
   return (mMilestoneGrowth);
}

/********************************************************************/

double Trade::getStopGrowth(void)
{
   return (getMilestoneGrowth() * 2.5);
}

/********************************************************************/

double Trade::getMinMarginLevel(void)
{
   return (mMarginLevel);
}

/********************************************************************/

void Trade::setMinMarginLevel(double marginlevel)
{
   if (marginlevel < 0.5) marginlevel = 0.5;
   
   mMarginLevel = marginlevel;
}

/********************************************************************/

