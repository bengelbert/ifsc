/*#define AccountBalance()  ()
#define OrderOpenTime()  ()
#define iBarShift()  ()
#define ObjectGet()  ()
#define AccountMargin()  ()
#define AccountLeverage()  ()
#define StringFind()  ()
#define Sleep()  ()
#define Print()  ()
#define ObjectCreate()  ()
#define OrderCloseBy()  ()
#define ObjectSet()     ()
#define MarketInfo()    ()
#define NormalizeDouble()    ()
#define ObjectSetText()    ()
#define ObjectDelete()    ()
#define OrdersTotal()    ()
#define DoubleToStr()    ()
#define OrderSelect()    ()
#define OrderSend()    ()
#define OrderModify()    ()
#define OrderMagicNumber()    ()
#define StrToTime()    ()
#define TimeCurrent()    ()
#define OrderMagicNumber()    ()
#define iMA()    ()
#define iMACD()    ()
#define MathPow()    ()
#define OrderSymbol()    ()
#define OrderTicket()    ()
#define OrderTakeProfit()    ()
#define OrderSwap()    ()
#define OrderStopLoss()    ()
#define RefreshRates()    ()
#define OrderClose()    ()
#define Symbol()    ()
#define OrderType()    ()
#define OrderOpenPrice()    ()
#define OrderLots()    ()
#define GetLastError()    ()
#define iOpen()    ()
#define iClose()    ()
#define iLowest()    ()
#define iHighest()    ()
#define IsTradeAllowed()    ()
#define iAC()    ()
#define iHigh()    ()
#define iLow()    ()
#define string char*
#define bool char
#define datetime int
#define true 1
#define True 1
#define false 0
#define False 0
#define MODE_SMA    0
#define PRICE_CLOSE    0
#define MODE_MAIN 0
#define PRICE_MEDIAN    0
#define MODE_SPREAD    0
#define MODE_LOW 0
#define MODE_HIGH 0
#define Point    0
#define OP_BUY  0
#define OP_SELL  0
#define color    int
#define Red    0
#define Green    1
#define Bid    1
#define Ask    0
#define PERIOD_M15    0
#define PERIOD_M30    0
#define PERIOD_H1    0
#define PERIOD_H4    0
#define PERIOD_D1    0
#define PERIOD_W1    0
#define PERIOD_MN1    0
#define Time    0
*/
/******************************************************************************/

#include <stderror.mqh>
#include <stdlib.mqh>

//---- input parameters
extern int x1 = 135;
extern int x2 = 127;
extern int x3 = 16;
extern int x4 = 93;
// StopLoss level
extern double sl = 85;
extern double lots = 1;
extern int MagicNumber = 888;

static int gOrderbar = -1;
static int gOrderTicket = -1;

    //+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+

int init()
{
#define WT_INFO_NAME    "info"
    ObjectCreate(WT_INFO_NAME, OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet(WT_INFO_NAME, OBJPROP_CORNER, 0); // Reference corner
    ObjectSet(WT_INFO_NAME, OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet(WT_INFO_NAME, OBJPROP_YDISTANCE, 15); // Y coordinate
    //----
    return (0);
}
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+

int deinit()
{
    //----
    return (0);
}
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+

int start()
{
    int err = 0;
    int total = 0;
    int prevticket = 0;
    int modespread = MarketInfo(Symbol(), MODE_SPREAD);
    
    //----
    if (IsTradeAllowed()) {
        RefreshRates();
        modespread = MarketInfo(Symbol(), MODE_SPREAD);
    } else {
        return (0);
    }
    
    ObjectSetText(WT_INFO_NAME,
        "Perceptron("+gOrderbar+"): " +
        "Spread("+modespread+") "+
        ""+DoubleToStr(perceptron(), 2)+"",
        7, "Arial", White);

    // check for opened position
    total = OrdersTotal();
    
    //----
    for (int i = 0; i < total; i++) {
        OrderSelect(i, SELECT_BY_POS, MODE_TRADES);
        // check for symbol & magic number
        if (OrderSymbol() == Symbol() && OrderMagicNumber() == MagicNumber) {
            prevticket = OrderTicket();
            
            gOrderbar = iBarShift(Symbol(), 0, OrderOpenTime());
            
            // long position is opened
            if (OrderType() == OP_BUY) {
                // check profit 
                if (Bid > (OrderStopLoss() + (sl * 2 + modespread) * Point)) {
                    if (perceptron() < 0) { // reverse
                        gOrderTicket = OrderSend(Symbol(), OP_SELL, OrderLots(), Bid, 3,
                            Ask + sl * Point, 0, "AI Rev", MagicNumber, 0, Red);
                        Sleep(10000);
                        //----
                        if (gOrderTicket >= 0) {
                            OrderCloseBy(gOrderTicket, prevticket, Blue);
                        } else {
                            err=GetLastError();
                            Print("error[1](",err,"): ",ErrorDescription(err));
                            
                            if (OrderStopLoss() < OrderOpenPrice()) {
                                if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), 0, 0, Blue)) {
                                    Sleep(1000);
                                }
                            }
                            
                        }
                    } else { // trailing stop
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), Bid - sl * Point, 0, 0, Blue)) {
                            Sleep(1000);
                        }
                    }
                }
                // short position is opened
            } else {
                // check profit 
                if (Ask < (OrderStopLoss() - (sl * 2 + modespread) * Point)) {
                    if (perceptron() > 0) { // reverse
                        gOrderTicket = OrderSend(Symbol(), OP_BUY, OrderLots(), Ask, 3,
                            Bid - sl * Point, 0, "AI Rev", MagicNumber, 0, Blue);
                        Sleep(10000);
                        //----
                        if (gOrderTicket >= 0) {
                            OrderCloseBy(gOrderTicket, prevticket, Blue);
                        } else {
                            err=GetLastError();
                            Print("error[2](",err,"): ",ErrorDescription(err));
                            
                            if (OrderStopLoss() > OrderOpenPrice()) {
                                if (!OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice(), 0, 0, Blue)) {
                                    Sleep(1000);
                                }
                            }
                        }
                    } else { // trailing stop
                        if (!OrderModify(OrderTicket(), OrderOpenPrice(), Ask + sl * Point,
                            0, 0, Blue)) {
                            Sleep(1000);
                        }
                    }
                }
            }
            // exit
            return (0);
        }
    }

    if (gOrderbar == iBarShift(Symbol(), 0, 0)) {
        return (0);
    }

    lots = NormalizeDouble(AccountBalance() * aiGetRisk(Period()) / (sl + modespread), 2);
    if (lots < 0.01) lots = 0.01;
    
    // check for long or short position possibility
    if (perceptron() > 0) { //long
        gOrderTicket = OrderSend(Symbol(), OP_BUY, lots, Ask, 3, Bid - sl * Point, 0, "AI",
            MagicNumber, 0, Blue);
        //----
        if (gOrderTicket < 0) {
            Sleep(10000);
            err=GetLastError();
            Print("error[3](",err,"): ",ErrorDescription(err));
        }
    } else { // short
        gOrderTicket = OrderSend(Symbol(), OP_SELL, lots, Bid, 3, Ask + sl * Point, 0, "AI",
            MagicNumber, 0, Red);
        if (gOrderTicket < 0) {
            Sleep(10000);
            err=GetLastError();
            Print("error[4](",err,"): ",ErrorDescription(err));
        }
    }
    //--- exit
    return (0);
}

/******************************************************************************/

double aiGetRisk(int timeframe)
{
    double risk = 0.005;

    switch (timeframe) {
    case PERIOD_M15: risk = 0.005;
        break;
    case PERIOD_M30: risk = 0.005;
        break;
    case PERIOD_H1: risk = 0.01;
        break;
    case PERIOD_H4: risk = 0.02;
        break;
    case PERIOD_D1: risk = 0.04;
        break;
    case PERIOD_W1: risk = 0.08;
        break;
    case PERIOD_MN1: risk = 0.16;
        break;
    }

    return (risk);
}


double perceptron()
{
    double w1 = x1 - 100;
    double w2 = x2 - 100;
    double w3 = x3 - 100;
    double w4 = x4 - 100;
    double a1 = iAC(Symbol(), 0, 0);
    double a2 = iAC(Symbol(), 0, 7);
    double a3 = iAC(Symbol(), 0, 14);
    double a4 = iAC(Symbol(), 0, 21);
    return (w1 * a1 + w2 * a2 + w3 * a3 + w4 * a4);
}
//+------------------------------------------------------------------+
