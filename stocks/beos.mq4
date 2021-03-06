//|$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//  Multi Levels Trend Expert Advisor
//  hodhabi@gmail.com
//|$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

#define     NL    "\n" 

extern double LotsRate = 3; // = (0.01 * LotsRate) per symbol();
extern bool allowSL = false;
extern bool maReversal = true;
extern double SL = 600;
extern double MAP = 500;
extern double MaxTrade = 4;
extern bool   CloseAllNow      = false;          // closes all orders now
extern bool   CloseProfitableTradesOnly = false; // closes only profitable trades
extern double ProftableTradeAmount      = 1;     // Only trades above this amount close out
extern bool   ClosePendingOnly = false;          // closes pending orders only
extern bool   UseAlerts        = false;

class MLTrendE
{
public:
    string   mName;
    int      mPeriod;
    double   mRisk;
    int      mOrdersTotal;
    int      mTradeType;
    int      mTP;
    double   mLots;
    double   mBuyLots; 
    double   mSellLots;
    double   mBuyProfit;
    double   mSellProfit;  
    int      mOrdersBUY;  
    int      mOrdersSELL;
    int      mCloseToTrend;
    int      mPullBack;
    string   mEnableTrade;
    datetime mReversionCloseTime;
    int      mBarOpen;
    
    double  getFirstOpenPrice();   
    void    onCloseAllBuy(void);
    void    onCloseAllSell(void);
    void    onInit(string name, int period, double risk);
    int     onTick(void);
    void    printLine(string name, datetime date, double price, int colour, string comment);
};

static MLTrendE mlt[5];

//+-------------+
//| Custom init |
//|-------------+
int init()
{
    mlt[0].onInit("H1", PERIOD_H1, 0.005);
    mlt[1].onInit("H4", PERIOD_H4, 0.001);
    mlt[2].onInit("D1", PERIOD_D1, 0.002);
    mlt[3].onInit("W1", PERIOD_W1, 0.004);
    mlt[4].onInit("MN", PERIOD_MN1, 0.008);
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
            string result[5];
            
            StringSplit(OrderComment(), '.', result);
            
            if (result[0] == mName) {
                if ((OrderType() == OP_BUY && result[3] != "rev"))  
                    OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 5, Yellow);

                if ((OrderType()== OP_BUYSTOP && result[3] != "rev"))  
                    OrderDelete(OrderTicket(), White);

                if ((OrderType() == OP_SELL && result[3] == "rev"))  
                    OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Yellow);

                if ((OrderType() == OP_SELLSTOP && result[3] == "rev"))  
                    OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Yellow);
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
            string result[5];
            
            StringSplit(OrderComment(), '.', result);
            
            if (result[0] == mName) 
            {
                if ((OrderType() == OP_SELL && result[3] != "rev"))  
                    OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Yellow);

                if ((OrderType()== OP_SELLSTOP  && result[3] != "rev"))  
                    OrderDelete(OrderTicket(), White);

                if ((OrderType() == OP_BUY && result[3] == "rev"))  
                    OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 5, Yellow);

                if ((OrderType()== OP_BUYSTOP && result[3] == "rev"))  
                    OrderDelete(OrderTicket(), White);
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
    mlt[0].onTick();
    mlt[1].onTick();
    mlt[2].onTick();
    mlt[3].onTick();
    mlt[4].onTick();
}


int MLTrendE::onTick(void)
{
    int ticket;
    int i;
    int candle, total_orders, total_orders_rev;
    int spread = (int) MarketInfo(Symbol(), MODE_SPREAD);
    string trade_comment = "";
    string trend_name = "";
    double tick_value = (double) MarketInfo(Symbol(), MODE_TICKVALUE);
    int lotsize = (int) MarketInfo(Symbol(), MODE_LOTSIZE);
    double  lots = 0;
    int     pullback;
    bool    reversion_enable = false;

    //+------------------------------------------------------------------+
    //  Determine last order price                                       |
    //-------------------------------------------------------------------+
    double Trend   = iMA(Symbol(), mPeriod, MAP, 0, MODE_EMA, PRICE_CLOSE, 1);
    double Trend21 = iMA(Symbol(), mPeriod,  21, 0, MODE_EMA, PRICE_CLOSE, 0);
    double Trend34 = iMA(Symbol(), mPeriod,  34, 0, MODE_EMA, PRICE_CLOSE, 0);

    candle = 0;
    for (i = 0; i < MAP; i++) 
    {
        candle += (iHigh(Symbol(), mPeriod, i) - iLow(Symbol(), mPeriod, i)) / Point;        
    }
    
    mTP = candle / MAP;
        
    mCloseToTrend = MathAbs(iClose(Symbol(), mPeriod, 0) - iMA(Symbol(), mPeriod, MAP, 0, MODE_EMA, PRICE_CLOSE, 0)) / Point;

    //mPullBack = mCloseToTrend/4;

    //if (mPullBack < mTP) 
    //{
        mPullBack = mTP;
    //}

    //mLots = NormalizeDouble((spread * 0.00001) / MarketInfo(Symbol(), MODE_LOTSTEP) , 2);
    //mLots = NormalizeDouble(LotsRate / (mTP * tick_value * lotsize * Point), 2);
    int total_pullback = mPullBack * 5 + mPullBack * 4 + mPullBack * 3 * 2 + mPullBack * 2 * 4 + mPullBack * 8;
    
    mLots = NormalizeDouble((AccountBalance() * mRisk) / (total_pullback * tick_value * lotsize * Point), 2);
    
    //mLots = NormalizeDouble(LotsRate / mTP, 2);
    //mLots = 
    //mLots = 0.03;
    //Print("s:", spread ,"|", DoubleToStr(mLots, 2));
        
    int tp_max = NormalizeDouble(LotsRate * 100 * tick_value, 0);
    
    if (NormalizeDouble(mTP/spread, 0) < 3.0 || MathAbs((Trend - Trend34)/Point) < mTP ||/*mTP > tp_max ||*/ mLots == 0) 
    {
        mEnableTrade = "DISABLE";
    }
    else
    {
        mEnableTrade = "ENABLE";
    }
    
    total_orders = 0;
    total_orders_rev = 0;
    for (i = 0; i < OrdersTotal(); i++) 
    {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES);

        if (OrderSymbol() == Symbol()) 
        {
            string result[5];
            
            StringSplit(OrderComment(), '.', result);
            
            if (result[0] == mName) {
                if (result[3] == "rev")
                    total_orders_rev++;
                else
                    total_orders++;
            }
        }
    }
    mOrdersTotal = total_orders;

    if (iClose(Symbol(), mPeriod, 0) < Trend &&
        iClose(Symbol(), mPeriod, 1) < Trend &&
        Trend34 < Trend && 
        Trend21 < Trend34)
    {
        trend_name = "DW";
    }
    else if (iClose(Symbol(), mPeriod, 0) > Trend && 
             iClose(Symbol(), mPeriod, 1) > Trend &&
             Trend34 > Trend && 
             Trend21 > Trend34)
    {
        trend_name = "UP";
    }
    else
    {
        trend_name = "--";
        
        if (iClose(Symbol(), mPeriod, 1) < Trend)
            mTradeType = 2;
        else
            mTradeType = 1;
    }

    //close on SL        
    if (trend_name == "DW" && mOrdersTotal > 0 && maReversal == true)
    { 
        onCloseAllBuy();
        mTradeType = 2;
    }

    if(trend_name == "UP" && mOrdersTotal > 0  && maReversal == true)
    {
        onCloseAllSell();
        mTradeType = 1;
    }

    if (mTradeType == 1 && 
        iClose(Symbol(), mPeriod, 0) < Trend21 && 
        Trend21 < Trend34 &&
        mOrdersTotal > 0)
        //MathAbs((Trend - Trend34)/Point) > mTP)
        reversion_enable = true;
    else if (mTradeType == 2 && 
             Trend21 > Trend34 &&
             iClose(Symbol(), mPeriod, 0) > Trend21 && 
             mOrdersTotal > 0) 
             //MathAbs((Trend - Trend34)/Point) > mTP)
        reversion_enable = true;
    else
        reversion_enable = false;


    //trade_comment = mName+"."+mOrdersTotal+"."+DoubleToStr(AccountBalance(),0);
    trade_comment = mName+".0."+mPullBack;
    if(trend_name == "UP" &&
       mOrdersTotal == 0 && 
       mTradeType == 0 && 
       mEnableTrade == "ENABLE") 
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

    if(trend_name == "DW" &&
       mOrdersTotal == 0 && 
       mTradeType == 0 && 
       mEnableTrade == "ENABLE")
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


      if(mOrdersTotal == 0 && 
         trend_name == "UP" &&
         mTradeType == 1 && 
         mEnableTrade == "ENABLE")
        {
         ticket=OrderSend(Symbol(),OP_BUY,mLots,Ask,3,0,0,trade_comment,16384,0,Green);

         if(ticket>0)
           {
            if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) sendEmail();
           }
         else Print("Error opening BUY order : ",GetLastError()); 

         return(0); 

        }



      if(mOrdersTotal == 0 && 
         trend_name == "DW" &&
         mTradeType == 2 && 
         mEnableTrade == "ENABLE")
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

            // Reversao
            if (result[0] == mName && 
                StringToInteger(result[1]) == (mOrdersTotal-1) && 
                total_orders_rev == 0 && 
                mOrdersTotal >= 1 && 
                reversion_enable == true)
            {
                if (result[1] == "0")
                    lots = OrderLots();
                else
                    lots = NormalizeDouble(OrderLots() * 2, 2);
                    
                pullback = result[2];
                
                if (pullback == 0 || lots == 0) return (0);
                
                trade_comment = OrderComment()+".rev";
                if (OrderType() == OP_BUY && iClose(Symbol(), mPeriod, 0) < OrderOpenPrice()) 
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

                if (OrderType() == OP_SELL && iClose(Symbol(), mPeriod, 0) > OrderOpenPrice())
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
            }
            
            if (result[0] == mName && 
                StringToInteger(result[1]) == (mOrdersTotal-1))
            {
                mBarOpen = iBarShift(Symbol(), mPeriod, OrderOpenTime());
            }
            
            if (result[0] == mName && 
                StringToInteger(result[1]) == (mOrdersTotal-1) && 
                mOrdersTotal <= MaxTrade  &&
                result[3] != "rev" && 
                reversion_enable == false &&
                mBarOpen > 0) 
            {
                if (result[1] == "0")
                    lots = OrderLots() * 1;
                else if(result[1] == "1")
                    lots = OrderLots() * 2;
                else if(result[1] == "2")
                    lots = OrderLots() * 2;
                else if(result[1] == "3")
                    lots = OrderLots() * 2;
                else
                    return (0);
                    
                lots = NormalizeDouble(lots, 2);
                pullback = result[2];
                
                if (pullback == 0 || lots == 0) return (0);

                trade_comment = mName+"."+mOrdersTotal+"."+pullback;

                if ((OrderType() == OP_BUY) && ((OrderOpenPrice()-OrderClosePrice())/Point)>= pullback)
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
                
                if ((OrderType()==OP_SELL) && ((OrderClosePrice()- OrderOpenPrice())/Point)>= pullback)
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
                    double exit_price;
                    
                    pullback = result[2];
                    
                    if (pullback == 0) return (0);
                
                    exit_price = pullback;
                
                    if (StringToInteger(result[1]) == mOrdersTotal-1/* && mPeriod == Period()*/)
                    {
                        if (OrderType() == OP_BUY && result[3] != "rev")
                            printLine("target"+mName, OrderOpenTime(), OrderOpenPrice() + exit_price*Point, Blue, OrderComment());
                        else if (OrderType() == OP_SELL && result[3] != "rev")
                            printLine("target"+mName, OrderOpenTime(), OrderOpenPrice() - exit_price*Point, Blue, OrderComment());
                    }
                    
                    if ((OrderType()==OP_BUY) && ((OrderClosePrice()-OrderOpenPrice())/Point)>= exit_price && result[3] != "rev")
                    {
                        OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 5, Green);
                        return(0); 
                    } 

                    if ((OrderType()==OP_SELL) && ((OrderOpenPrice()-OrderClosePrice())/Point)>= exit_price && result[3] != "rev")
                    {
                        OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Green);
                        return(0); 
                    } 
                    
                    if ((OrderType()==OP_BUY) && OrderOpenPrice() < Trend34 && result[3] == "rev" && OrderOpenPrice() != OrderStopLoss())
                    {
                        OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), OrderTakeProfit(), 0, Pink);
                        return(0); 
                    } 

                    if ((OrderType()==OP_SELL) && OrderOpenPrice() > Trend34 && result[3] == "rev" && OrderOpenPrice() != OrderStopLoss())
                    {
                        OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), OrderTakeProfit(), 0, Pink);
                        return(0); 
                    } 
                    
                    
                    if ((OrderType() == OP_BUY) && ((OrderOpenPrice() - OrderClosePrice())/Point) >= exit_price && result[3] == "rev" && iClose(Symbol(), mPeriod, 0) < Trend34)
                    {
                        if (OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 5, Red))
                        {
                            return(0); 
                        }
                    } 

                    if ((OrderType() == OP_SELL) && ((OrderClosePrice() - OrderOpenPrice())/Point) >= exit_price && result[3] == "rev" && iClose(Symbol(), mPeriod, 0) > Trend34)
                    {
                        if (OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Red))
                        {
                            return(0); 
                        }
                    } 

/*
                    if (OrderType() == OP_BUY && 
                        reversion_enable == false &&
                        OrderClosePrice() > OrderOpenPrice() && 
                        result[3] == "rev")
                    {
                        if (OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_BID), 5, Orange))
                        {
                            return(0); 
                        }
                    } 

                    if (OrderType() == OP_SELL && 
                        reversion_enable == false &&
                        OrderClosePrice() < OrderOpenPrice() && 
                        result[3] == "rev")
                    {
                        if (OrderClose(OrderTicket(), OrderLots(), MarketInfo(OrderSymbol(), MODE_ASK), 5, Orange))
                        {
                            return(0); 
                        }
                    } 
*/
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
        //printLine("tp0", Time[1], Trend + mTP*Point, Red, "");
        //printLine("tp1", Time[1], Trend - mTP*Point, Red, "");
    
        Comment("Developed by: Bruno Engelbert", NL,             
                "mName: ", mName,"|", trend_name, "|", mEnableTrade, NL,
                "Trend|reversion|shiftClose: ",DoubleToStr(Trend, 5), "|",reversion_enable,"|",iBarShift(Symbol(), mPeriod, mReversionCloseTime), NL,
                "mLots|Size|Step|Tick: ", DoubleToStr(mLots, 2),"|", lotsize, "|", MarketInfo(Symbol(), MODE_LOTSTEP),"|",tick_value, NL,          
                "mOrdersTotal(rev)|mBarOpen: ", mOrdersTotal,"(",total_orders_rev,")|",mBarOpen, NL, 
                "mTradeType: ", mTradeType, NL, 
                "mTP(delta|max)|Spread|Rate: ", mTP,"(",DoubleToStr(MathAbs((Trend - Trend34)/Point), 0),"|",tp_max,")","|",spread,"|",DoubleToStr(mTP/spread, 1), NL,
                "mCloseToTrend :", mCloseToTrend,"|",mPullBack, NL
                "total_pullback: ", total_pullback, "(",NormalizeDouble(total_pullback * tick_value * lotsize * Point,0),")", NL
                );
    }
} // start()


/****************************************************************/

double MLTrendE::getFirstOpenPrice()
{
    int i;
    double open_price = 0;;
    
    for(i = 0; i < OrdersTotal(); i++)
    {
        if(OrderSelect(i, SELECT_BY_POS) == true)
        {
            if (OrderSymbol() == Symbol()) 
            {
                string result[5];
                
                StringSplit(OrderComment(), '.', result);
            
                if (result[0] == mName && result[1] == "0")
                {
                    open_price = OrderOpenPrice();
                }
            }
        }
    }    
    
    return open_price;
}

/****************************************************************/

void MLTrendE::onInit(string name, int period, double risk)
{
    mName = name;
    mPeriod = period;
    mRisk = risk;
    mReversionCloseTime = 0;
}

void MLTrendE::printLine(string name, datetime date, double price, int colour, string comment)
{
    ObjectDelete(name);

    if (price > 0) {
        //ObjectCreate(name, OBJ_TREND, 0, date, price, iTime(Symbol(), 0, 0), price);
        ObjectCreate(name, OBJ_HLINE, 0, date, price, iTime(Symbol(), 0, 0), price);
        ObjectSet(name, OBJPROP_COLOR, colour); 
        ObjectSet(name, OBJPROP_STYLE, 2);
        ObjectSetText(name, comment, 7, "Arial", White);
    }    
}







