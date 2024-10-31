const process = require("process");

const mode = process.argv[2];
if (process.argv.length !== 3 || (mode !== "broken" && mode !== "fixed")) {
  console.error(
    `Usage: node index.js [broken|fixed] ${JSON.stringify(process.argv)}`
  );
  process.exit(1);
}

// Load the compiled addon
const addon = require("./build/Release/addon");

console.log(`Calling run_${mode}()`);
addon[`run_${mode}`]();
setTimeout(() => {
  console.log("Exiting");
}, 100);
