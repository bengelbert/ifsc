const async = require('async');
const debug = require('debug')('investboost:cvm-crawler-test');
const fs = require('fs');
const should = require('chai').should();
const CvmCrawler = require('../../lib/cvm-crawler');

describe('CvmCrawler', () => {
  describe('#downloadAllRegistrationData()', () => {
    it('Deve fazer o download dos dados cadastrais de todas as companhias abertas.', (done) => {
      const cvmCrawler = new CvmCrawler();
      const localPath= '/tmp/SPW_CIA_ABERTA.zip';
      cvmCrawler.downloadAllRegistrationData({ 'localPath': localPath }, (e, res) => {
        should.not.exist(e);
        should.exist(res);
        fs.existsSync(localPath).should.be.equal(true);
        done();
      });
    });
  });  

  describe('#parserAllRegistrationData()', () => {
    it('Deve fazer o parser dos dados cadastrais de todas as companhias abertas.', (done) => {
      const cvmCrawler = new CvmCrawler();
      const localPath= '/tmp/SPW_CIA_ABERTA.zip';
      cvmCrawler.parserAllRegistrationData({ 'localPath': localPath }, (e, res) => {
        should.not.exist(e);
        should.exist(res);
        const co = res.find(f => f.CD_CVM === '21954');
        should.exist(co);
        done();
      });
    });
  });  

  describe('#getAllActiveRegistration()', () => {
    it('Deve filtrar somente os registros ativos.', (done) => {
      const cvmCrawler = new CvmCrawler();
      cvmCrawler.getAllActiveRegistration({ force: true }, (e, res) => {
        should.not.exist(e);
        should.exist(res);
        const coExist = res.find(f => f.SIT_REG === 'ATIVO');
        should.exist(coExist);
        const coNotExist = res.find(f => f.SIT_REG === 'CANCELADA');
        should.not.exist(coNotExist);
        done();
      });
    });
  });  

  describe('#getDFPListFromCdCvm()', () => {
    it('Deve retornar a lista de DFPs da empresa a partir do código cvm e seu cnpj.', (done) => {
      const cvmCrawler = new CvmCrawler();
      cvmCrawler.getDFPListFromCdCvm({ cdcvm: '18970', cnpj: '4128563000110' }, (e, res) => {
        debug('callback.getDFPListFromCdCvm()', e, res);
        should.not.exist(e);
        should.exist(res);
        done();
      });
    });
  });  

  describe('#getInfo()', () => {
    it('Deve retornar as infos de todas as empresas ativas e em fase operacional seus respectivos códigos bolsa.', (done) => {
      const cvmCrawler = new CvmCrawler();
      cvmCrawler.getInfo({ cdcvm: '18970' }, (e, res) => {
        should.not.exist(e);
        should.exist(res);
        done();
      });
    });
  });  
})
