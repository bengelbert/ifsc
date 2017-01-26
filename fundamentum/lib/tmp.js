const node_xj = require("xls-to-json");

node_xj({
  input: "database/cap.xls",  // input xls 
  output: "database/cap.json", // output json 
  sheet: "Plan1"  // specific sheetname 
}, (err, result) => {
  if(err) {
    console.error(err);
  } else {
    console.log(result);
  }
});