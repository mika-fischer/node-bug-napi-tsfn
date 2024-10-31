const process = require("process");

const modes = ["broken", "fixed", "atomic_flag"];

const mode = process.argv[2];
if (process.argv.length !== 3 || !modes.includes(mode)) {
  console.error(
    `Usage: node index.js [${modes.join("|")}] ${JSON.stringify(process.argv)}`
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
