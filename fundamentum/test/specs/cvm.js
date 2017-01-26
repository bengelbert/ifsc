const chai = require('chai'),
  Cvm = require('../../lib/cvm'),
  fs = require('fs');
  
const should = chai.should(),
  crdDownloadPath = 'test/fixtures/crd.zip';

describe('#requestCompaniesRegistrationData()', () => {
  const cvm = new Cvm();
  it('Deve fazer o download do zip que contém os dados cadastrais de todas as companhias abertas', (done) => {
    cvm.requestCompaniesRegistrationData({ DownloadPath: crdDownloadPath }, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      fs.existsSync(res).should.be.equal(true);
      done();
    });
  });
});

describe('#parseCompaniesRegistrationData()', () => {
  const cvm = new Cvm();
  it('Deve fazer o parser do zip que contém os dados cadastrais de todas as companhias abertas', (done) => {
    cvm.parseCompaniesRegistrationData({ Path: crdDownloadPath }, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      res.should.to.be.instanceof(Array);
      console.log(res[0]);
      done();
    });
  });
});
