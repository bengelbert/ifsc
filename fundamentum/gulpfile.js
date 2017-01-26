const gulp = require('gulp');
const http = require('http');
const connect = require('connect');
const serveStatic = require('serve-static');
const Launcher = require('webdriverio/build/lib/launcher');
const path = require('path');
const wdio = new Launcher(path.join(__dirname, 'wdio.conf.js'));
const mocha = require('gulp-mocha');

const caminhoCodigoFonte = 'lib/**/*.js';
const caminhoCodigoTeste = 'test/**/*.js';

gulp.task('testar', function() {
  gulp.src(caminhoCodigoTeste)
  .pipe(mocha());
});
   
gulp.task('tdd-continuo', ['testar'], function() {
  gulp.watch(caminhoCodigoFonte, ['testar']);
  gulp.watch(caminhoCodigoTeste, ['testar']);
});

const pathDatabaseSrc = 'lib/database.js',
  pathDatabaseTest = 'test/specs/database.js'

gulp.task('database-test', function() {
  gulp.src(pathDatabaseTest)
  .pipe(mocha());
});
   
gulp.task('tdd-database', ['database-test'], function() {
  gulp.watch(pathDatabaseSrc, ['database-test']);
  gulp.watch(pathDatabaseTest, ['database-test']);
});


const pathCvmSrc = 'lib/cvm.js',
  pathCvmTest = 'test/specs/cvm.js'

gulp.task('cvm-test', function() {
  gulp.src(pathCvmTest)
  .pipe(mocha({ timeout: 3000 }));
});
   
gulp.task('tdd-cvm', ['cvm-test'], function() {
  gulp.watch(pathCvmSrc, ['cvm-test']);
  gulp.watch(pathCvmTest, ['cvm-test']);
});
process.on('uncaughtException', function(e) {
  console.error(e.stack);
});
  