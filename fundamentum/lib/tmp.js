/*
require('xls').parse('database/cap.xls', function(err, data) {
  console.log(data);
});
*/
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
var timeseries = require("timeseries-analysis");
const stats = require("stats-lite");
var googleFinance = require('google-finance');
var moment = require('moment');



googleFinance.companyNews({
  symbol: 'BVMF:ETER3'
}, function (err, news) {
  //if (!err) 
   // console.log(news);
});
 
var now = new Date().toJSON();
var past = moment(now).add(-30, 'days').toDate();


console.log(now);
console.log(past);

googleFinance.historical({
  symbol: 'BVMF:ETER3', 
  from: past,
  to: now
}, function (err, quotes) {
  if (!err) {
    const volume = [];
    var t     = new timeseries.main(timeseries.adapter.fromDB(quotes, {
      date:   'date',     // Name of the property containing the Date (must be compatible with new Date(date) )
      value:  'volume'     // Name of the property containign the value. here we'll use the "close" price.
    }));
    console.log(t.mean());
  }
});

//console.log(jsonDate);

var quoth = require("quoth");
 
// Listed companies 
quoth.spot('BVMF:TUPY3', (err, result) => {
  console.log(result);
 });*/
