/*
require('xls').parse('database/cap.xls', function(err, data) {
  console.log(data);
});

const node_xj = require("xls-to-json");

node_xj({
  input: "database/cap.xls",  // input xls
  output: "database/cap.json", // output json
  sheet: "Plan1"  // specific sheetname
}, (err, result) => {
  if(err) {
    console.error(err);
  } else {
    console.log(result);
  }
});
/*
const stats = require("stats-lite");
var googleFinance = require('google-finance');

const timeseries = require('timeseries-analysis')
const moment = require('moment')
const googleFinance = require('google-finance')

const now = new Date().toJSON()
const past = moment(now).add(-30, 'days').toDate()

googleFinance.companyNews({
  symbol: 'NASDAQ:AAPL'
}, function (err, news) {
  console.log(news)
})

googleFinance.historical({
  symbol: 'NASDAQ:AAPL',
  from: '2014-01-01',
  to: '2014-12-31'
}, function (err, quotes) {
  console.log(quotes)
})

/*
//console.log(jsonDate);

var quoth = require("quoth");

// Listed companies
quoth.spot('BVMF:TUPY3', (err, result) => {
  console.log(result);
 }); */
const { lookup, history } = require('yahoo-stocks')

const main = async () => {
  const response = await history('PETR3.SA', { interval: '1d', range: '1mo' })

  console.log(response)
}

main()
