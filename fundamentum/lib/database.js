const async = require('async');
const CONST = require('./const');
const debug = require('debug')('database');
const Events = require('events');
const Exception = require('./exception');
const fs = require('fs');
const googleStocks = require('google-stocks');
const Log = require('./log');
const stats = require("stats-lite");
const timeseries = require("timeseries-analysis");

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

  consultInfo(options, callback) {
    try {
      const ccvm = options.Ccvm;
      async.parallel({
        loadCap: cb => fs.readFile('database/cap.json', 'utf8', (e, res) => cb(e, res)),
        loadCompany: cb => fs.readFile(`database/companies/${ccvm}.json`, 'utf8', (e, res) => cb(e, res)),
      }, (e, res) => {
        if (e) callback(e);
        else {
          const info = {};
          const loadCap = JSON.parse(res.loadCap);
          const loadCompany = JSON.parse(res.loadCompany);
          const cap = loadCap.find(f => f['Nome de pregão'].trim() === loadCompany.ConsultaInfoEmp['Nome de Pregão'].trim());
          info.weekendInfo = cap;
          info.anualInfo = loadCompany;
          callback(null, info);
        }
      });
    } catch (e) {
      this.exception.create('consultInfo', e, callback);
    }
  }

  getDividendsPV(options, callback) {
    try {
      const ccvm = options.Ccvm;
      const wdn = options.Wdn;
      this.consultInfo({ Ccvm:ccvm }, (e, res) => {
        if (e) callback(e);
        else {
          const dividendsPV = [];
          const dividendsNoPV = [];
          if (res.anualInfo.dfps && res.weekendInfo && res.weekendInfo['Código'] !== '' && res.weekendInfo['Código'] !== undefined) {
            let lastYear = 0;
            res.anualInfo.dfps.forEach((dfp, index) => {
              if (index < wdn) {
                if (index === 0) lastYear = dfp.Documento.DataReferenciaDocumento;
                const dividends = {};
                const scale = (dfp.Documento.CodigoEscalaQuantidade === '2') ? 1000 : 1;
                dividends.year = dfp.Documento.DataReferenciaDocumento;
                dividends.ordStocks = dfp.ComposicaoCapitalSocial.QuantidadeAcaoOrdinariaCapitalIntegralizado * scale
                dividends.prefStocks = dfp.ComposicaoCapitalSocial.QuantidadeAcaoPreferencialCapitalIntegralizado * scale;
                dividends.cash = [];
                dfp.PagamentoProventoDinheiro.forEach((cash) => {
                  let stockSpecies = '';
                  if (cash.CodigoEspecieAcao === 'ON')
                    stockSpecies = cash.CodigoEspecieAcao;
                  else 
                    if (typeof (cash.CodigoClasseAcaoPreferencial) === 'object')
                      stockSpecies = cash.CodigoEspecieAcao;
                    else {
                      if (res.weekendInfo.Classe1.trim() === '-' && 
                          res.weekendInfo.Classe2.trim() === '-' &&
                          res.weekendInfo.Classe3.trim() === '-' &&
                          res.weekendInfo.Classe4.trim() === '-' &&
                          res.weekendInfo.Classe5.trim() === '-' &&
                          res.weekendInfo.Classe6.trim() === '-') {
                        stockSpecies = cash.CodigoEspecieAcao;
                      } else
                        stockSpecies = cash.CodigoClasseAcaoPreferencial;
                    }
                  const found = dividends.cash.find(f => f.species === stockSpecies);
                  if (!found) {
                    const money = {
                      species: stockSpecies,
                      perStockValue: Number(cash.ValorProventoPorAcao),
                    }
                    dividends.cash.push(money);
                  } else {
                    found.perStockValue += Number(cash.ValorProventoPorAcao);
                  }
                });
                const on = dividends.cash.find(f => f.species === 'ON');
                const pn = dividends.cash.find(f => f.species === 'PN');
                const pna = dividends.cash.find(f => f.species === 'PNA');
                const pnb = dividends.cash.find(f => f.species === 'PNB');
                const pnc = dividends.cash.find(f => f.species === 'PNC');
                const pnd = dividends.cash.find(f => f.species === 'PND');
                const pne = dividends.cash.find(f => f.species === 'PNE');
                const pnf = dividends.cash.find(f => f.species === 'PNF');
                if (on && pn && !pna && !pnb) {
                  dividends.pnRate = pn.perStockValue / on.perStockValue;
                  dividends.payout = dividends.ordStocks * on.perStockValue + dividends.prefStocks * pn.perStockValue;
                } else if (on && !pn && pna && !pnb) {
                  dividends.pnaRate = pna.perStockValue / on.perStockValue;
                  dividends.payout = dividends.ordStocks * on.perStockValue + dividends.prefStocks * pna.perStockValue;
                } else if (on && !pn && pna && pnb) {
                  dividends.pnaRateStock = (res.weekendInfo.Classe1 === 'PA') ? res.weekendInfo['Qtde ações1'] / res.weekendInfo['Qtde ações preferenciais'] : 1;
                  dividends.pnbRateStock = (res.weekendInfo.Classe2 === 'PB') ? res.weekendInfo['Qtde ações2'] / res.weekendInfo['Qtde ações preferenciais'] : 1;
                  dividends.pnaRate = pna.perStockValue / on.perStockValue;
                  dividends.pnbRate = pnb.perStockValue / on.perStockValue;
                  dividends.payout = dividends.ordStocks * on.perStockValue + (dividends.prefStocks * dividends.pnaRateStock * pna.perStockValue) + (dividends.prefStocks * dividends.pnbRateStock * pnb.perStockValue);
                } else if (on && !pn && !pna && !pnb) {
                  dividends.payout = dividends.ordStocks * on.perStockValue;
                } else {
                  //console.log(`${ccvm} type not defined ON[${on}] PN[${pn}] PNA[${pna}] PNB[${pnb}] PNC[${pnc}] PND[${pnd}] PNE[${pne}] PNF[${pnf}]`);
                }
                dividendsNoPV.push(dividends);
              }
            });
            //dividendsPV.code = res.weekendInfo['Código'].trim();
            //dividendsPV.lastYear = lastYear;
            //if (ccvm === '21733') console.log(JSON.stringify(dividendsNoPV));
            dividendsNoPV.forEach((item) => {
              const pv = {};
              const on = item.cash.find(f => f.species === 'ON');
              const pn = item.cash.find(f => f.species === 'PN');
              const pna = item.cash.find(f => f.species === 'PNA');
              const pnb = item.cash.find(f => f.species === 'PNB');
              const pnc = item.cash.find(f => f.species === 'PNC');
              const pnd = item.cash.find(f => f.species === 'PND');
              const pne = item.cash.find(f => f.species === 'PNE');
              const pnf = item.cash.find(f => f.species === 'PNF');
              pv.code = res.weekendInfo['Código'].trim();
              pv.lastYear = lastYear;
              pv.on = 0;
              pv.pn = 0;
              pv.pna = 0;
              pv.pnb = 0;
              pv.pnc = 0;
              pv.pnd = 0;
              pv.pne = 0;
              pv.pnf = 0;
              //if (ccvm === '21733') console.log(JSON.stringify(on));
              pv.year = item.year;
              if (on && pn && !pna && !pnb) {
                pv.on = item.payout / (Number(res.weekendInfo['Qtde ações ordinárias']) + item.pnRate * Number(res.weekendInfo['Qtde ações preferenciais']));
                pv.pn = item.pnRate * pv.on;
              } else if (on && !pn && pna && !pnb) {
                pv.on = item.payout / (Number(res.weekendInfo['Qtde ações ordinárias']) + item.pnaRate * Number(res.weekendInfo['Qtde ações preferenciais']));
                pv.pna = item.pnaRate * pv.on;
              } else if (on && !pn && pna && pnb) {
                pv.on = item.payout / (Number(res.weekendInfo['Qtde ações ordinárias']) + (item.pnaRate * item.pnaRateStock * Number(res.weekendInfo['Qtde ações preferenciais']) + (item.pnbRate * item.pnbRateStock * Number(res.weekendInfo['Qtde ações preferenciais']))));
                pv.pna = item.pnaRate * pv.on;
                pv.pnb = item.pnbRate * pv.on;
              } else if (on && !pn && !pna && !pnb) {
                pv.on = item.payout / Number(res.weekendInfo['Qtde ações ordinárias']);
              } else if (!on && !pn && !pna && !pnb && !pnc && !pnd && !pne && !pnf) {
                
              } else {
                //console.log(`${res.weekendInfo['Código']} ${ccvm} ON[${on}] PN[${pn}] PNA[${pna}] PNB[${pnb}] PNC[${pnc}] PND[${pnd}] PNE[${pne}] PNF[${pnf}]`);
              }
              dividendsPV.push(pv);
            });
          }
          //console.log(dividendsPV.filter(f => f.code === 'BBAS'));
          //debug('getDividendsPV()');
          //debug(dividendsPV);
          callback(null, dividendsPV);
        }
      });
    } catch (e) {
      this.exception.create('getDividendsPV', e, callback);
    }
  }

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
            let scaleBase = 1;
            company.dfps.forEach((dfp, index) => {
              if (index < wdn) {
                if (index === 0) {
                  ordStocksBase = (!shareCapital) ? dfp.ComposicaoCapitalSocial.QuantidadeAcaoOrdinariaCapitalIntegralizado : shareCapital['Qtde ações ordinárias'];
                  prefStocksBase = (!shareCapital) ? dfp.ComposicaoCapitalSocial.QuantidadeAcaoPreferencialCapitalIntegralizado : shareCapital['Qtde ações preferenciais'];
                  yearBase = dfp.Documento.DataReferenciaDocumento;
                  scaleBase = (!shareCapital) ? ((dfp.Documento.CodigoEscalaQuantidade === '2') ? 1000 : 1) : 1;
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
                    let species = null;
                      if (cash.CodigoEspecieAcao === 'ON')
                        species = cash.CodigoEspecieAcao;
                      else {
                        if (typeof (cash.CodigoClasseAcaoPreferencial) === 'object')
                          species = cash.CodigoEspecieAcao;
                        else
                          species = cash.CodigoClasseAcaoPreferencial;
                      }

                    dividend.ccvm = ccvm,
                    dividend.year = dfp.Documento.DataReferenciaDocumento;
                    dividend.yearBase = yearBase;
                    dividend.totalDfps = Math.min(company.dfps.length, wdn),
                    dividend.scale = (dfp.Documento.CodigoEscalaQuantidade === '2') ? 1000 : 1;
                    dividend.scaleBase = scaleBase;
                    dividend.cashPerStock = Number(cash.ValorProventoPorAcao);
                    dividend.species = species;
                    dividend.ordStocks = dfp.ComposicaoCapitalSocial.QuantidadeAcaoOrdinariaCapitalIntegralizado * dividend.scale,
                    dividend.prefStocks = dfp.ComposicaoCapitalSocial.QuantidadeAcaoPreferencialCapitalIntegralizado * dividend.scale,
                    //dividend.totalStocks = (cash.CodigoEspecieAcao === 'ON') ? dfp.ComposicaoCapitalSocial.QuantidadeAcaoOrdinariaCapitalIntegralizado : dfp.ComposicaoCapitalSocial.QuantidadeAcaoPreferencialCapitalIntegralizado;
                    dividend.ordStocksBase = ordStocksBase * scaleBase;
                    dividend.prefStocksBase = prefStocksBase * scaleBase;
                    dividend.tradingName = company.ConsultaInfoEmp['Nome de Pregão'];
                    dividend.negociationCode = company.ConsultaInfoEmp['Códigos de Negociação'].find((code) => {
                      if (code.indexOf('3') == 4 && cash.CodigoEspecieAcao === 'ON') return true;
                      else if (code.indexOf('5') == 4 && cash.CodigoEspecieAcao === 'PN' && cash.CodigoClasseAcaoPreferencial === 'PNA') return true;
                      else if (code.indexOf('6') == 4 && cash.CodigoEspecieAcao === 'PN' && cash.CodigoClasseAcaoPreferencial === 'PNB') return true;
                      else if (code.indexOf('4') == 4 && cash.CodigoEspecieAcao === 'PN') return true;
                    });

                    //dividend.totalStocks *= dividend.scale;
                    //dividend.ordStocksBase *= dividend.scale;
                    //dividend.prefStocksBase *= dividend.scale;
                    //if (!dividend.negociationCode) dividend.negociationCode = company.ConsultaInfoEmp['Códigos de Negociação'][0];
                    //console.log(`${dividend.tradingName} ${dividend.negociationCode} ${JSON.stringify(species)}`);
                    if (dividend.ordStocks === NaN || dividend.prefStocks === NaN) {
                      console.log(`Deu NaN: ${dividend.negociationCode} ${dividend.year}`);
                    }
                    if (dividend.negociationCode) {
                      dividendList.push(dividend);
                    }
                  } else {
                    found.cashPerStock += Number(cash.ValorProventoPorAcao);
                  }
                });
              }
            });
            //console.log(dividendList.filter(f => f.tradingName === 'TAESA'));
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
      debug(`getDividendAverageFromAllCompanies() => options: ${JSON.stringify(options)}`);
      const wdn = options.Wdn;
      async.mapSeries(this.getCcvmFileList(),
      (item, cb) => {
        const ccvm = item.split('.')[0];
        this.getDividendsPV({ Ccvm: ccvm, Wdn: wdn }, (e, res) => {
          cb(e, res);
        });
      }, (e, res) => {
        if (!e) {
          this.getDividendStatisticsFromList({ DividendListArray: res }, (err, result) => {
            if (err) callback(err);
            else callback(null, result);
          });
        } else callback(e);
      });
    } catch (e) {
      callback(e);
    }
  }

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

  appendUnitToList(options, callback) {
    try {
      const dividendList = options.DividendList,
        unitList = [
          { code: 'ALUP11', ordCode: 'ALUP3', ordAmount: 1, prefCode: 'ALUP4', prefAmount: 2 },
          { code: 'ENGI11', ordCode: 'ENGI3', ordAmount: 1, prefCode: 'ENGI4', prefAmount: 4 },
          { code: 'KLBN11', ordCode: 'KLBN3', ordAmount: 1, prefCode: 'KLBN4', prefAmount: 4 },
          { code: 'RNEW11', ordCode: 'RNEW3', ordAmount: 1, prefCode: 'RNEW4', prefAmount: 2 },
          { code: 'SANB11', ordCode: 'SANB3', ordAmount: 1, prefCode: 'SANB4', prefAmount: 1 },
          { code: 'SULA11', ordCode: 'SULA3', ordAmount: 1, prefCode: 'SULA4', prefAmount: 2 },
          { code: 'TAEE11', ordCode: 'TAEE3', ordAmount: 1, prefCode: 'TAEE4', prefAmount: 2 },
          { code: 'TIET11', ordCode: 'TIET3', ordAmount: 1, prefCode: 'TIET4', prefAmount: 4 },
          { code: 'VVAR11', ordCode: 'VVAR3', ordAmount: 1, prefCode: 'VVAR4', prefAmount: 2 },
        ];
      unitList.forEach((unit) => {
        const on = dividendList.find((f) => f.code === unit.ordCode);
        const pn = dividendList.find((f) => f.code === unit.prefCode);
        if (on && pn) {
          const data = {
            lastYear: on.lastYear,
            code: unit.code,
            average: on.average * unit.ordAmount + pn.average * unit.prefAmount,
            stdev: on.stdev,
            totYearsWithProvents: on.totYearsWithProvents,
          };
          dividendList.push(data);
        }
      });
      //console.log(dividendList);
      callback(null, dividendList);
    } catch (e) {
      this.exception.create('appendUnitToList', e, callback);
    }
  }

  getDividendStatisticsFromList(options, callback) {
    try {
      const array = options.DividendListArray;
      const avgList = [];
      array.forEach((list) => {
        //console.log(list.filter(f => f.code ==='CIEL'));
        //list.forEach((l) => {
          //console.log(list[0]);
          //const name = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'code' }));
          //console.log(name);
          debug(list);
          const on = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'on' }));
          if (list[0] && list[0].code && !isNaN(on.mean())) {
            const data = {
              lastYear: list[0].lastYear,
              code: `${list[0].code}3`,
              average: on.mean(),
              stdev: (on.stdev() / on.mean() * 100).toFixed(2),
              totYearsWithProvents: on.output().filter(f => f[1] > 0).length, 
            }
            avgList.push(data);
          }
          const pn = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'pn' }));
          if (list[0] && list[0].code && !isNaN(pn.mean())) {
            const data = {
              lastYear: list[0].lastYear,
              code: `${list[0].code}4`,
              average: pn.mean(),
              stdev: (pn.stdev() / pn.mean() * 100).toFixed(2),
              totYearsWithProvents: pn.output().filter(f => f[1] > 0).length, 
            }
            avgList.push(data);
          }
          const pna = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'pna' }));
          if (list[0] && list[0].code && !isNaN(pna.mean())) {
            const data = {
              lastYear: list[0].lastYear,
              code: `${list[0].code}5`,
              average: pna.mean(),
              stdev: (pna.stdev() / pna.mean() * 100).toFixed(2),
              totYearsWithProvents: pna.output().filter(f => f[1] > 0).length, 
            }
            avgList.push(data);
          }
          const pnb = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'pnb' }));
          if (list[0] && list[0].code && !isNaN(pnb.mean())) {
            const data = {
              lastYear: list[0].lastYear,
              code: `${list[0].code}6`,
              average: pnb.mean(),
              stdev: (pnb.stdev() / pnb.mean() * 100).toFixed(2),
              totYearsWithProvents: pnb.output().filter(f => f[1] > 0).length, 
            }
            avgList.push(data);
          }
          const pnc = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'pnc' }));
          if (list[0] && list[0].code && !isNaN(pnc.mean())) {
            const data = {
              lastYear: list[0].lastYear,
              code: `${list[0].code}7`,
              average: pnc.mean(),
              stdev: (pnc.stdev() / pnc.mean() * 100).toFixed(2),
              totYearsWithProvents: pnc.output().filter(f => f[1] > 0).length, 
            }
            avgList.push(data);
          }
          const pnd = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'pnd' }));
          if (list[0] && list[0].code && !isNaN(pnd.mean())) {
            const data = {
              lastYear: list[0].lastYear,
              code: `${list[0].code}8`,
              average: pnd.mean(),
              stdev: (pnd.stdev() / pnd.mean() * 100).toFixed(2),
              totYearsWithProvents: pnd.output().filter(f => f[1] > 0).length, 
            }
            avgList.push(data);
          }
          const pne = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'pne' }));
          if (list[0] && list[0].code && !isNaN(pne.mean())) {
            const data = {
              lastYear: list[0].lastYear,
              code: `${list[0].code}9`,
              average: pne.mean(),
              stdev: (pne.stdev() / pne.mean() * 100).toFixed(2),
              totYearsWithProvents: pne.output().filter(f => f[1] > 0).length, 
            }
            avgList.push(data);
          }
          const pnf = new timeseries.main(timeseries.adapter.fromDB(list, { date: 'year', value: 'pnf' }));
          if (list[0] && list[0].code && !isNaN(pnf.mean())) {
            const data = {
              lastYear: list[0].lastYear,
              code: `${list[0].code}10`,
              average: pnf.mean(),
              stdev: (pnf.stdev() / pnf.mean() * 100).toFixed(2),
              totYearsWithProvents: pnf.output().filter(f => f[1] > 0).length, 
            }
            avgList.push(data);
          }
        //});
      });
      //console.log(avgList.filter(f => f.code === 'CIEL3'))
      this.appendUnitToList({ DividendList: avgList }, (e, res) => {
        callback(e, res);
      });
    } catch (e) {
      callback(e);
    }
  }

  getQuoteFromList(options = {}, callback = () => {}) {
    try {
      debug(`getQuoteFromList() => options: ${options}, len: ${options.QuoteList.length}`);
      const quoteList = options.QuoteList;
      debug(`${JSON.stringify(quoteList)}`);
      let quoteListAux = [];
      const quoteListfull = [];
      quoteList.forEach((item, index) => {
        if (index > 0 && !(index % 100)) {
          quoteListfull.push(quoteListAux);
          quoteListAux = [];
        }
        quoteListAux.push(item);
      });
      quoteListfull.push(quoteListAux);
      async.map(quoteListfull, (quoteList, cb) => {
        googleStocks(quoteList, (e, res) => {
          cb(e, res);
        });
      }, (e, res) => {
        try {
          if (e) throw e;
          const quoteList = [];
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
  getDividendYield(options, callback) {
    try {
      debug(`getDividendYieldgetDividendYield() => options: ${JSON.stringify(options)}`);
      let avgList = [];
      let dividendYieldList = [];
      const filterDivYieldMin = options.FilterDivYieldMin || 0;
      const filterDivYieldMax = options.FilterDivYieldMax || 50000;
      const stdevMax = options.FilterStdevMax || 1000;
      const sortBy = options.SortBy || 'yield';
      async.waterfall([
      cb => this.getDividendAverageFromAllCompanies(options, (e, res) => cb(e, res)),
      (res, cb) => {
        const quoteList = [];
        avgList = res;
        res.forEach((item) => {
          quoteList.push(`BVMF:${item.code}`);
        });
        this.getQuoteFromList({ QuoteList: quoteList }, (e, res) => cb(e, res));
      }], (e, res) => {
        try {
          if (e) throw e;
          const quoteList = res;
          //console.log(avgList);
          avgList.forEach((item) => {
            const quote = quoteList.find((f) => {
              return (f.t === item.code);
            });
            if (quote) {
              const last = Number(quote.l);
              if (last > 0 && item.stdev > 0) {
                const dividendYield = (item.average / last) * 100;
                item.yield = dividendYield.toFixed(2);
                item.quote = last;
                if (item.yield > filterDivYieldMin && item.yield < filterDivYieldMax && item.stdev < stdevMax) {
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
