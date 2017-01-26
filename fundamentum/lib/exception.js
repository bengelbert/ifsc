/**
 *
 *
 */
const Events = require('events');
const Log = require('./log');
/**
 *
 *
 */
class Exception extends Events {
  constructor(options) {
    super();
    this.name = 'Exception';
    this.log = new Log({ name: this.name });
    this.on('error', (functionName, error, callback) => {
      this.log.error(functionName, `${error.message}`);
      callback(error);
    });
  }
  /**
   *
   *
   */
  create(functionName, error, callback = () => {}) {
    this.emit('error', functionName || '', error, callback);
  }
}

module.exports = Exception;
