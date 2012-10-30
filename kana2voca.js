var fs   = require('fs')
  , path = require('path')
  , addon
;

if ( fs.existsSync( path.join(__dirname, './build/Debug') ) ) {
	addon = require('./build/Debug/kana2voca');
} else if ( fs.existsSync( path.join(__dirname, './build/Release') ) ) {
	addon = require('./build/Release/kana2voca');
} else {
	throw '"kana2voca" has not been compiled yet.'
}

module.exports = addon.kana2voca;
