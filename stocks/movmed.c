/*#define ObjectCreate()  ()
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
#define PERIOD_W1    0*/
/******************************************************************************/

#define MAX_PERIODS 3
#define PERIOD      PERIOD_D1

/******************************************************************************/

double aux = 0;
//---- input parameters
extern string StopSettings = "Set stops below";

extern bool CloseOnReverseSignal = true;

extern string TimeSettings = "Set the time range the EA should trade";

extern string StartTime = "00:00";

extern string EndTime = "23:59";

extern string Ma1 = "First Ma settings";

extern string Ma2 = "Second Ma settings";

extern string MagicNumbers = "Set different magicnumber for each timeframe of a pair";



static int gblHaltBuy[MAX_PERIODS] = {0, 0};
static int gblHaltSell[MAX_PERIODS] = {0, 0};
static int gblMagicNumber[MAX_PERIODS] = {103, 206, 412};
static int gblMa1Period = 16;
static int gblMa1Shift = 8;
static int gblMa1Method = MODE_SMA;
static int gblMa1AppliedPrice = PRICE_MEDIAN;
static int gblMa2Period = 1;
static int gblMa2Shift = 0;
static int gblMa2Method = MODE_SMA;
static int gblMa2AppliedPrice = PRICE_MEDIAN;
static int gblOpenSell[MAX_PERIODS] = {0, 0};
static int gblOpenBuy[MAX_PERIODS] = {0, 0};
static int gblSlip = 5;
static int gblSlippage[MAX_PERIODS] = {0, 0};
static int gblSpread;
static int gblStopMultd[MAX_PERIODS] = {0, 0};
static int gblTimeFrame[MAX_PERIODS] = {PERIOD_D1, PERIOD_W1, PERIOD_MN1};

static bool gblBuySignal[MAX_PERIODS] = {0, 0};
static bool gblCloseSell[MAX_PERIODS] = {0, 0};
static bool gblCloseBuy[MAX_PERIODS] = {0, 0};
static bool gblSellSignal[MAX_PERIODS] = {0, 0};
static bool gblAllowBuy[MAX_PERIODS] = {false, true, true};
static bool gblAllowSell[MAX_PERIODS] = {false, true, true};

static double gblLots[MAX_PERIODS] = {0, 0};
static double gblMA1_bc[MAX_PERIODS] = {0, 0};
static double gblMA1_bp[MAX_PERIODS] = {0, 0};
static double gblMA1_bl[MAX_PERIODS] = {0, 0};
static double gblMA2_bc[MAX_PERIODS] = {0, 0};
static double gblMA2_bp[MAX_PERIODS] = {0, 0};
static double gblMA2_bl[MAX_PERIODS] = {0, 0};
static double gblMedCandles[MAX_PERIODS] = {0, 0};
static double gblMedCandlesFactor[MAX_PERIODS] = {1.25, 1.25, 1.25};
static double gblPercentage[MAX_PERIODS] = {0, 0};
static double gblSL[MAX_PERIODS] = {0, 0};
static double gblTP[MAX_PERIODS] = {0, 0};

static string gblFreeze[MAX_PERIODS] = {"", ""};

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

    ObjectCreate("label_2", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_2", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_2", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_2", OBJPROP_YDISTANCE, 41); // Y coordinate

    ObjectCreate("label_buy0", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_buy0", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_buy0", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_buy0", OBJPROP_YDISTANCE, 54); // Y coordinate

    ObjectCreate("label_sell0", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_sell0", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_sell0", OBJPROP_XDISTANCE, 125); // X coordinate
    ObjectSet("label_sell0", OBJPROP_YDISTANCE, 54); // Y coordinate

    ObjectCreate("label_buy1", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_buy1", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_buy1", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_buy1", OBJPROP_YDISTANCE, 67); // Y coordinate

    ObjectCreate("label_sell1", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_sell1", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_sell1", OBJPROP_XDISTANCE, 125); // X coordinate
    ObjectSet("label_sell1", OBJPROP_YDISTANCE, 67); // Y coordinate

    ObjectCreate("label_buy2", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_buy2", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_buy2", OBJPROP_XDISTANCE, 10); // X coordinate
    ObjectSet("label_buy2", OBJPROP_YDISTANCE, 80); // Y coordinate

    ObjectCreate("label_sell2", OBJ_LABEL, 0, 0, 0); // Creating obj.
    ObjectSet("label_sell2", OBJPROP_CORNER, 0); // Reference corner
    ObjectSet("label_sell2", OBJPROP_XDISTANCE, 125); // X coordinate
    ObjectSet("label_sell2", OBJPROP_YDISTANCE, 80); // Y coordinate

    return (0);
}

/******************************************************************************/

int start()
{
    gblSpread = MarketInfo(Symbol(), MODE_SPREAD);

    movmedStart(0);
    movmedStart(1);
    movmedStart(2);

    return (0);
}

/******************************************************************************/

int movmedCheckBuyOrders(int index)
{
    double slb = 0;
    double tpb = 0;
    double new_sl = 0;
    double swapStop = 0;

    RefreshRates();

    gblFreeze[index] = "Buying trend";

    if (CloseOnReverseSignal == true) {
        if (gblCloseBuy[index] == true && OrderStopLoss() >= OrderOpenPrice()) {
            OrderClose(OrderTicket(), OrderLots(), Bid, gblSlippage[index], CLR_NONE);
        }
    }

    gblPercentage[index] = (Bid - OrderOpenPrice()) / (OrderTakeProfit() - OrderOpenPrice()) * 100;

    if (movmedIsHighCandle(gblTimeFrame[index], 1)) {
        new_sl = iLow(Symbol(), gblTimeFrame[index], 1) - gblSL[index] * Point;
    }

    if ((gblPercentage[index] < -15.0) && (OrderStopLoss() == 0)) {
        OrderClose(OrderTicket(), OrderLots(), Bid, gblSlippage[index], CLR_NONE);
    }

    // Calculate take profit
    tpb = NormalizeDouble(OrderOpenPrice() + gblTP[index] * Point, Digits - 1);

    // Calculate stop loss
    slb = NormalizeDouble(OrderOpenPrice() - (gblSL[index]) * Point, Digits - 1);

    if (OrderTakeProfit() == 0) {
        OrderModify(OrderTicket(), 0, OrderStopLoss(), tpb, 0, CLR_NONE);
    }

    if (OrderStopLoss() == 0) {
        OrderModify(OrderTicket(), 0, slb, OrderTakeProfit(), 0, CLR_NONE);
    }

    if (OrderStopLoss() != 0 && 
        OrderStopLoss() < OrderOpenPrice() && 
        ((Bid - OrderOpenPrice()) > (OrderOpenPrice() - OrderStopLoss()))) {
        OrderModify(OrderTicket(), 0, OrderOpenPrice(), OrderTakeProfit(), 0, CLR_NONE);
    }

    // SL up candles
    if (new_sl != 0 && new_sl > OrderStopLoss() && OrderStopLoss() != 0) {
        OrderModify(OrderTicket(), 0, new_sl, OrderTakeProfit(), 0, CLR_NONE);
    }

    if (OrderSwap() < 0) {
        swapStop = -(OrderSwap() / OrderLots()) * Point;
    }

    if (OrderTakeProfit() != 0 &&
        OrderSwap() < 0 &&
        Bid > OrderOpenPrice() &&
        OrderStopLoss() >= OrderOpenPrice() &&
        ((OrderStopLoss() - OrderOpenPrice()) < swapStop)) {
        OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() + swapStop, OrderTakeProfit(), 0, CLR_NONE);
    }
}

/******************************************************************************/

int movmedCheckSellOrders(int index)
{
    double sls = 0;
    double tps = 0;
    double new_sl = 0;
    double swapStop = 0;

    RefreshRates();

    gblFreeze[index] = "Selling trend";

    if (CloseOnReverseSignal == true) {
        if (gblCloseSell[index] == true && OrderStopLoss() <= OrderOpenPrice()) {
            OrderClose(OrderTicket(), OrderLots(), Ask, gblSlippage[index], CLR_NONE);
        }
    }

    gblPercentage[index] = (OrderOpenPrice() - Ask) / (OrderOpenPrice() - OrderTakeProfit()) * 100;

    if (movmedIsLowCandle(gblTimeFrame[index], 1)) {
        new_sl = iHigh(Symbol(), gblTimeFrame[index], 1) + gblSL[index] * Point;
    }

    if ((gblPercentage[index] < -15.0) && (OrderStopLoss() == 0)) {
        OrderClose(OrderTicket(), OrderLots(), Ask, gblSlippage[index], CLR_NONE);
    }

    // Calculate take profit
    tps = NormalizeDouble(OrderOpenPrice() - gblTP[index] * Point, Digits - 1);

    // Calculate stop loss
    sls = NormalizeDouble(OrderOpenPrice() + (gblSL[index]) * Point, Digits - 1);
    
    if (OrderTakeProfit() == 0) {
        OrderModify(OrderTicket(), 0, OrderStopLoss(), tps, 0, CLR_NONE);
    }

    if (OrderStopLoss() == 0) {
        OrderModify(OrderTicket(), 0, sls, OrderTakeProfit(), 0, CLR_NONE);
    }

    if (OrderStopLoss() != 0 && 
        OrderStopLoss() > OrderOpenPrice() && 
        ((OrderOpenPrice() - Ask) > (OrderStopLoss() - OrderOpenPrice()))) {
        OrderModify(OrderTicket(), 0, OrderOpenPrice(), OrderTakeProfit(), 0, CLR_NONE);
    }

    if (new_sl != 0 && new_sl < OrderStopLoss() && OrderStopLoss() != 0) {
        OrderModify(OrderTicket(), 0, new_sl, OrderTakeProfit(), 0, CLR_NONE);
    }

    if (OrderSwap() < 0) {
        swapStop = -(OrderSwap() / OrderLots()) * Point;
    }

    if (OrderTakeProfit() != 0 &&
        OrderSwap() < 0 &&
        Ask < OrderOpenPrice() &&
        OrderStopLoss() <= OrderOpenPrice() &&
        ((OrderOpenPrice() - OrderStopLoss()) < swapStop)) {
        OrderModify(OrderTicket(), OrderOpenPrice(), OrderOpenPrice() - swapStop, OrderTakeProfit(), 0, CLR_NONE);
    }
}

/******************************************************************************/

int movmedCheckHaltState(int index)
{
    int i = 0;

    if (OrdersTotal() > 0) {
        for (i = 1; i <= OrdersTotal(); i++) {
            if (OrderSelect(i - 1, SELECT_BY_POS) == true) {
                if (OrderMagicNumber() == gblMagicNumber[index]) {
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

int movmedCheckOpenOrders(int index)
{
    int i;

    if (gblCloseSell[index] == 1 || gblCloseBuy[index] == 1 || gblOpenBuy[index] < 1 || gblOpenSell[index] < 1) {// start

        if (OrdersTotal() > 0) {

            for (i = 1; i <= OrdersTotal(); i++) { // Cycle searching in orders

                if (OrderSelect(i - 1, SELECT_BY_POS) == true) { // If the next is available

                    if (OrderSymbol() == Symbol()) {
                        if (OrderMagicNumber() == gblMagicNumber[index]) {
                            if (OrderType() == OP_BUY) {
                                movmedCheckBuyOrders(index);
                            } else if (OrderType() == OP_SELL) {
                                movmedCheckSellOrders(index);
                            }
                        }
                    }
                } // if available
            } // cycle
        }// orders total
    }// stop
}

/******************************************************************************/

int movmedClosingCriteria(int index)
{
    if ((gblMA1_bc[index] > gblMA2_bc[index]) && (gblMA1_bp[index] > gblMA2_bp[index]) && (gblMA1_bl[index] < gblMA2_bl[index])) {
        gblCloseBuy[index] = true;
    } else {
        gblCloseBuy[index] = false;
    }

    if ((gblMA1_bc[index] < gblMA2_bc[index]) && (gblMA1_bp[index] < gblMA2_bp[index]) && (gblMA1_bl[index] > gblMA2_bl[index])) {
        gblCloseSell[index] = true;
    } else {
        gblCloseSell[index] = false;
    }
}

/******************************************************************************/

int movmedIndicatorChecks(int index)
{
    gblMA1_bc[index] = iMA(NULL, gblTimeFrame[index], gblMa1Period, gblMa1Shift, gblMa1Method, gblMa1AppliedPrice, 0);
    gblMA1_bp[index] = iMA(NULL, gblTimeFrame[index], gblMa1Period, gblMa1Shift, gblMa1Method, gblMa1AppliedPrice, 1);
    gblMA1_bl[index] = iMA(NULL, gblTimeFrame[index], gblMa1Period, gblMa1Shift, gblMa1Method, gblMa1AppliedPrice, 2);

    gblMA2_bc[index] = iMA(NULL, gblTimeFrame[index], gblMa2Period, gblMa2Shift, gblMa2Method, gblMa2AppliedPrice, 0);
    gblMA2_bp[index] = iMA(NULL, gblTimeFrame[index], gblMa2Period, gblMa2Shift, gblMa2Method, gblMa2AppliedPrice, 1);
    gblMA2_bl[index] = iMA(NULL, gblTimeFrame[index], gblMa2Period, gblMa2Shift, gblMa2Method, gblMa2AppliedPrice, 2);
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

int movmedOpeningCriteria(int index)
{
    string ma1;
    string ma2;
    string ma3;
    string ma4;

    if ((gblMA1_bc[index] < gblMA2_bc[index]) && (gblMA1_bp[index] < gblMA2_bp[index]) && (gblMA1_bl[index] > gblMA2_bl[index])) {
        gblBuySignal[index] = true;
    } else {
        gblBuySignal[index] = false;
        gblFreeze[index] = "";
    }

    if (gblMA1_bc[index] < gblMA2_bc[index]) {
        ma1 = "ok";
    } else {
        ma1 = "--";
    }

    if (gblMA1_bp[index] < gblMA2_bp[index]) {
        ma2 = "ok";
    } else {
        ma2 = "--";
    }

    if (gblMA1_bl[index] > gblMA2_bl[index]) {
        ma3 = "ok";
    } else {
        ma3 = "--";
    }

    if (gblFreeze[index] != "Buying trend") {
        ma4 = "ok";
    } else {
        ma4 = "--";
    }

    ObjectSetText("label_buy"+index,
        "Buy["+index+"]: " + ma4 + " || " +
        "" + ma3 + " | " +
        "" + ma2 + " | " +
        "" + ma1 + " | ",
        7, "Arial", DarkOrange);

    if ((gblMA1_bc[index] > gblMA2_bc[index]) && (gblMA1_bp[index] > gblMA2_bp[index]) && (gblMA1_bl[index] < gblMA2_bl[index])) {
        gblSellSignal[index] = true;
    } else {
        gblSellSignal[index] = false;
        gblFreeze[index] = "";
    }

    if (gblMA1_bc[index] > gblMA2_bc[index]) {
        ma1 = "ok";
    } else {
        ma1 = "--";
    }
    if (gblMA1_bp[index] > gblMA2_bp[index]) {
        ma2 = "ok";
    } else {
        ma2 = "--";
    }
    if (gblMA1_bl[index] < gblMA2_bl[index]) {
        ma3 = "ok";
    } else {
        ma3 = "--";
    }

    if (gblFreeze[index] != "Selling trend") {
        ma4 = "ok";
    } else {
        ma4 = "--";
    }

    ObjectSetText("label_sell"+index,
        "Sell["+index+"]: " + ma4 + " || " +
        "" + ma3 + " | " +
        "" + ma2 + " | " +
        "" + ma1 + " | ",
        7, "Arial", DarkOrange);

    if ((gblBuySignal[index] == true) && (gblCloseBuy[index] != true) && (gblFreeze[index] != "Buying trend")) {
        if (gblAllowBuy[index] == true && gblHaltBuy[index] != true)
            gblOpenBuy[index] = OrderSend(Symbol(), OP_BUY, gblLots[index], Ask, gblSlippage[index], 0, 0, "Buy order: " + gblTimeFrame[index], gblMagicNumber[index], 0, Blue);
    }

    if ((gblSellSignal[index] == true) && (gblCloseSell[index] != true) && (gblFreeze[index] != "Selling trend")) {
        if (gblAllowSell[index] == true && gblHaltSell[index] != true)
            gblOpenSell[index] = OrderSend(Symbol(), OP_SELL, gblLots[index], Bid, gblSlippage[index], 0, 0, "Sell order: " + gblTimeFrame[index], gblMagicNumber[index], 0, Red);
    }

    /*
        if (buysignal == true) {
            freeze = "Buying trend";
        }

        if (sellsignal == true) {
            freeze = "Selling trend";
        }
     */
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
        "Perc(" + DoubleToStr(gblPercentage[index], 2) + "%) " +
        "Mult(" + gblStopMultd[index] + ") ",
        7, "Arial", DarkOrange);

    gblPercentage[index] = 0;

    /*
     * Check open orders
     */
    movmedCheckHaltState(index);

    /*
     * indicator checks
     */
    movmedIndicatorChecks(index);

    /*
     * closing criteria
     */
    movmedClosingCriteria(index);

    /*
     * opening criteria
     */
    movmedOpeningCriteria(index);

    //-------------------------------------------------------------------------------------------------

    // Closing criteria

    //-------------------------------------------------------------------------------------------------
    movmedCheckOpenOrders(index);
}

/******************************************************************************/

