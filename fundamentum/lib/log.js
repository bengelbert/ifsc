class Log {
  constructor(options = { name: 'Log' }) {
    this.name = options.name;
  }
  /**
   *
   *
   */
  debug(funcName, message) {
    console.log(`[DEBUG] ${this.name}::${funcName}() ${message || ''}`);
  }
  /**
   *
   *
   */
  info(message) {
    console.log(`[INFOS] ${this.name} ${message}`);
  }
  /**
   *
   *
   */
  error(funcName, message) {
    console.log(`[ERROR] ${this.name}::${funcName}() ${message || ''}`);
  }
}

module.exports = Log;
