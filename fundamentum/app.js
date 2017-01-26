const Cvm = require('./lib/cvm');
const Database = require('./lib/database');
const nconf = require('nconf');
const Log = require('./lib/log');

const cvm = new Cvm();
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

if (nconf.get('update-database')) {
  cvm.updateAllCompanies(e => {
    if (!e) {
      log.debug('Completo');
    } else console.log(e);
  });
} else {
  database.getDividendYield(opt, (e, res) => {
    if (!e) {
      //console.log(res);
      res.forEach((item, index) => {
        log.info(`${(index + 1)}: ${item.negociationCode}(${item.totalDfps}) last year: ${item.yearBase} avg: ${item.averageCashPerStock.toPrecision(5)} +/- (${item.stdevCashPercent.toPrecision(4)}%) quote: ${item.quote.toPrecision(4)} yield: ${item.yield.toPrecision(3)}% totalStocks: ${item.totalStocks}`);
      });
    } else {
      log.error(e);
    }
  });
}

