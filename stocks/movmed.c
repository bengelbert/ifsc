#define ObjectCreate()  ()
#define ObjectSet()     ()
#define MarketInfo()    ()
#define NormalizeDouble()    ()
#define ObjectSetText()    ()
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
#define iHigh()    ()
#define iLow()    ()
#define string char*
#define bool char
#define true 1
#define false 0
#define MODE_SMA    0
#define PRICE_MEDIAN    0
#define MODE_SPREAD    0
#define Point    0
#define OP_BUY  0
#define OP_SELL  0
#define color    int
#define Bid    1
#define Ask    0
#define PERIOD_D1    0
#define PERIOD_W1    0
/******************************************************************************/

#define MAX_PERIODS 2
#define PERIOD      PERIOD_D1

/******************************************************************************/

double aux = 0;
//---- input parameters
extern string StopSettings = "Set stops below";

extern bool AllowBuy = false;

extern bool AllowSell = false;

extern bool CloseOnReverseSignal = true;

extern string TimeSettings = "Set the time range the EA should trade";

extern string StartTime = "00:00";

extern string EndTime = "23:59";

extern string Ma1 = "First Ma settings";

extern int Ma1Period = 16;

extern int Ma1Shift = 8;

extern int Ma1Method = MODE_SMA;

extern int Ma1AppliedPrice = PRICE_MEDIAN;

extern string Ma2 = "Second Ma settings";

extern int Ma2Period = 1;

extern int Ma2Shift = 0;

extern int Ma2Method = MODE_SMA;

extern int Ma2AppliedPrice = PRICE_MEDIAN;

extern string MagicNumbers = "Set different magicnumber for each timeframe of a pair";

extern int MagicNumber = 103;


static int gblHaltBuy[MAX_PERIODS] = {0, 0};
static int gblHaltSell[MAX_PERIODS] = {0, 0};
static int gblSlip = 5;
static int gblSlippage[MAX_PERIODS] = {0, 0};
static int gblSpread;
static int gblStopMultd[MAX_PERIODS] = {0, 0};
static int gblTimeFrame[MAX_PERIODS] = {PERIOD_D1, PERIOD_W1};
static double gblLots[MAX_PERIODS] = {0, 0};
static double gblMedCandles[MAX_PERIODS] = {0, 0};
static double gblMedCandlesFactor[MAX_PERIODS] = {1.25, 1};
static double gblSL[MAX_PERIODS] = {0, 0};
static double gblTP[MAX_PERIODS] = {0, 0};

double perc = 0;
string freeze;

/******************************************************************************/

int deinit()
{
    return;
}

/******************************************************************************/

int init()
{
    ObjectCreate("label_0", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_0", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_0", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_0", OBJPROP_YDISTANCE, 15); // Y coordinate

    ObjectCreate("label_1", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_1", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_1", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_1", OBJPROP_YDISTANCE, 28); // Y coordinate
    
    ObjectCreate("label_buy", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_buy", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_buy", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_buy", OBJPROP_YDISTANCE, 41); // Y coordinate

    ObjectCreate("label_sell", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_sell", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_sell", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_sell", OBJPROP_YDISTANCE, 53); // Y coordinate

    return (0);
}

/******************************************************************************/

int start()
{
    int i, j;
    double high, low, lenCandle;
    double difference;
    int TradeTimeOk;
    bool buysignal;
    bool sellsignal;
    int opensell;
    int openbuy;
    int closesell = 0;
    int closebuy = 0;

    gblSpread = MarketInfo(Symbol(), MODE_SPREAD);

    movmedStart(0);
    movmedStart(1);


    //-------------------------------------------------------------------+

    // time check

    //-------------------------------------------------------------------

    if ((TimeCurrent() >= StrToTime(StartTime)) && (TimeCurrent() <= StrToTime(EndTime))) {

        TradeTimeOk = 1;

    } else {
        TradeTimeOk = 1;
    }

    //-----------------------------------------------------------------

    // indicator checks

    //-----------------------------------------------------------------

    // Ma strategy one

    double MA1_bc = iMA(NULL, PERIOD, Ma1Period, Ma1Shift, Ma1Method, Ma1AppliedPrice, 0);

    double MA1_bp = iMA(NULL, PERIOD, Ma1Period, Ma1Shift, Ma1Method, Ma1AppliedPrice, 1);

    double MA1_bl = iMA(NULL, PERIOD, Ma1Period, Ma1Shift, Ma1Method, Ma1AppliedPrice, 2);

    // Ma strategy two

    double MA2_bc = iMA(NULL, PERIOD, Ma2Period, Ma2Shift, Ma2Method, Ma2AppliedPrice, 0);

    double MA2_bp = iMA(NULL, PERIOD, Ma2Period, Ma2Shift, Ma2Method, Ma2AppliedPrice, 1);

    double MA2_bl = iMA(NULL, PERIOD, Ma2Period, Ma2Shift, Ma2Method, Ma2AppliedPrice, 2);

    //------------------opening criteria------------------------

    if ((MA1_bc < MA2_bc) && (MA1_bp < MA2_bp) && (MA1_bl > MA2_bl)) {
        buysignal = true;
    } else {
        buysignal = false;
        freeze = "";
    }

    string ma1;
    string ma2;
    string ma3;
    string ma4;

    if (MA1_bc < MA2_bc) {
        ma1 = "ok";
    } else {
        ma1 = "--";
    }

    if (MA1_bp < MA2_bp) {
        ma2 = "ok";
    } else {
        ma2 = "--";
    }

    if (MA1_bl > MA2_bl) {
        ma3 = "ok";
    } else {
        ma3 = "--";
    }

    if (freeze != "Buying trend") {
        ma4 = "ok";
    } else {
        ma4 = "--";
    }

    ObjectSetText("label_buy",
        "Buy: " + ma4 + " || " +
        "" + ma3 + " | " +
        "" + ma2 + " | " +
        "" + ma1 + " | ",
        7, "Arial", DarkOrange);

    if ((MA1_bc > MA2_bc) && (MA1_bp > MA2_bp) && (MA1_bl < MA2_bl)) {
        sellsignal = true;
    } else {
        sellsignal = false;
        freeze = "";
    }

    if (MA1_bc > MA2_bc) {
        ma1 = "ok";
    } else {
        ma1 = "--";
    }

    if (MA1_bp > MA2_bp) {
        ma2 = "ok";
    } else {
        ma2 = "--";
    }

    if (MA1_bl < MA2_bl) {
        ma3 = "ok";
    } else {
        ma3 = "--";
    }

    if (freeze != "Selling trend") {
        ma4 = "ok";
    } else {
        ma4 = "--";
    }

    ObjectSetText("label_sell",
        "Sell: " + ma4 + " || " +
        "" + ma3 + " | " +
        "" + ma2 + " | " +
        "" + ma1 + " | ",
        7, "Arial", DarkOrange);



    //------------------------------closing criteria--------------

    if ((MA1_bc > MA2_bc) && (MA1_bp > MA2_bp) && (MA1_bl < MA2_bl)) {
        closebuy = 1;
    } else {
        closebuy = 0;
    }

    if ((MA1_bc < MA2_bc) && (MA1_bp < MA2_bp) && (MA1_bl > MA2_bl)) {
        closesell = 1;
    } else {
        closesell = 0;
    }

    //-----------------------------------------------------------------------------------------------------

    // Opening criteria

    //-----------------------------------------------------------------------------------------------------

    // Open buy

    if ((buysignal == true) && (closebuy != 1) && (freeze != "Buying trend") && (TradeTimeOk == 1)) {
        if (AllowBuy == true && gblHaltBuy[0] != true)
            openbuy = OrderSend(Symbol(), OP_BUY, gblLots[0], Ask, gblSlippage[0], 0, 0, "Stochastic rsi buy order " + PERIOD, MagicNumber, 0, Blue);
    }





    // Open sell

    if ((sellsignal == true) && (closesell != 1) && (freeze != "Selling trend") && (TradeTimeOk == 1)) {
        if (AllowSell == true && gblHaltSell[0] != true)
            opensell = OrderSend(Symbol(), OP_SELL, gblLots[0], Bid, gblSlippage[0], 0, 0, "Stochastic rsi sell order " + PERIOD, MagicNumber, 0, Red);
    }

    /*
        if (buysignal == true) {
            freeze = "Buying trend";
        }

        if (sellsignal == true) {
            freeze = "Selling trend";
        }
     */
    //-------------------------------------------------------------------------------------------------

    // Closing criteria

    //-------------------------------------------------------------------------------------------------

    if (closesell == 1 || closebuy == 1 || openbuy < 1 || opensell < 1) {// start

        if (OrdersTotal() > 0) {

            for (i = 1; i <= OrdersTotal(); i++) { // Cycle searching in orders



                if (OrderSelect(i - 1, SELECT_BY_POS) == true) { // If the next is available

                    if (OrderMagicNumber() == MagicNumber && OrderType() == OP_BUY && OrderSymbol() == Symbol()) {
                        freeze = "Buying trend";
                    }

                    if (OrderMagicNumber() == MagicNumber && OrderType() == OP_SELL && OrderSymbol() == Symbol()) {
                        freeze = "Selling trend";
                    }

                    if (CloseOnReverseSignal == true) {

                        if (OrderMagicNumber() == MagicNumber && closebuy == 1 && OrderType() == OP_BUY && OrderSymbol() == Symbol() && (OrderStopLoss() >= OrderOpenPrice())) {
                            OrderClose(OrderTicket(), OrderLots(), Bid, gblSlippage[0], CLR_NONE);
                        }

                        if (OrderMagicNumber() == MagicNumber && closesell == 1 && OrderType() == OP_SELL && OrderSymbol() == Symbol() && (OrderStopLoss() <= OrderOpenPrice())) {
                            OrderClose(OrderTicket(), OrderLots(), Ask, gblSlippage[0], CLR_NONE);
                        }

                    }

                    // set stops
                    double new_sl = 0;
                    if ((OrderMagicNumber() == MagicNumber) && (OrderSymbol() == Symbol()) && (OrderType() == OP_BUY)) {
                        perc = (Bid - OrderOpenPrice()) / (OrderTakeProfit() - OrderOpenPrice()) * 100;

                        if (movmedIsHighCandle(PERIOD, 1)) {
                            new_sl = iLow(Symbol(), PERIOD, 1) - gblSL[0]*Point;
                        }

                    } else if ((OrderMagicNumber() == MagicNumber) && (OrderSymbol() == Symbol()) && (OrderType() == OP_SELL)) {
                        perc = (OrderOpenPrice() - Ask) / (OrderOpenPrice() - OrderTakeProfit()) * 100;

                        if (movmedIsLowCandle(PERIOD, 1)) {
                            new_sl = iHigh(Symbol(), PERIOD, 1) + gblSL[0]*Point;
                        }
                    }

                    if ((perc < -15.0) && (OrderStopLoss() == 0) && (OrderMagicNumber() == MagicNumber) && (OrderSymbol() == Symbol()) && (OrderType() == OP_BUY)) {
                        OrderClose(OrderTicket(), OrderLots(), Bid, gblSlippage[0], CLR_NONE);
                    }

                    if ((perc < -15.0) && (OrderStopLoss() == 0) && (OrderMagicNumber() == MagicNumber) && (OrderSymbol() == Symbol()) && (OrderType() == OP_SELL)) {
                        OrderClose(OrderTicket(), OrderLots(), Ask, gblSlippage[0], CLR_NONE);
                    }

                    // Calculate take profit

                    double tpb = NormalizeDouble(OrderOpenPrice() + gblTP[0]*Point, Digits - 1);

                    double tps = NormalizeDouble(OrderOpenPrice() - gblTP[0]*Point, Digits - 1);

                    // Calculate stop loss

                    double slb = NormalizeDouble(OrderOpenPrice() - (gblSL[0]) * Point, Digits - 1);

                    double sls = NormalizeDouble(OrderOpenPrice() + (gblSL[0]) * Point, Digits - 1);




                    RefreshRates();

                    if ((OrderMagicNumber() == MagicNumber) && (OrderTakeProfit() == 0) && (OrderSymbol() == Symbol()) && (OrderType() == OP_BUY)) {
                        OrderModify(OrderTicket(), 0, OrderStopLoss(), tpb, 0, CLR_NONE);
                    }

                    if ((OrderMagicNumber() == MagicNumber) && (OrderTakeProfit() == 0) && (OrderSymbol() == Symbol()) && (OrderType() == OP_SELL)) {
                        OrderModify(OrderTicket(), 0, OrderStopLoss(), tps, 0, CLR_NONE);
                    }




                    if ((OrderMagicNumber() == MagicNumber) && (OrderStopLoss() == 0) && (OrderSymbol() == Symbol()) && (OrderType() == OP_BUY)) {
                        OrderModify(OrderTicket(), 0, slb, OrderTakeProfit(), 0, CLR_NONE);
                    }

                    if ((OrderMagicNumber() == MagicNumber) && (OrderStopLoss() == 0) && (OrderSymbol() == Symbol()) && (OrderType() == OP_SELL)) {
                        OrderModify(OrderTicket(), 0, sls, OrderTakeProfit(), 0, CLR_NONE);
                    }


                    if ((OrderMagicNumber() == MagicNumber) && (OrderStopLoss() != 0) && (OrderSymbol() == Symbol()) && (OrderType() == OP_BUY) && (OrderStopLoss() < OrderOpenPrice()) && ((Bid - OrderOpenPrice()) > (OrderOpenPrice() - OrderStopLoss()))) {
                        OrderModify(OrderTicket(), 0, OrderOpenPrice(), OrderTakeProfit(), 0, CLR_NONE);
                    }

                    if ((OrderMagicNumber() == MagicNumber) && (OrderStopLoss() != 0) && (OrderSymbol() == Symbol()) && (OrderType() == OP_SELL) && (OrderStopLoss() > OrderOpenPrice()) && ((OrderOpenPrice() - Ask) > (OrderStopLoss() - OrderOpenPrice()))) {
                        OrderModify(OrderTicket(), 0, OrderOpenPrice(), OrderTakeProfit(), 0, CLR_NONE);
                    }

                    // SL up candles
                    if (OrderMagicNumber() == MagicNumber &&
                        OrderSymbol() == Symbol() &&
                        OrderType() == OP_BUY &&
                        new_sl != 0 &&
                        new_sl > OrderStopLoss() &&
                        OrderStopLoss() != 0) {
                        OrderModify(OrderTicket(), 0, new_sl, OrderTakeProfit(), 0, CLR_NONE);
                    }

                    if (OrderMagicNumber() == MagicNumber &&
                        OrderSymbol() == Symbol() &&
                        OrderType() == OP_SELL &&
                        new_sl != 0 &&
                        new_sl < OrderStopLoss() &&
                        OrderStopLoss() != 0) {
                        OrderModify(OrderTicket(), 0, new_sl, OrderTakeProfit(), 0, CLR_NONE);
                    }

                    double swapStop = 0;

                    if (OrderMagicNumber() == MagicNumber &&
                        OrderSymbol() == Symbol() &&
                        OrderSwap() < 0) {
                        swapStop = -(OrderSwap() / OrderLots()) * Point;
                    }

                    if (OrderMagicNumber() == MagicNumber &&
                        OrderSymbol() == Symbol() &&
                        OrderType() == OP_BUY &&
                        OrderTakeProfit() != 0 &&
                        OrderSwap() < 0 &&
                        Bid > OrderOpenPrice() &&
                        OrderStopLoss() >= OrderOpenPrice() &&
                        ((OrderStopLoss() - OrderOpenPrice()) < swapStop)) {
                        OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + swapStop, OrderTakeProfit(), 0, CLR_NONE);
                    }

                    if (OrderMagicNumber() == MagicNumber &&
                        OrderSymbol() == Symbol() &&
                        OrderType() == OP_SELL &&
                        OrderTakeProfit() != 0 &&
                        OrderSwap() < 0 &&
                        Ask < OrderOpenPrice() &&
                        OrderStopLoss() <= OrderOpenPrice() &&
                        ((OrderOpenPrice() - OrderStopLoss()) < swapStop)) {
                        OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - swapStop, OrderTakeProfit(), 0, CLR_NONE);
                    }


                } // if available

            } // cycle

        }// orders total





    }// stop

    return (0);
}

/******************************************************************************/

int movmedCheckOpenOrders(int index)
{
    int i = 0;

    if (OrdersTotal() > 0) {
        for (i = 1; i <= OrdersTotal(); i++) {
            if (OrderSelect(i - 1, SELECT_BY_POS) == true) {
                if (OrderMagicNumber() == MagicNumber) {
                    if (OrderSymbol() == Symbol()) {
                        if (OrderType() == OP_BUY) {
                            gblHaltBuy[index] = true;
                        } else if (OrderType() == OP_SELL) {
                            gblHaltSell[index] = true;
                        }
                    }
                }
            }
        }
    }
}

/******************************************************************************/

bool movmedIsHighCandle(int timeframe, int shift)
{
    bool ret = false;
    double open = 0;
    double close = 0;

    open = iOpen(Symbol(), timeframe, shift);
    close = iClose(Symbol(), timeframe, shift);

    if (close >= open) {
        ret = true;
    }

    return (ret);
}

/******************************************************************************/

bool movmedIsLowCandle(int timeframe, int shift)
{
    bool ret = false;
    double open;
    double close;

    open = iOpen(Symbol(), timeframe, shift);
    close = iClose(Symbol(), timeframe, shift);

    if (close <= open) {
        ret = true;
    }

    return (ret);
}

/******************************************************************************/

int movmedStart(int index)
{
    int i = 0;
    double low = 0;
    double high = 0;
    double lenCandle = 0;

    gblMedCandles[index] = 0;

    for (i = 21; i != 0; i--) {
        high = iHigh(Symbol(), gblTimeFrame[index], index);
        low = iLow(Symbol(), gblTimeFrame[index], index);
        lenCandle = high - low;
        gblMedCandles[index] += lenCandle;
    }

    gblMedCandles[index] /= 21;
    gblMedCandles[index] /= Point;
    gblMedCandles[index] /= gblMedCandlesFactor[index];

    gblStopMultd[index] = gblMedCandles[index] / gblSpread;

    if (gblStopMultd[index] == 0) {
        gblStopMultd[index] = 1;
    }

    gblSlippage[index] = gblSlip * gblStopMultd[index];

    gblTP[index] = NormalizeDouble(gblSpread * 10 * gblStopMultd[index], Digits);

    gblSL[index] = NormalizeDouble(gblSpread * gblStopMultd[index], Digits);

    gblLots[index] = NormalizeDouble(AccountBalance()*0.01 / gblSL[index], 2);

    if (gblLots[index] < 0.01) {
        gblLots[index] = 0.01;
    }

    ObjectSetText("label_" + index,
        "Med(" + DoubleToStr(gblMedCandles[index], 0) + ") " +
        "TP(" + DoubleToStr(gblTP[index], 0) + ") " +
        "SL(" + DoubleToStr(gblSL[index], 0) + ") " +
        "Lots(" + DoubleToStr(gblLots[index], 2) + ") " +
        "Perc(" + DoubleToStr(perc, 2) + "%) " +
        "Mult(" + gblStopMultd[index] + ") ",
        7, "Arial", DarkOrange);

    perc = 0;

    /*
     * Check open orders
     */
    movmedCheckOpenOrders(index);
}

/******************************************************************************/

