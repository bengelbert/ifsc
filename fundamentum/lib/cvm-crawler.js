const async = require('async');
const Crawler = require("crawler");
const csv = require('csv');
const debug = require('debug')('investboost:cvm-crawler');
const fs = require('fs');
const moment = require('moment');
const nconf = require('nconf');
const progress = require('request-progress');
const request = require('request');
const unzip = require('unzip');

nconf.file('cvm.json');

class CvmCrawler {
  constructor() {
    this.crawler = new Crawler({ maxConnections : 10 });
  }

//=============================================================================

  downloadAllRegistrationData(options = {}, callback = () => {}) {
    const cb = (typeof options === 'function') ? options   : callback;
    const opts = (typeof options === 'object') ? options : undefined;
    const localPath = (opts) ? opts.localPath : '/tmp/SPW_CIA_ABERTA.zip';
    debug('#downloadAllRegistrationData()', opts);
    progress(request('http://sistemas.cvm.gov.br/cadastro/SPW_CIA_ABERTA.ZIP'), {})
      .on('progress', (state) => {
        debug('progress', `${(state.percent * 100).toFixed(2)}%`);
      })
      .on('error', (err) => {
        debug('progress', err);
        cb(err);
      })
      .on('end', () => {
        debug('progress', 'end');
        cb(null, {});
      })
      .pipe(fs.createWriteStream(localPath));
  }

//=============================================================================

  parserAllRegistrationData(options = {}, callback = () => {}) {
    const cb = (typeof options === 'function') ? options   : callback;
    const opts = (typeof options === 'object') ? options : undefined;
    const localPath = (opts) ? opts.localPath : '/tmp/SPW_CIA_ABERTA.zip';
    debug('#parserAllRegistrationData()', opts, localPath);
    this.downloadAllRegistrationData({ 'localPath': localPath }, (e, res) => {
      fs.createReadStream(localPath).pipe(unzip.Extract({ path: '/tmp/' }));
      async.waterfall([
        acb => fs.readFile('/tmp/SPW_CIA_ABERTA.txt', 'binary', (e, res) => acb(e, res)),
        (res, acb) => csv.parse(res.toString(), { 
          columns: true,
          delimiter: '\t',
          relax: true,
          trim: true
        }, (e, res) => acb(e, res)),
      ], (e, res) => {
        cb(e, res);
      });
    });
  }

//=============================================================================

  getAllActiveRegistration(options = {}, callback = () => {}) {
    const force = options.force || false;
    const companies = nconf.get('companies');
    debug('#getAllActiveRegistration()', options);
    if (companies.length <= 0 || force === true) {
      this.parserAllRegistrationData((e, res) => {
        if (!e) {
          const active = res.filter(f => f.SIT_REG === 'ATIVO');
          nconf.set('companies', active);
          nconf.save(() => {
            callback(null, active);
          });
        } else {
          callback(e);
        }
      });
    } else {
      callback(null, companies);
    }
  }

//=============================================================================

  getInfo(options = {}, callback = () => {}) {
    const cdcvm = options.cdcvm || 'all';
    debug('#getAllActiveRegistration()', options);
    this.getAllActiveRegistration(options, (e, res) => {
      if (!e) {
        const colist = [];
        let map = [];
        if (cdcvm !== 'all') {
          map.push(res.find(f => f.CD_CVM === cdcvm));
        } else {
          map = res;
        }
        async.map(map, (item, mapcb) => {
          const uri = `http://bvmf.bmfbovespa.com.br/pt-br/mercados/acoes/empresas/ExecutaAcaoConsultaInfoEmp.asp?CodCVM=${item.CD_CVM}&ViewDoc=0`;
          debug('#getInfo()', `Try request '${uri}'`);
          this.crawler.queue({
            'uri': uri,
            callback: (eCrawlerQueue, resCrawlerQueue, done) => {
              if (!eCrawlerQueue) {
                debug(uri);
                const $ = resCrawlerQueue.$;
                let lineCodes = null;
                item['Nome de Pregão'] = $('#accordionDados').find('td').eq(1).text().trim(),
                item['Atividade Principal'] = $('#accordionDados').find('td').eq(7).text().trim(),
                item['Classificação Setorial'] = $('#accordionDados').find('td').eq(9).text().trim(),
                item['Site'] = $('#accordionDados').find('td').eq(11).text().trim()
                lineCodes = $('#accordionDados').find('td').eq(3).text().trim().split('\n')[1];
                if (lineCodes && lineCodes.indexOf('Nenhum') < 0) {
                  item['Códigos de Negociação'] = [];
                  lineCodes.trim().split(';').forEach((code) => {
                    if (code.trim() !== '') {
                      item['Códigos de Negociação'].push(code.trim());
                    }
                  });
                  this.getDFPListFromCdCvm({ cdcvm: item.CD_CVM, cnpj: item.CNPJ }, (eGetDFPListFromCdCvm, resGetDFPListFromCdCvm) => {
                    if (!e) {
                      item.dfps = resGetDFPListFromCdCvm;
                    }
                    colist.push(item);
                  });
                }
              } 
              done();
              mapcb(eCrawlerQueue);
            }
          });
        }, (error, result) => {
          if (!error) {
            const companies = nconf.get('companies');
            debug(companies.length);
            colist.forEach((item) => {
              let found = companies.find(f => f.CD_CVM === item.CD_CVM);
              if (!found) {
                companies.push = item;
              } else {
                found = item;
              }
            });
            nconf.set('companies', companies);
            nconf.save(() => {
              callback(null, colist);
            });
          } else {
            callback(error);
          }
        });
      } else {
        callback(e);
      }
    });
  }

//=============================================================================

  getDFPListFromCdCvm(options = {}, callback = () => {}) {
    const cnpj = options.cnpj;
    const cdcvm = options.cdcvm;
    const cb = callback;
    const url = `http://siteempresas.bovespa.com.br/consbov/ExibeTodosDocumentosCVM.asp?CNPJ=${cnpj}&CCVM=${cdcvm}&TipoDoc=C&QtLinks=20`;
    debug('#getDFPListFromCdCvm()', options);
    debug('#getDFPListFromCdCvm()', `Try request '${url}'`);
    this.crawler.queue({
      'url': url,
      method: 'POST',
      form: {
        hdnCategoria: 'IDI2',
        hdnPagina: '',
        FechaI: '',
        FechaV: ''
      },
      callback: (eCrawlerQueue, resCrawlerQueue, done) => {
        if (!eCrawlerQueue) {
          const $ = resCrawlerQueue.$;
          const dfpList = [];
          $('.TableOptions').each(function () {
            const active = $(this).find('td').eq(1).text();
            if (active.indexOf('Ativo') >= 0) {
              const date = moment($(this).next().find('td').eq(1).text(), 'DD/MM/YYYY');
              const dfp = $(this).find('a').eq(0).attr('href').match(/\d+/g)[0];
              debug('#getDFPListFromCdCvm()', cdcvm, moment(date).format('YYYY'), dfp);
              const data = {
                year: moment(date).format('YYYY'),
                'dfp': dfp
              }
              dfpList.push(data);
            }
          });
          cb(null, dfpList);
          done();
        }
      }
    });
  }
}

module.exports = CvmCrawler;