/**
 *
 *
 */
const async = require('async');
const CONST = require('./const');
const Events = require('events');
const Exception = require('./exception');
const fs = require('fs');
const googleStocks = require('google-stocks');
const Log = require('./log');
const stats = require("stats-lite");

const priv = {};
/**
 *
 *
 */
class Database {
  constructor() {
    this.name = 'Database';
    this.exception = new Exception({ name: this.name });
    this.log = new Log({ name: this.name });
    this.companies = {};
  }
  /**
   *
   *
   */
  getDividendList(options = {}, callback = () => {}) {
    try {
      const ccvm = options.Ccvm;
      const wdn = options.Wdn;
      const path = `database/companies/${ccvm}.json`;
      // ---
      if (!ccvm) throw new Error('invalid ccvm');
      // ---
      fs.readFile(path, 'utf8', (e, data) => {
        try {
          if (e) throw new Error(e);
          // ---
          const dividendList = [];
          const company = JSON.parse(data);
          const shareCapital = priv.shareCapitalList.find(item => {
            return (item['Nome de pregão'] === company.ConsultaInfoEmp['Nome de Pregão']);
          });
          
          if (company.dfps) {
            let ordStocksBase = 0;
            let prefStocksBase = 0;
            let yearBase = '';
            company.dfps.forEach((dfp, index) => {
              if (index < wdn) {
                if (index === 0) {
                  ordStocksBase = (!shareCapital) ? dfp.ComposicaoCapitalSocial.QuantidadeAcaoOrdinariaCapitalIntegralizado : shareCapital['Qtde ações ordinárias'];
                  prefStocksBase = (!shareCapital) ? dfp.ComposicaoCapitalSocial.QuantidadeAcaoPreferencialCapitalIntegralizado : shareCapital['Qtde ações preferenciais'];
                  yearBase = dfp.Documento.DataReferenciaDocumento;
                }
                // ---
                dfp.PagamentoProventoDinheiro.forEach((cash) => {
                  const dividend = {};
                  const found = dividendList.find((item) => {
                    if ((item && item.species) === cash.CodigoEspecieAcao && item.year === dfp.Documento.DataReferenciaDocumento)
                      return true;
                  });
                  // ---
                  if (!found) {
                    dividend.ccvm = ccvm,
                    dividend.year = dfp.Documento.DataReferenciaDocumento;
                    dividend.yearBase = yearBase;
                    dividend.totalDfps = Math.min(company.dfps.length, wdn),
                    dividend.scale = (dfp.Documento.CodigoEscalaQuantidade === '2') ? 1000 : 1;
                    dividend.cashPerStock = Number(cash.ValorProventoPorAcao);
                    dividend.species = cash.CodigoEspecieAcao;
                    dividend.totalStocks = (cash.CodigoEspecieAcao === 'ON') ? dfp.ComposicaoCapitalSocial.QuantidadeAcaoOrdinariaCapitalIntegralizado : dfp.ComposicaoCapitalSocial.QuantidadeAcaoPreferencialCapitalIntegralizado;
                    dividend.ordStocksBase = ordStocksBase;
                    dividend.prefStocksBase = prefStocksBase;
                    dividend.tradingName = company.ConsultaInfoEmp['Nome de Pregão'];
                    dividend.negociationCode = company.ConsultaInfoEmp['Códigos de Negociação'].find((code) => {
                      if (code.indexOf('3') == 4 && cash.CodigoEspecieAcao === 'ON') return true;
                      else if (code.indexOf('5') == 4 && cash.CodigoEspecieAcao === 'PN' && cash.CodigoClasseAcaoPreferencial === 'PNA') return true;
                      else if (code.indexOf('6') == 4 && cash.CodigoEspecieAcao === 'PN' && cash.CodigoClasseAcaoPreferencial === 'PNB') return true;
                      else if (code.indexOf('4') == 4 && cash.CodigoEspecieAcao === 'PN') return true;
                    });

                    dividend.totalStocks *= dividend.scale;
                    //dividend.ordStocksBase *= dividend.scale;
                    //dividend.prefStocksBase *= dividend.scale;
  
                    //if (!dividend.negociationCode) dividend.negociationCode = company.ConsultaInfoEmp['Códigos de Negociação'][0];
                    if (dividend.negociationCode) {
                      dividendList.push(dividend);
                    }
                  } else {
                    found.cashPerStock += Number(cash.ValorProventoPorAcao);
                  }
                });
              }
            });
            callback(null, dividendList);
          } else {
            callback(null, dividendList);
          }
        } catch (err) {
          this.log.error('getDividendList', err);
          callback(err);
        }
      });
    } catch (e) {
      this.log.error('getDividendList', e);
      callback(e);
    }
  }

  loadShareCapitalList(callback) {
    try {
      this.log.debug('loadShareCapitalList');
      fs.readFile('database/cap.json', 'utf8', (e, out) => {
        if (!e) {
          priv.shareCapitalList = JSON.parse(out);
          callback(null);
        } else this.exception.create('loadShareCapitalList', e, callback);
      });
    } catch (e) {
      this.exception.create('loadShareCapitalList', e, callback);
    }
  }

  getDividendAverageFromAllCompanies(options, callback) {
    try {
      const wdn = options.Wdn;
      const allDividendAvarage = [];
      async.map(this.getCcvmFileList(),
      (item, cb) => {
        const ccvm = item.split('.')[0];
        async.waterfall([
          cb => this.getDividendList({ Ccvm: ccvm, Wdn: wdn }, (e, res) => cb(e, res)),
          (res, cb) => this.getDividendAvarageFromList({ DividendList: res }, (e, res) => cb(e, res))
        ], (e, res) => {
          cb(e, res);
        });
      }, (e, res) => {
        if (!e) {
          res.forEach((item) => {
            if (item.length > 0) {
              item.forEach((subitem) => {
                allDividendAvarage.push(subitem);
              })
            }
          })
          callback(null, allDividendAvarage);
        } else callback(e);
      });
    } catch (e) {
      callback(e);
    }
  }
  /**
   *
   *
   */
  getDividendStdevFromAverage(options = {}, callback = () => {}) {
    try {
      const wdn = options.Wdn;
      const filterStdevMax = options.FilterStdevMax;
      const dividendStdedList = [];
      this.getDividendAverageFromAllCompanies({ Wdn: wdn }, (e, res) => {
        if (!e) {
          res.forEach((item) => {
            if (item.negociationCode.indexOf('Nenhum') < 0) {
              if (item.cash.length < item.totalDfps) {
                for (; item.cash.length < item.totalDfps ;) {
                  item.cash.push(0);
                }
              }
              item.stdevCash = stats.stdev(item.cash);
              item.stdevCashPercent = (item.stdevCash / item.averageCashPerStock) * 100;
              
              if (!filterStdevMax || (filterStdevMax && item.stdevCashPercent < filterStdevMax)) {
                dividendStdedList.push(item);
              }
            }
          });
          callback(null, dividendStdedList);
        } else {
          callback(e);
        }
      });
    } catch (e) {
      this.exception.create('getDividendStdevFromAverage', e, callback);
    }
  }
  /**
   *
   *
   */
  getDividendAvarageFromList(options = {}, callback = () => {}) {
    try {
      const dividendAvarage = [];
      const dividendList = options.DividendList.sort((a, b) => {
        return (b.year - a.year);
      });
      // ---
      dividendList.forEach((dividend, index) => {
        const found = dividendAvarage.find((item) => {
          return (item.negociationCode === dividend.negociationCode);
        });
        // ---
        if (!found) {
          const data = {
            ccvm: dividend.ccvm,
            tradingName: dividend.tradingName,
            negociationCode: dividend.negociationCode,
            yearBase: dividend.yearBase,
            totalStocks: (dividend.species === 'ON') ? Number(dividend.ordStocksBase) : Number(dividend.prefStocksBase) ,
            totalCashPerStock: dividend.cashPerStock,
            species: dividend.species,
            cash: [],
            year: [],
            totalDfps: dividend.totalDfps
          }
          data.year.push(dividend.year);
          data.cash.push(dividend.cashPerStock);
          dividendAvarage.push(data);
        } else {
          const cashPerStock = (dividend.cashPerStock * dividend.totalStocks) / found.totalStocks;
          found.year.push(dividend.year);
          found.cash.push(cashPerStock);
          found.totalCashPerStock = stats.sum(found.cash);
          found.averageCashPerStock = found.totalCashPerStock / found.totalDfps;
        }
      });
      // ---
      callback(null, dividendAvarage);
    } catch (e) {
      callback(e);
    }
  }
  /**
   *
   *
   */
  getQuoteFromList(options = {}, callback = () => {}) {
    try {
      const quoteList = options.QuoteList;
      let quoteListAux = [];
      const quoteListfull = [];
      // ---
      quoteList.forEach((item, index) => {
        if (index > 0 && !(index % 100)) {
          quoteListfull.push(quoteListAux);
          quoteListAux = [];
        }
        quoteListAux.push(item);
      });
      quoteListfull.push(quoteListAux);
      // ---
      async.map(quoteListfull, (quoteList, cb) => {
        googleStocks(quoteList, (e, res) => {
          cb(e, res);
        });
      }, (e, res) => {
        try {
          if (e) throw e;
          // ---
          const quoteList = [];
          // ---
          res.forEach((item) => {
            item.forEach((subitem) => quoteList.push(subitem));
          });
          callback(e, quoteList);
        } catch (e) {
          this.exception.create('getQuoteFromList', e, callback);
        }
      });
    } catch (e) {
      this.exception.create('getQuoteFromList', e, callback);
    }
  }
  /**
   *
   *
   */
  getCcvmFileList() {
    try {
      const files = p => fs.readdirSync(p).filter(f => fs.statSync(`${p}/${f}`).isFile());
      return files(CONST.DATABASE_JSON_PATH);
    } catch (e) {
      throw e;
    }
  }
  /**
   * 
   * 
   */
  getDividendYield(options = {}, callback = () => {}) {
    try {
      let dividendStdedList = [];
      let dividendYieldList = [];
      const filterDivYieldMin = options.FilterDivYieldMin || 0;
      const filterDivYieldMax = options.FilterDivYieldMax || 50000;
      const sortBy = options.SortBy || 'yield';
      // ---
      async.waterfall([
      cb => this.loadShareCapitalList(e => cb(e)),
      cb => this.getDividendStdevFromAverage(options, (e, res) => cb(e, res)),
      (res, cb) => {
        const quoteList = [];
        dividendStdedList = res;
        res.forEach((item) => {
          quoteList.push(`BVMF:${item.negociationCode}`);
        });
        this.getQuoteFromList({ QuoteList: quoteList }, (e, res) => cb(e, res));
      }], (e, res) => {
        try {
          if (e) throw e;
          // ---
          const quoteList = res;
          dividendStdedList.forEach((item) => {
            const quote = quoteList.find((f) => {
              return (f.t === item.negociationCode);
            });
            // ---
            if (quote) {
              const last = Number(quote.l);
              if (last > 0 && item.stdevCashPercent > 0) {
                const dividendYield = (item.averageCashPerStock / last) * 100;
                item.yield = dividendYield;
                item.quote = last;
                if (item.yield > filterDivYieldMin && item.yield < filterDivYieldMax) {
                  dividendYieldList.push(item);
                }
              }
            } 
          });
          
          dividendYieldList = dividendYieldList.sort((a, b) => {
            return (b[`${sortBy}`] - a[`${sortBy}`]);
          });
          callback(null, dividendYieldList);
        } catch (e) {
          this.exception.create('getDividendYield', e, callback);
        }
      });
    } catch (e) {
      this.exception.create('getDividendYield', e, callback);
    }
  }
}

module.exports = Database;
