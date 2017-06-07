const async = require('async');
const Cvm = require('./lib/cvm');
const CvmCrawler = require('./lib/cvm-crawler');
const Database = require('./lib/database');
const debug = require('debug')('app');
const googleFinance = require('google-finance');
const Log = require('./lib/log');
const moment = require('moment');
const nconf = require('nconf');
const timeseries = require("timeseries-analysis");

const cvm = new Cvm();
const cvmCrawler = new CvmCrawler();
const database = new Database();
const log = new Log({ name: 'App' });

nconf.argv();

const opt = {
  FilterStdevMax: nconf.get('stdev-max') || 50000,
  FilterDivYieldMin: nconf.get('yield-min') || 0,
  FilterDivYieldMax: nconf.get('yield-max') || 50000,
  SortBy: nconf.get('sort-by') || 'yield',
  Wdn: nconf.get('window') || 6,
}

if (nconf.get('cvm-crawler')) {
  cvmCrawler.getInfo({ cdcvm: 'all' });
} else if (nconf.get('update-database')) {
  cvm.updateAllCompanies({
    Ccvm: nconf.get('ccvm')
  },(e) => {
    if (!e) {
      log.debug('Completo');
    } else console.log(e);
  });
} else {
  database.getDividendYield(opt, (e, res) => {
    if (!e) {
      //console.log(res.filter(f => f.tradingName === 'TAESA'));
      const now = new Date().toJSON(),
        past = moment(now).add(-30, 'days').toDate(),
        filterList = [];
      async.mapSeries(res, (item, cb) => {
        googleFinance.historical({
          symbol: `BVMF:${item.code}`, 
          from: past,
          to: now
        }, (err, quotes) => {
          if (!err) {
            const t = new timeseries.main(timeseries.adapter.fromDB(quotes, {
              date: 'date',
              value: 'volume'
            }));
            //const notPay = item.cash.find(x => x === 0);
            if ((!nconf.get('volume-min') || t.mean() > nconf.get('volume-min')) && (!nconf.get('years-with-provents') || item.totYearsWithProvents === nconf.get('years-with-provents'))) {
              //log.info(`${item.code}(item.totalDfps) last year: item.yearBase avg: ${item.average.toFixed(5)} +/- (${item.stdev}%) quote: ${item.quote} yield: ${item.yield}% volume: ${t.mean()}`);
              log.info(`${item.lastYear}[${item.totYearsWithProvents}]: ${item.code} avg: ${item.average.toFixed(5)} +/- (${item.stdev}%) quote: ${item.quote.toFixed(2)} yield: ${item.yield}% volume: ${t.mean().toFixed(2)}`);
              filterList.push(Number(item.stdev));
            }
            cb();
          } else cb(err);
        });
      }, (e) => {
        if (!e) {
          const t = new timeseries.main(timeseries.adapter.fromArray(filterList)); 
          console.log(`Média dos desvios: ${t.mean().toPrecision(4)}% +/-(${t.stdev().toPrecision(4)})`);
        }
      });
    } else {
      log.error(e);
    }
  });
}

