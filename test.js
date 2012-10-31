var kana2voca = require('kana2voca');

// 非同期版
kana2voca.async('キョーハイーテンキダナー', function(err, result) {
	console.log('非同期:');
	console.log(result);
});

// 同期版
console.log('同期:');
console.log( kana2voca.sync('キョーハイーテンキダナー') );
