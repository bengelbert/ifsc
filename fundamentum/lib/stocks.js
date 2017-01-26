const Events = require('events');
const Exception = require('./exception');
const googleStocks = require('google-stocks');
const Log = require('./log');
const nconf = require('nconf');
/**
 *
 *
 */
class Stocks extends Events {
  constructor() {
    super();
    this.name = 'Stocks';
    this.exception = new Exception({ name: this.name });
    this.log = new Log({ name: this.name });
    this.quotes = {};
    this.updated = false;
    nconf.file('info', { file: 'info.json', dir: 'database/json', search: true });
    this.coList = nconf.get('companies');
    this.on('error', (error, callback = () => {}) => {
      callback(error);
    });
  }
  /**
   *
   *
   */
  getCode(options = {}) {
    try {
      this.log.debug(`getCode(${JSON.stringify(options)})`);
      const co = this.coList.find((l) => {
        return (l.name === options.name && l.type === options.type);
      });
      return co.code;
    } catch (e) {
      this.exception.create(`getCode()::${e.message}`);
      return null;
    }
  }
  /**
   *
   *
   */
  getQuote(options = {}) {
    try {
      this.log.debug(`getQuote(${JSON.stringify(options)})`);
      const co = this.coList.find((l) => {
        return (l.name === options.name && l.type === options.type);
      });
      const quote = this.quotes.find((q) => {
        return (q.t === co.code);
      });
      return quote.l;
    } catch (e) {
      this.exception.create(`getQuote()::${e.message}`);
      return -1;
    }
  }
  /**
   *
   *
   */
  getUnitMult(options = () => {}) {
    try {
      this.log.debug(`getUnitMult(${JSON.stringify(options)})`);
      const co = this.coList.find((l) => {
        return (l.name === options.name && l.type === "UNIT");
      });
      return co[`${options.type}`];
    } catch (e) {
      this.exception.create(`getUnitMult()::${e.message}`);
      return null;
    }
  }
  /**
   *
   *
   */
  getTypes(options = {}) {
    try {
      this.log.debug(`getType(${JSON.stringify(options)})`);
      const types = [];
      this.coList.forEach((co) => {
        if (co.name === options.name) {
          types.push(co.type);
        }
      });
      return types;
    } catch (e) {
      this.exception.create(`getType()::${e.message}`);
      return null;
    }
  }
  /**
   *
   *
   */
  updateQuotes(callback = () => {}) {
    try {
      this.log.debug('updateQuotes()');
      const stocklist = [];
      this.coList.forEach((co) => {
        stocklist.push(`${co.market}:${co.code}`);
      });
      googleStocks(stocklist, (error, data) => {
        if (!error) {
          this.quotes = data;
          this.emit('QuotesUpdated');
          callback(null);
        } else {
          this.emit('error', this.exception.create(`updateQuotes()::${error}`, callback));
        }
      });
    } catch (e) {
      this.emit('error', this.exception.create(`updateQuotes()::${e.message}`, callback));
    }
  }
}

module.exports = Stocks;
