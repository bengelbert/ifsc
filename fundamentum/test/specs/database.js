const async = require('async'),
  should = require('chai').should(),
  Database = require('../../lib/database'),
  timeseries = require("timeseries-analysis");

const database = new Database();

describe('#consultInfo()', () => {
  it('Deve retornar os dados brutos do banco de dados referente ao codigo cvm da empresa', (done) => {
    database.consultInfo({ Ccvm: '20257' }, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      res.should.be.a('object');
      done();
    });
  });
});

describe('#getDividendsPV()', () => {
  it('Deve retornar o valor pago em dividendos corrigido pro valor presente referente ao total de ações atuais de empresas que tenham ações ON', (done) => {
    database.getDividendsPV({ Ccvm: '1023', Wdn: 6 }, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      res.should.be.a.instanceof(Array);
      done();
    });
  });

  it('Deve retornar o valor pago em dividendos corrigido pro valor presente referente ao total de ações atuais de empresas que tenham ações ON e com escala de 1', (done) => {
    database.getDividendsPV({ Ccvm: '5762', Wdn: 6 }, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      res.should.be.a.instanceof(Array);
      done();
    });
  });

  it('Deve retornar o valor pago em dividendos corrigido pro valor presente referente ao total de ações atuais de empresas que tenham ações PN e ON', (done) => {
    database.getDividendsPV({ Ccvm: '20257', Wdn: 6 }, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      res.should.be.a.instanceof(Array);
      done();
    });
  });

  it('Deve retornar o valor pago em dividendos corrigido pro valor presente referente ao total de ações atuais de empresas que tenham ações PN de valor 10% maiores que as ON', (done) => {
    database.getDividendsPV({ Ccvm: '18724', Wdn: 6 }, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      res.should.be.a.instanceof(Array);
      done();
    });
  });

  it('Deve retornar o valor pago em dividendos corrigido pro valor presente referente ao total de ações atuais de empresas que tenham ações PNA e PNB', (done) => {
    database.getDividendsPV({ Ccvm: '14311', Wdn: 6 }, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      res.should.be.a.instanceof(Array);
      done();
    });
  });
});

describe('#getDividendAverageFromAllCompanies()', () => {
  it('Deve retornar os dividendos corrigidos pro valor presente de todas as empresas', (done) => {
    database.getDividendAverageFromAllCompanies({}, (e, res) => {
      should.not.exist(e);
      should.exist(res);
      //console.log(res);
      done();
    });
  });
})
