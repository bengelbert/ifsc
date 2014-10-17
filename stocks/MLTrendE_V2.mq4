//|$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//  Multi Levels Trend Expert Advisor
//  hodhabi@gmail.com
//|$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

#define     NL    "\n" 

extern double L1 = 1;
extern double L2 = 2;
extern double L3 = 4;
extern double L4 = 8;
extern bool allowSL = false;
extern bool maReversal = true;
extern double SL = 600;
extern double MAP = 34;
extern double MaxTrade = 4;
extern bool   CloseAllNow      = false;          // closes all orders now
extern bool   CloseProfitableTradesOnly = false; // closes only profitable trades
extern double ProftableTradeAmount      = 1;     // Only trades above this amount close out
extern bool   ClosePendingOnly = false;          // closes pending orders only
extern bool   UseAlerts        = false;

class MLTrendE
{
public:
    string  mName;
    int     mPeriod;
    double  mRisk;
    int     mOrdersTotal;
    int     mTradeType;
    int     mTP;
    double  mLots;
    double  mBuyLots; 
    double  mSellLots;
    double  mBuyProfit;
    double  mSellProfit;  
    int     mOrdersBUY;  
    int     mOrdersSELL;
    int     mCloseToTrend;
    
    void onCloseAllBuy(void);
    void onCloseAllSell(void);
    void onInit(string name, int period, double risk);
    int  onTick(void);
    void printLine(string name, datetime date, double price, int colour, string comment);
};

static MLTrendE mlt[5];

//+-------------+
//| Custom init |
//|-------------+
int init()
{
//    mlt[0].onInit("M15", PERIOD_M15, 0.00025);
    mlt[0].onInit("H1", PERIOD_H1, 0.01);
    mlt[1].onInit("H4", PERIOD_H4, 0.02);
    mlt[2].onInit("D1", PERIOD_D1, 0.04);
    mlt[3].onInit("W1", PERIOD_W1, 0.08);
    mlt[4].onInit("MN1", PERIOD_MN1, 0.16);
}

//+----------------+
//| Custom DE-init |
//+----------------+

int deinit()
{

}



void sendEmail()
{
    if (UseAlerts==true) 
        SendMail("YTF Alert", "New order has been added  "+OrdersTotal()+"   Balance = " +AccountBalance() + " Equity = "+AccountEquity() +" Current Price: " + Close[0]);

    return;
}

void MLTrendE::onCloseAllBuy(void)
{
    int i;
    
    for (i = 0; i < OrdersTotal(); i++) 
    {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES);

        if (OrderSymbol() == Symbol()) 
        {
            string result[];
            
            StringSplit(OrderComment(), '.', result);
            
            if (result[0] == mName) {
                if (OrderType() == OP_BUY)  
                    OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 5, Yellow);

                if (OrderType()== OP_BUYSTOP)  
                    OrderDelete(OrderTicket(), White);

                if (UseAlerts) 
                    PlaySound("alert.wav");
            }
        }
    }

    return; 
}



void MLTrendE::onCloseAllSell(void)
{
    int i;
    
    for (i = 0; i < OrdersTotal(); i++) 
    {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES);

        if (OrderSymbol() == Symbol()) 
        {
            string result[];
            
            StringSplit(OrderComment(), '.', result);
            
            if (result[0] == mName) 
            {
                if (OrderType() == OP_SELL)  
                    OrderClose( OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Yellow );

                if (OrderType()== OP_SELLSTOP)  
                    OrderDelete( OrderTicket(), White);

                if (UseAlerts) 
                    PlaySound("alert.wav");
            }
        }
    }

    return; 
}

//+-----------+
//| Main      |
//+-----------+
int start()
{
    //ObjectsDeleteAll();

    mlt[0].onTick();
    mlt[1].onTick();
    mlt[2].onTick();
    mlt[3].onTick();
    mlt[4].onTick();
    //mlt[5].onTick();
}


int MLTrendE::onTick(void)
{
    int ticket;
    int i;
    int candle, total_orders;
    int spread = (int) MarketInfo(Symbol(), MODE_SPREAD);
    string trade_comment = "";
    string trend_name = "";

    //+------------------------------------------------------------------+
    //  Determine last order price                                       |
    //-------------------------------------------------------------------+
    double Trend = iMA(Symbol(), mPeriod, MAP, 0, MODE_EMA, PRICE_CLOSE, 1);
    double Trend9 = iMA(Symbol(), mPeriod, 9, 0, MODE_EMA, PRICE_CLOSE, 1);

    candle = 0;
    for (i = 0; i < 9; i++) 
    {
        candle += (iHigh(Symbol(), mPeriod, i) - iLow(Symbol(), mPeriod, i)) / Point;        
    }
    
    mTP = candle / 9;
    mTP += spread;
    
    mCloseToTrend = MathAbs(Close[0] - iMA(Symbol(), mPeriod, MAP, 0, MODE_EMA, PRICE_CLOSE, 0)) / Point;
    
    if (mCloseToTrend > 0)
        mLots = NormalizeDouble((AccountBalance() * mRisk) / (mCloseToTrend+mTP+spread), 2);
    else
        mLots = 0;

    total_orders = 0;
    for (i = 0; i < OrdersTotal(); i++) 
    {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES);

        if (OrderSymbol() == Symbol()) 
        {
            string result[];
            
            StringSplit(OrderComment(), '.', result);
            
            if (result[0] == mName) {
                total_orders++;
            }
        }
    }
    mOrdersTotal = total_orders;

    //close on SL
    if (Trend <= Trend9)
        trend_name = "UP";
    else
        trend_name = "DOWN";
        
    if (iClose(Symbol(), mPeriod, 1) < Trend && mOrdersTotal > 0 && maReversal == true && Trend > Trend9)
    { 
        onCloseAllBuy();
        mTradeType = 2;
    }

    if(iClose(Symbol(), mPeriod, 1) > Trend && mOrdersTotal > 0  && maReversal == true && Trend < Trend9)
    {
        onCloseAllSell();
        mTradeType = 1;
    }

    //trade_comment = mName+"."+mOrdersTotal+"."+DoubleToStr(AccountBalance(),0);
    trade_comment = mName+"."+mOrdersTotal;
    if(Close[0] > Trend && mOrdersTotal == 0 && mTradeType == 0 && mCloseToTrend > mTP && mLots > 0 && Trend <= Trend9)
    {
        ticket=OrderSend(Symbol(),OP_BUY,mLots,Ask,3,0,0,trade_comment,16384,0,Green);

        if(ticket>0)
        {
            if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) 
                sendEmail();
        }
        else 
            Print("Error opening BUY order : ",GetLastError()); 

        return(0); 
    }

    if(Close[0] < Trend && mOrdersTotal == 0 && mTradeType == 0 && mCloseToTrend > mTP && mLots > 0 && Trend >= Trend9)
    {
        ticket=OrderSend(Symbol(),OP_SELL,mLots,Bid,3,0,0,trade_comment,16384,0,Green);

        if(ticket>0)
        {
            if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) 
                sendEmail();
        }
        else 
            Print("Error opening BUY order : ",GetLastError()); 

        return(0); 
    }





      if(mOrdersTotal == 0 && mTradeType ==1  && mCloseToTrend > mTP && mLots > 0)

        {

         ticket=OrderSend(Symbol(),OP_BUY,mLots,Ask,3,0,0,trade_comment,16384,0,Green);

         if(ticket>0)

           {

            if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) sendEmail();

           }

         else Print("Error opening BUY order : ",GetLastError()); 

         return(0); 

        }



      if(mOrdersTotal == 0 && mTradeType == 2 && mCloseToTrend > mTP && mLots > 0)

        {

         ticket=OrderSend(Symbol(),OP_SELL,mLots,Bid,3,0,0,trade_comment,16384,0,Green);

         if(ticket>0)

           {

            if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) sendEmail();

           }

         else Print("Error opening BUY order : ",GetLastError()); 

         return(0); 

        }


    for (i = 0; i < OrdersTotal(); i++) 
    {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES);

        if (OrderSymbol() == Symbol()) 
        {
            StringSplit(OrderComment(), '.', result);
            
            if (result[0] == mName && StringToInteger(result[1]) == (mOrdersTotal-1) && mOrdersTotal <= MaxTrade) 
            {
                double lots = 0;
                    
                if (result[1] == "0")
                    lots = OrderLots() * L1;
                else if(result[1] == "1")
                    lots = OrderLots() * L2;
                else if(result[1] == "2")
                    lots = OrderLots() * L3;
                else if(result[1] == "3")
                    lots = OrderLots() * L4;
                else
                    return (0);
                    
                lots = NormalizeDouble(lots, 2);

                if ((OrderType()==OP_BUY) && ((OrderOpenPrice()-OrderClosePrice())/Point)>= mTP && lots > 0 && mTradeType == 1)
                {
                    ticket=OrderSend(Symbol(),OP_BUY,lots,Ask,3,0,0,trade_comment,16384,0,Green);

                    if(ticket>0)
                    {
                        if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) 
                            sendEmail();
                    }
                    else 
                        Print("Error opening BUY order : ",GetLastError()); 

                    return(0);
                }
                
                if ((OrderType()==OP_SELL) && ((OrderClosePrice()- OrderOpenPrice())/Point)>= mTP && lots > 0 && mTradeType == 2)
                {
                    ticket=OrderSend(Symbol(),OP_SELL,lots,Bid,3,0,0,trade_comment,16384,0,Green);

                    if(ticket>0)
                    {
                        if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) 
                            sendEmail();
                    }
                    else 
                        Print("Error opening BUY order : ",GetLastError()); 
                    
                    return(0); 
                }
            }
        }
    }    


//Close all



    for(i = 0; i < OrdersTotal(); i++)
    {
        if(OrderSelect(i, SELECT_BY_POS) == true)
        {
            if (OrderSymbol() == Symbol()) 
            {
                StringSplit(OrderComment(), '.', result);
            
                if (result[0] == mName)
                {
                    if ((OrderType()==OP_BUY) && ((OrderClosePrice()-OrderOpenPrice())/Point)>= mTP)
                    {
                        OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 5, Red);
                        return(0); 
                    } 

                    if ((OrderType()==OP_SELL) && ((OrderOpenPrice()-OrderClosePrice())/Point)>= mTP)
                    {
                        OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Red);
                        return(0); 
                    } 
                }
            }
        }
    }
    
    mOrdersBUY = 0;
    mOrdersSELL = 0;
    mBuyLots = 0;
    mSellLots = 0;
    mBuyProfit = 0;
    mSellProfit = 0;
    
    for(i = 0; i < OrdersTotal(); i++)
    {
        if(OrderSelect(i, SELECT_BY_POS) == true)
        {
            if (OrderSymbol() == Symbol()) 
            {
                StringSplit(OrderComment(), '.', result);
            
                if (result[0] == mName)
                {
                    if (StringToInteger(result[1]) == mOrdersTotal-1 && mPeriod == Period())
                    {
                        printLine("chan_up", OrderOpenTime(), OrderOpenPrice() + mTP*Point, Gold, OrderComment());
                        printLine("chan_down", OrderOpenTime(), OrderOpenPrice() - mTP*Point, Gold, OrderComment());
                    }
                
                    if(OrderType()==OP_BUY)  mOrdersBUY++;
                    if(OrderType()==OP_SELL) mOrdersSELL++;
                    if(OrderType()==OP_BUY)  mBuyLots += OrderLots();
                    if(OrderType()==OP_SELL) mSellLots += OrderLots();
                    if(OrderType() == OP_BUY)  mBuyProfit += OrderProfit() + OrderCommission() + OrderSwap();
                    if(OrderType() == OP_SELL)  mSellProfit += OrderProfit() + OrderCommission() + OrderSwap();
                }
            }
        }
    }   

    if (mPeriod == Period()) 
    {
        
        printLine("tp0", Time[1], Trend + mTP*Point, Red, "");
        printLine("tp1", Time[1], Trend - mTP*Point, Red, "");
    
        Comment("Developed by: Bruno Engelbert", NL,             
                "mName: ", mName,"|", trend_name, NL,
                "Trend9|21: ", DoubleToStr(Trend9, 5), "|",DoubleToStr(Trend, 5), NL,
                "mLots: ", mLots, NL,          
                "Lucro: ", DoubleToStr(mLots*L1*(mTP-spread), 2),
                     "|",  DoubleToStr(mLots*L2*(mTP-spread), 2),
                     "|",  DoubleToStr(mLots*L3*(mTP-spread), 2),
                     "|",  DoubleToStr(mLots*L4*(mTP-spread), 2), NL,
                "mOrdersTotal: ", mOrdersTotal, NL, 
                "mTradeType: ", mTradeType, NL, 
                "mTP           :", mTP, NL,
                "mCloseToTrend :", mCloseToTrend, NL
                );
    }
} // start()


/****************************************************************/

void MLTrendE::onInit(string name, int period, double risk)
{
    mName = name;
    mPeriod = period;
    mRisk = risk;
}

void MLTrendE::printLine(string name, datetime date, double price, int colour, string comment)
{
    ObjectDelete(name);

    if (price > 0) {
        ObjectCreate(name, OBJ_TREND, 0, date, price, iTime(Symbol(), 0, 0), price);
        ObjectSet(name, OBJPROP_COLOR, colour); 
        ObjectSet(name, OBJPROP_STYLE, 2);
        ObjectSetText(name, comment, 7, "Arial", White);
    }    
}








