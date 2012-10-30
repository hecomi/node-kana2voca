var MeCab     = new require('mecab-async')
  , mecab     = new MeCab()
  , kana2voca = require('kana2voca')
;

var str2voca = function(str, callback) {
	mecab.parse(str, function(err, result) {
		var kana = '';
		for (var i in result) {
			kana += result[i][9];
		}
		kana2voca(kana, function(err, result) {
			console.log(result);
		});
	});
}

str2voca('今日はいい天気だなぁ', function(err, result) {
	console.log(result);
});
