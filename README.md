Katakana to Julius voca format converter
==============

はじめに
--------------
ICU を利用して、カタカナをローマ字および音声認識エンジン Julius の voca 形式に非同期で変換する Node.js のアドオンです。

インストール
--------------
git clone して次のコマンドを実行して下さい。

	$ npm install kana2voca

C++0x 対応のコンパイラが必要です。

使い方
--------------
```javascript
var kana2voca = require('kana2voca');

kana2voca('キョーハイーテンキダナー', function(err, result) {
	console.log(result);
});
```

これで次のような出力となります。

	ky o: h a i: t e N k i d a n a:

連携
--------------
node-mecab-async (https://github.com/hecomi/node-mecab-async) と組み合わせると、カタカナだけでない文字列をローマ字に出来ます。

```javascript
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
```

詳細
--------------
その他詳細は Twitter:@hecomi へご質問いただくか、http://d.hatena.ne.jp/hecomi/ をご参照下さい。


