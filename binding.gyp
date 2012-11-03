{
	'targets': [{
		'target_name' : 'kana2voca',
		'sources'     : [ 'kana2voca.cc' ],
		'cflags'      : [ '-std=c++0x' ],
		'libraries'   : [ '-licuuc -licui18n -lboost_regex' ],
		'conditions'  : [
			['OS=="mac"', {
				'include_dirs' : [
					'/usr/local/include/libcxx',
				],
				'xcode_settings' : {
					'OTHER_CFLAGS': [
						'-std=c++0x',
					],
				},
			},],
		],
	},],
}
