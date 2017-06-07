const async = require('async');
const cheerio = require('cheerio');
const debug = require('debug')('cvm');
const Events = require('events');
const Exception = require('./exception');
const fs = require('fs');
const Log = require('./log');
const nconf = require('nconf');
const progress = require('request-progress');
const request = require('request');
const xml = require('xml2json');
const unzip = require('unzip');

function httpPost(url, callback) {
  const formData = {
    hdnCategoria: 'IDI2',
    hdnPagina: '',
    FechaI: '',
    FechaV: ''
  };

  request({
    url: url,
    method: 'POST',
    json: true,
    form: formData
  }, (err, res, body) => {
    callback(err, body);
  });
}

class Cvm {
  constructor() {
    this.name = 'Cvm';
    this.log = new Log({ name: this.name });
    this.exception = new Exception({ name: this.name });
    this.events = new Events();
    this.companies = {};
    this.companies.consult = [];
    this.dfps = {};
    this.dfps.contents = {};
  }
  
  requestCompaniesRegistrationData(options, callback) {
    try {
      const url = 'http://sistemas.cvm.gov.br/cadastro/SPW_CIA_ABERTA.ZIP',
        donwloadPath = options.DownloadPath;
      
      progress(request(url))
      .on('progress', (progressState) => {
        const progressPercent = progressState.size.transferred / progressState.size.total * 100;
        console.log(`requestCompaniesRegistrationData() progressPercent: ${progressPercent.toPrecision(4)}`)
      })
      .on('error', (e) => {
        callback(e);
      })
      .pipe(fs.createWriteStream(donwloadPath))
      .on('close', () => {
        callback(null, donwloadPath);
      });
    } catch (e) {
      callback(e);
    }
  }
  
  parseCompaniesRegistrationData(options, callback) {
    try {
      const path = options.Path,
        outPath = `${path.split('.zip')[0]}.txt`;
      //console.log(outPath);
      fs.createReadStream(path)
      .pipe(unzip.Parse())
      .on('entry', (entry) => {
        const fileName = entry.path;
        if (fileName.indexOf('.TXT') >= 0) {
          entry.pipe(fs.createWriteStream(outPath))
          .on('close', () => {
            fs.readFile(outPath, 'utf8', (e, res) => {
              if (e) callback(e);
              else {
                let cols = null,
                  colsName = [];
                const lines = res.split('\n'),
                  regDataList = [];
                lines.forEach((line, i) => {
                  const data = {};
                  cols = line.split('\t');
                  if (i === 0) {
                    colsName = cols;
                  } else {
                    colsName.forEach((item, idx) => {
                      if (item === 'DENOMINACAO_SOCIAL' && cols[idx])
                        data[`${item.trim()}`] = cols[idx].replace(/[^0-9A-Za-z\s]+/g, '');
                      else
                        data[`${item.trim()}`] = cols[idx];
                    });
                    regDataList.push(data);
                  }
                });
                callback(null, regDataList);
              }
            });
          });
        } else entry.autodrain();
      });
    } catch (e) {
      callback(e);
    }
  }

  requestCompaniesList(callback = () => {}) {
    try {
      const self = this;
      // ---
      const urls = [];
      for (let char of 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789') {
        urls.push(`http://cvmweb.cvm.gov.br/SWB/Sistemas/SCW/CPublica/CiaAb/FormBuscaCiaAbOrdAlf.aspx?LetraInicial=${char}`);
      }
      // ---
      async.map(urls, (url, cb) => {
        request(url, (err, res, body) => {
          cb(err, body);
        });
      }, (err, res) => {
        try {
          if (!err) {
            this.companies.consult = [];
            res.forEach((r) => {
              const $ = cheerio.load(r);
              $('#dlCiasCdCVM tr:not(:first-child)').each(function (i) {
                try {
                  const row = {};
                  // ---
                  row.CNPJ = $(this).find('td').eq(0).text().trim();
                  row.NOME = $(this).find('td').eq(1).text().trim();
                  row['TIPO DE PARTICIPANTE'] = $(this).find('td').eq(2).text().trim();
                  row['CÓDIGO CVM'] = $(this).find('td').eq(3).text().trim();
                  row['SITUAÇÃO REGISTRO'] = $(this).find('td').eq(4).text().trim();
                  // ---
                  if (row['SITUAÇÃO REGISTRO'].indexOf('Cancelado') === -1) {
                    self.companies.consult.push(row);
                  }
                } catch (e) {
                  this.exception.create('requestCompaniesList', e, callback);
                }
              });
            });
            // ---
            fs.writeFile('database/cvmcolist.json', JSON.stringify(this.companies.consult), (wrErr) => {
              if (!wrErr) {
                callback(null);
              } else {
                this.exception.create('requestCompaniesList', new Error(wrErr), callback);
              }
            });
          } else {
            throw new Error(err);
          }
        } catch (e) {
          this.exception.create('requestCompaniesList', e, callback);
        }
      });
    } catch (e) {
      this.exception.create('requestCompaniesList', e, callback);
    }
  }
  /**
   * 
   * 
   */
  requestDFPList(options = {}, callback = () => {}) {
    try {
      const ccvm = options.Ccvm;
      const cnpj = options.Cnpj;
      const url = `http://siteempresas.bovespa.com.br/consbov/ExibeTodosDocumentosCVM.asp?CNPJ=${cnpj}&CCVM=${ccvm}&TipoDoc=C&QtLinks=20`;
      //console.log(`requestDFPList() => ${JSON.stringify(options)}`);
      httpPost(url, (error, body) => {
        try {
          if (!error) {
            const content = body.split('\n');
            const codeList = [];
            for (let i in content) {
              if (content[i].indexOf('Categoria</b>') >= 0) {
                let state = null;
                for (let j = 1; (content[(Number(i)+j)] && content[(Number(i)+j)].indexOf('Categoria</b>')) === -1; j += 1) {
                  if (content[(Number(i)+j)].indexOf('DFP - ENET - Ativo') >= 0) {
                    state = 'active';
                  } else if (content[(Number(i)+j)].indexOf('DFP - ENET - Inativo') >= 0) {
                    state = 'inactive';
                  }
                  // ---
                  if (state === 'inactive') {
                    if (content[(Number(i)+j)].indexOf('DOWNLOAD') >= 0) {
                      const code = content[(Number(i)+j)].slice(content[(Number(i)+j)].indexOf('fVisualizaArquivo_ENET')).replace(/[^0-9]/g,'').trim();
                      const path = `dl/${ccvm}-${code}.zip`;
                      if (fs.existsSync(path)) {
                        fs.unlinkSync(path);
                      };
                    }
                  } else if (state === 'active') {
                    if (content[(Number(i)+j)].indexOf('DOWNLOAD') >= 0) {
                      const code = content[(Number(i)+j)].slice(content[(Number(i)+j)].indexOf('fVisualizaArquivo_ENET')).replace(/[^0-9]/g,'').trim();
                      if (code !== '') {
                        codeList.push(code);
                      }
                    } 
                  }
                }
              }
            }
            this.requestDownloadDFPList({ Ccvm: ccvm, CodeList: codeList }, callback);
          } else {
            throw new Error(error);
          }
        } catch (e) {
          this.exception.create('requestDFPList', e, callback);
        }
      });
    } catch (e) {
      this.exception.create('requestDFPList', e, callback);
    }
  }
  /**
   * 
   * 
   */
  requestDownloadDFPList(options = {}, callback = () => {}) {
    try {
      const urls = [];
      const ccvm = options.Ccvm;
      for (let code of options.CodeList) {
        urls.push(`https://www.rad.cvm.gov.br/enetconsulta/frmDownloadDocumento.aspx?CodigoInstituicao=1&NumeroSequencialDocumento=${code}`);
      }
      // ---
      //console.log(`requestDownloadDFPList() => ${JSON.stringify(options)}`);
      async.mapSeries(urls, (url, cb) => {
        try {
          const code = url.slice(url.indexOf('NumeroSequencialDocumento')).replace(/[^0-9]/g,'').trim();
          const jsonPath = `database/companies/${ccvm}.json`;
          fs.readFile(jsonPath, 'utf8', (e, data) => {
            try {
              if (!e) {
                const dfps = JSON.parse(data).dfps;
                const dfp = dfps.find((dfp) => {
                  return (dfp.XmlCode === code);
                });
                // ---
                if (!dfp) {
                  //console.log(`requestDownloadDFPList() => DfpCode '${code}' not found`);
                  this.requestDownloadDfp({ Url: url, Ccvm: ccvm, DfpCode: code}, cb);
                } else {
                  //console.log(`requestDownloadDFPList() => DfpCode '${code}' found`);
                  if (!this.companies[`${ccvm}`]) this.companies[`${ccvm}`] = {};
                  if (!this.companies[`${ccvm}`].dfps) this.companies[`${ccvm}`].dfps = [];
                  this.companies[`${ccvm}`].dfps.push(dfp);
                  cb(null);
                }
              } else if (e.code === 'ENOENT') {
                //console.log(`requestDownloadDFPList() => jsonPath '${jsonPath}' not found`);
                this.requestDownloadDfp({ Url: url, Ccvm: ccvm, DfpCode: code}, cb);
              } else cb(e);
            } catch (e) {
              this.exception.create('requestDownloadDFPList', e, cb);
            }
          });
        } catch (e) {
          this.exception.create('requestDownloadDFPList', e, cb);
        }
      }, (err) => {
        if (!err) {
          console.log(`Docs parsed`);
          fs.writeFile(`database/companies/${ccvm}.json`, JSON.stringify(this.companies[`${ccvm}`]), (err) => {
            if (!err) callback(null)
            else this.exception.create('requestDownloadDFPList', new Error(err), callback);
          })
        } else {
          this.exception.create('requestDownloadDFPList', err, callback);
        }
      });
    } catch (e) {
      this.exception.create('requestDownloadDFPList', e, callback);
    }
  }
  /**
   * 
   * 
   */
  requestDownloadDfp(options = {}, callback = () => {}) {
    try {
      const url = options.Url;
      const ccvm = options.Ccvm;
      const dfpCode = options.DfpCode;
      const dlPath = `dl/${ccvm}-${dfpCode}.zip`;
      const stat = (fs.existsSync(dlPath)) ? fs.statSync(dlPath) : null;
      // ---
      if (stat) fs.unlinkSync(dlPath);
      // ---
      //console.log(`requestDownloadDfp() Download => ccvm: ${ccvm} doc: ${dfpCode}`);
      progress(request(url))
      .on('progress', (progressState) => {
        const progressPercent = progressState.size.transferred / progressState.size.total * 100;
        console.log(`requestDownloadDfp() Download => ccvm: ${ccvm} progresso: ${progressPercent.toPrecision(4)}%`);
      })
      .on('error', (progressError) => {
        console.log(progressError);
      })
      .pipe(fs.createWriteStream(dlPath))
      .on('close', () => {
        console.log(`requestDownloadDfp() Download => ccvm: ${ccvm} doc: ${dfpCode} ok`);
        this.parseAllDocuments({ Ccvm: ccvm, XmlCode: dfpCode }, err => callback(err));
      });    
    } catch (e) {
      this.exception.create('requestDownloadDfp', e, callback);
    }
  }
  /**
   * 
   * 
   */
  requestNegociationCode(options = {}, callback = () => {}) {
    try {
      const ccvm = options.Ccvm;
      const url = `http://bvmf.bmfbovespa.com.br/pt-br/mercados/acoes/empresas/ExecutaAcaoConsultaInfoEmp.asp?CodCVM=${ccvm}&ViewDoc=0`;
      //console.log(`requestNegociationCode() => ${url}`);
      request(url, (err, res, body) => {
        try {
          if (!err) {
            let lineCodes = null;
            const $ = cheerio.load(body);
            const data = {
              'Nome de Pregão': $('#accordionDados').find('td').eq(1).text().trim(),
              'Atividade Principal': $('#accordionDados').find('td').eq(7).text().trim(),
              'Classificação Setorial': $('#accordionDados').find('td').eq(9).text().trim(),
              'Site': $('#accordionDados').find('td').eq(11).text().trim()
            }
            lineCodes = $('#accordionDados').find('td').eq(3).text().trim().split('\n')[1];
            if (lineCodes) {
              data['Códigos de Negociação'] = [];
              lineCodes.trim().split(';').forEach((code) => {
                if (code.trim() !== '') {
                  data['Códigos de Negociação'].push(code.trim());
                }
              });
              if (!this.companies[`${ccvm}`]) this.companies[`${ccvm}`] = {};
              this.companies[`${ccvm}`].ConsultaInfoEmp = data;
            }
          callback(null);
          } else {
            callback(err);
          }
        } catch (e) {
          callback(e);
        }
      });
    } catch (e) {
      this.exception.create('requestNegociationCode', e, callback);
    }
  }
  /**
   * 
   * 
   */
  extractAllFiles(options = {}, callback = () => {}) {
    try {
      const ccvm = options.Ccvm;
      const xmlCode = options.XmlCode;
      const path = `dl/${ccvm}-${xmlCode}`;
      // ---
      //this.log.debug('extractAllFiles', `${JSON.stringify(options)}`);
      // ---
      fs.createReadStream(`${path}.zip`)
      .pipe(unzip.Parse())
      .on('entry', (entry) => {
        const fileName = entry.path;
        if (fileName.indexOf('.dfp') >= 0) {
          entry.pipe(fs.createWriteStream(`${path}.dfp`))
          .on('close', () => {
            fs.createReadStream(`${path}.dfp`)
            .pipe(unzip.Parse())
            .on('entry', (entry) => {
              const fileName = entry.path;
              switch (fileName) {
                case 'ComposicaoCapitalSocialDemonstracaoFinanceiraNegocios.xml':
                  entry.pipe(fs.createWriteStream(`${path}-${fileName}`));
                  break;
                case 'Documento.xml':
                  entry.pipe(fs.createWriteStream(`${path}-${fileName}`))
                  break;
                case 'PagamentoProventoDinheiroDemonstracaoFinanceiraNegocios.xml':
                  entry.pipe(fs.createWriteStream(`${path}-${fileName}`))
                  break;
                default:
                  entry.autodrain();
              }
            })
            .on('close', (err) => {
              callback(err);
            });
          });
        } else {
          entry.autodrain();
        }
      });
    } catch (e) {
      console.log(e);
      callback(e);
    }
  }
  /**
   * 
   * 
   */
  parseAllDocuments(options = {}, callback = () => {}) {
    const ccvm = options.Ccvm;
    const xmlCode = options.XmlCode;
    const path = `dl/${ccvm}-${xmlCode}`;
    // ---
    //this.log.debug('parseAllDocuments', `${JSON.stringify(options)}`);
    // ---
    this.dfps.contents = {};
    async.series({
      extractAllFiles: cb => this.extractAllFiles(options, e => cb(e)),
      parseComposicaoCapitalSocialXml: cb => this.parseComposicaoCapitalSocialXml({ FileName: `${path}-ComposicaoCapitalSocialDemonstracaoFinanceiraNegocios.xml` }, e => cb(e)),
      parseDocumentoXml: cb => this.parseDocumentoXml({ FileName: `${path}-Documento.xml` }, e => cb(e)),
      parsePagamentoProventoDinheiroXml: cb => this.parsePagamentoProventoDinheiroXml({ FileName: `${path}-PagamentoProventoDinheiroDemonstracaoFinanceiraNegocios.xml` }, e => cb(e)),
    }, (e) => {
      if (!e) {
        if (!this.companies[`${ccvm}`]) this.companies[`${ccvm}`] = {};
        if (!this.companies[`${ccvm}`].dfps) this.companies[`${ccvm}`].dfps = [];
        this.dfps.contents.XmlCode = xmlCode;
        this.companies[`${ccvm}`].dfps.push(this.dfps.contents);
        fs.unlinkSync(`${path}.zip`);
        fs.unlinkSync(`${path}.dfp`);
        callback(null); 
      } else {
        this.log.error(e);
        callback(e);
      }
    });
  }
  /**
   *
   *
   */
  parseComposicaoCapitalSocialXml(options = {}, callback = () => {}) {
    try {
      const fileName = options.FileName;
      // ---
//      this.log.debug('parseComposicaoCapitalSocialXml', `${JSON.stringify(options)}`);
      // ---
      fs.readFile(fileName, 'utf8', (err, data) => {
        try {
          if (!err) {
            const json = JSON.parse(xml.toJson(data));
            const content = {
              QuantidadeAcaoOrdinariaCapitalIntegralizado: json.ArrayOfComposicaoCapitalSocialDemonstracaoFinanceira.ComposicaoCapitalSocialDemonstracaoFinanceira.QuantidadeAcaoOrdinariaCapitalIntegralizado,
              QuantidadeAcaoPreferencialCapitalIntegralizado: json.ArrayOfComposicaoCapitalSocialDemonstracaoFinanceira.ComposicaoCapitalSocialDemonstracaoFinanceira.QuantidadeAcaoPreferencialCapitalIntegralizado,
              QuantidadeTotalAcaoCapitalIntegralizado: json.ArrayOfComposicaoCapitalSocialDemonstracaoFinanceira.ComposicaoCapitalSocialDemonstracaoFinanceira.QuantidadeTotalAcaoCapitalIntegralizado
            };
            this.dfps.contents.ComposicaoCapitalSocial = content;
            fs.unlinkSync(fileName);
            callback(null);
          } else throw new Error(err);
        } catch (e) {
          this.exception.create('parseComposicaoCapitalSocial', e, callback);
        }
      });
    } catch (e) {
      this.exception.create('parseComposicaoCapitalSocial', e, callback);
    }
  }
  /**
   *
   *
   */
  parseDocumentoXml(options = {}, callback = () => {}) {
    try {
      const fileName = options.FileName;
      // ---
//      this.log.debug('parseDocumentoXml', `${JSON.stringify(options)}`);
      // ---
      fs.readFile(fileName, 'utf8', (err, data) => {
        try {
          if (!err) {
            const json = JSON.parse(xml.toJson(data));
            const content = {
              CompanhiaAberta: json.Documento.CompanhiaAberta,
              DataReferenciaDocumento: json.Documento.DataReferenciaDocumento.split('-')[0],
              CodigoEscalaQuantidade: json.Documento.CodigoEscalaQuantidade
            };
            this.dfps.contents.Documento = content;
            fs.unlinkSync(fileName);
            callback(null);
          } else throw new Error(err);
        } catch (e) {
          this.exception.create('parseDocumentoXml', e, callback);
        }
      });
    } catch (e) {
      this.exception.create('parseDocumentoXml', e, callback);
    }
  }
  /**
   *
   *
   */
  parsePagamentoProventoDinheiroXml(options = {}, callback = () => {}) {
    try {
      const fileName = options.FileName;
      // ---
//      this.log.debug('parsePagamentoProventoDinheiroXml', `${JSON.stringify(options)}`);
      // ---
      fs.readFile(fileName, 'utf8', (err, data) => {
        try {
          if (!err) {
            const json = JSON.parse(xml.toJson(data));
            const paymentList = json.ArrayOfPagamentoProventoDinheiroDemonstracaoFinanceira.PagamentoProventoDinheiroDemonstracaoFinanceira;
            const contentList = [];
            if (paymentList && paymentList.length > 0) {
              paymentList.forEach((payment) => {
                const content = {
                  EventoOrigemProvento: payment.EventoOrigemProvento.DescricaoOpcaoDominio,
                  DataAprovacaoProvento: payment.DataAprovacaoProvento.split('T')[0],
                  TipoProvento: payment.TipoProvento.DescricaoOpcaoDominio,
                    DataInicioPagamento: payment.DataInicioPagamento.split('T')[0],
                  CodigoEspecieAcao: payment.CodigoEspecieAcao.SiglaOpcaoDominio,
                  CodigoClasseAcaoPreferencial: payment.CodigoClasseAcaoPreferencial.SiglaOpcaoDominio,
                  ValorProventoPorAcao: payment.ValorProventoPorAcao
                };
                contentList.push(content);
              });
            }
            this.dfps.contents.PagamentoProventoDinheiro = contentList;
            fs.unlinkSync(fileName);
            callback(null);
          } else throw new Error(err);
        } catch (e) {
          this.exception.create('parsePagamentoProventoDinheiroXml', e, callback);
        }
      });
    } catch (e) {
      this.exception.create('parsePagamentoProventoDinheiroXml', e, callback);
    }
  }
  /**
   * 
   * 
   */
  updateCompany(options = {}, callback = () => {}) {
    try {
      const ccvm = options.Ccvm;
      const cnpj = options.Cnpj;
      debug('updateCompany()');
      debug(options);
      async.series([(cb) => {
        this.requestNegociationCode({ Ccvm: ccvm }, err => cb(err));
      }, (cb) => {
        try {
          if (this.companies[`${ccvm}`]) {
            debug(this.companies[`${ccvm}`]);
            const codes = this.companies[`${ccvm}`].ConsultaInfoEmp['Códigos de Negociação'];
            if (codes.length > 0) {
//              this.log.debug('updateCompany', `codes: ${codes}`);
              this.requestDFPList({ Ccvm: ccvm, Cnpj: cnpj }, err => cb(err));
            }
          } else cb(null);
        } catch (e) {
          this.exception.create('updateCompany', e, cb);
        }
      }], (err, res) => {
        callback(err);
      });
    } catch (e) {
      this.exception.create('updateCompany', e, callback);
    }
  }
  /**
   * 
   * 
   */
  updateAllCompanies(options, callback = () => {}) {
    try {
      const ccvm = options.Ccvm;
      fs.readFile('database/cvmcolist.json', (err, data) => {
        if (!err) {
          let coList = JSON.parse(data);
          coList = (ccvm) ? coList.filter(x => x['CÓDIGO CVM'] === `${ccvm}`) : coList;
          async.mapSeries(coList, (co, cb) => {
            if (co.CNPJ !== '-') {
              //console.log(`updateAllCompanies() => ${co.NOME}`);
              this.updateCompany({ Ccvm: co['CÓDIGO CVM'], Cnpj: co.CNPJ }, (err) => {
                cb(err);
              });
            } else cb(null);
          }, (err, res) => {
            callback(err);
          });
        } else callback(err);
      })
    } catch (e) {
      this.exception.create('updateAllCompanies', e, callback);
    }
  }
}

module.exports = Cvm;
