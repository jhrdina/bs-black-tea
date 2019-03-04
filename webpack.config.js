const path = require("path");

const isProd = process.env.NODE_ENV === "production";

module.exports = {
  entry: {
    Example: "./lib/js/examples/Example.js",
    ReactExample: "./lib/js/examples/ReactExample.js"
  },
  mode: isProd ? "production" : "development",
  output: {
    filename: "[name].bundle.js",
    path: path.join(__dirname, "lib")
  }
};
