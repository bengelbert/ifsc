const Crawler = require("crawler");
const Events = require('events');
const debug = require('debug')('investboost:crawler');
const nconf = require('nconf');
 
nconf.file({ file: 'cvm.json' });

const list = [];
const colist = [];
const crawler = new Crawler({ maxConnections : 10 });
const events = new Events();

let counter = 0;

events.on('end', () => {
  debug(`end: ${list.length}`);
  //nconf.set('cvm', list);
  //nconf.save();
});

events.on('endGetInfo', () => {
  debug(`Total CVM: ${colist.length}`);
  nconf.set('cvm', colist);
  nconf.save();
});

function crawlerGetInfo(options = {}, callback = () => {}) {
  const cias = options['cias-cd-cvm'];
  const id = options.id;
  crawler.queue({
    uri: `http://bvmf.bmfbovespa.com.br/pt-br/mercados/acoes/empresas/ExecutaAcaoConsultaInfoEmp.asp?CodCVM=${cias['CÓDIGO CVM']}&ViewDoc=0`,
    callback: (e, res, done) => {
      if (!e) {
        const $ = res.$;
        let lineCodes = null;
        cias['Nome de Pregão'] = $('#accordionDados').find('td').eq(1).text().trim(),
        cias['Atividade Principal'] = $('#accordionDados').find('td').eq(7).text().trim(),
        cias['Classificação Setorial'] = $('#accordionDados').find('td').eq(9).text().trim(),
        cias['Site'] = $('#accordionDados').find('td').eq(11).text().trim()
        lineCodes = $('#accordionDados').find('td').eq(3).text().trim().split('\n')[1];
        if (lineCodes && lineCodes.indexOf('Nenhum') < 0) {
          cias['Códigos de Negociação'] = [];
          lineCodes.trim().split(';').forEach((code) => {
            if (code.trim() !== '') {
              cias['Códigos de Negociação'].push(code.trim());
            }
          });
          colist.push(cias);
        }
        counter++;
        debug(counter, list.length, `(${((counter / list.length) * 100).toFixed(2)}%)`, cias['Códigos de Negociação']);
        if (counter >= list.length)
          events.emit('endGetInfo');
      } else {
        console.log(e);
      }
      done();
    }
  });
}

for (let char of 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789') {
  crawler.queue({ 
    uri: `http://cvmweb.cvm.gov.br/SWB/Sistemas/SCW/CPublica/CiaAb/FormBuscaCiaAbOrdAlf.aspx?LetraInicial=${char}`,
    // This will be called for each crawled page 
    callback: (e, res, done) => {
      if (!e) {
        const $ = res.$;
        // $ is Cheerio by default 
        //a lean implementation of core jQuery designed specifically for the server 
        $('#dlCiasCdCVM tr:not(:first-child)').each(function (i) {
          const row = {};
          // ---
          row.CNPJ = $(this).find('td').eq(0).text().trim();
          row.NOME = $(this).find('td').eq(1).text().trim();
          row['TIPO DE PARTICIPANTE'] = $(this).find('td').eq(2).text().trim();
          row['CÓDIGO CVM'] = $(this).find('td').eq(3).text().trim();
          row['SITUAÇÃO REGISTRO'] = $(this).find('td').eq(4).text().trim();
          // ---
          if (row['SITUAÇÃO REGISTRO'].indexOf('Cancelado') === -1) {
            crawlerGetInfo({ 'cias-cd-cvm': row });
            list.push(row);
          }
        });

        events.emit('end');
      }
      done();
    }
  });
}

