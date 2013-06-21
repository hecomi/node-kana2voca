#define BOOST_NO_EXCEPTIONS

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <boost/regex.hpp>
#include <unicode/translit.h>
#include <node.h>

using namespace std;
using namespace v8;
using namespace node;

static uv_mutex_t m;

// 例外を投げないようにする
namespace boost {
	void throw_exception(std::exception const & e)
	{
		return;
	}
}

// 非同期処理でやり取りするデータ
struct kana2voca_baton
{
	string               str;
	string               err;
	string               result;
	Persistent<Function> callback;
};

// ICU でカタカナをローマ字に変換してから
// 音声認識エンジン Julius の voca ファイル形式に変換するクラス（非同期版）
// (e.g. イースタートー --> i: s u t a: t o:)
Handle<Value> kana2voca(const Arguments& args)
{
	HandleScope scope;

	// 引数が文字列かどうかチェック
	if (!args[0]->IsString()) {
		Local<String> msg = String::New("[MeCab] Error! The 1st argument of 'parse' must be String.");
		ThrowException(Exception::TypeError(msg));
		return scope.Close(Undefined());
	}

	// 非同期処理に必要なデータの成形
	String::Utf8Value str(args[0]);
	auto data      = new kana2voca_baton;
	data->str      = *str;
	data->callback = Persistent<Function>::New( args[1].As<Function>() );
	auto req       = new uv_work_t;
	req->data      = data;

	// 非同期で形態素解析して結果を callback 経由で返す
	uv_queue_work(
		uv_default_loop(),
		req,
		[](uv_work_t* req) {
			uv_mutex_lock(&m);

			auto data = static_cast<kana2voca_baton*>( req->data );
			UnicodeString input = data->str.c_str();

			// 「ン」をマーキング
			input.findAndReplace("ン", "[[ン]]");

			// カタカナ --> Latin 変換
			auto error = U_ZERO_ERROR;
			auto t     = Transliterator::createInstance("Katakana-Latin", UTRANS_FORWARD, error);
			t->transliterate(input);
			if (error != U_ZERO_ERROR) {
				data->err = std::string("[kana2voca] Error! Code: ") + std::to_string(error);
				return;
			}

			// 伸ばす音の表記変更 + マーキングしたンをNにする + 「つ」を「q」にする
			std::map<UnicodeString, UnicodeString> long_map = {
				{"\u0101" , "a:"},
				{"\u0113" , "e:"},
				{"\u012B" , "i:"},
				{"\u014D" , "o:"},
				{"\u016B" , "u:"},
				{"[[n]]"  , "N" },
				{"~"      , "q" }
			};
			for (const auto& x : long_map) {
				input.findAndReplace(x.first, x.second);
			}

			// 変換結果取得
			std::vector<char> romaji( input.length() );
			input.extract(0, input.length(), &romaji[0], "utf8");

			// Julius の voca 形式へ整形
			std::string result(romaji.begin(), romaji.end());
			std::map<std::string, std::string> regex_map = {
				{"[aiueoNq]:?" , "$0 "},
				{"[^aiueoNq]+" , "$0 "},
				{"[^a-zN:@]"   , ""   },
				{" +"          , " "  },
			};
			for (const auto& x : regex_map) {
				result = boost::regex_replace(
					result,
					boost::regex(x.first),
					x.second,
					boost::match_default | boost::format_all
				);
				// boost::regex_replace(result, boost::regex("[aiueoNq]\:?"), "$0 ", boost::match_default | boost::format_all);
			}
			data->result = result;

			uv_mutex_unlock(&m);
		},
		(uv_after_work_cb)([](uv_work_t* req, int) {
			auto data  = static_cast<kana2voca_baton*>( req->data );
			unique_ptr<uv_work_t>       preq(req);
			unique_ptr<kana2voca_baton> pdata(data);
			HandleScope scope;

			auto err      = pdata->err;
			auto callback = pdata->callback;

			// 解析結果がエラーの場合はエラーを返す
			if ( !err.empty() ) {
				Local<Value> argv[2] = { String::New( err.c_str() ), String::New("") };
				callback->Call(Context::GetCurrent()->Global(), 2, argv);
			}

			// エラーでない場合は結果を整形して JavaScript へコールバック経由で呼び出す
			std::string voca_str = pdata->result;
			Local<Value> argv[2] = { String::New(""), String::New( voca_str.c_str() ) };
			callback->Call( Context::GetCurrent()->Global(), 2, argv );
		})
	);

	return scope.Close( Undefined() );
};

// ICU でカタカナをローマ字に変換してから
// 音声認識エンジン Julius の voca ファイル形式に変換するクラス（同期版）
// (e.g. イースタートー --> i: s u t a: t o:)
Handle<Value> kana2voca_sync(const Arguments& args)
{
	HandleScope scope;

	// 引数が文字列かどうかチェック
	if (!args[0]->IsString()) {
		Local<String> msg = String::New("[MeCab] Error! The 1st argument of 'parse' must be String.");
		ThrowException( Exception::TypeError(msg) );
		return scope.Close( Undefined() );
	}
	String::Utf8Value str(args[0]);
	UnicodeString input = *str;

	// 「ン」をマーキング
	input.findAndReplace("ン", "[[ン]]");

	// カタカナ --> Latin 変換
	auto error = U_ZERO_ERROR;
	auto t = Transliterator::createInstance("Katakana-Latin", UTRANS_FORWARD, error);

	t->transliterate(input);
	if (error != U_ZERO_ERROR) {
		string err = std::string("[kana2voca] Error! Code: ") + std::to_string(error);
		Local<String> msg = String::New( err.c_str() );
		ThrowException( Exception::TypeError(msg) );
		return scope.Close( Undefined() );
	}

	// 伸ばす音の表記変更 + マーキングしたンをNにする + 「つ」を「q」にする
	std::map<UnicodeString, UnicodeString> long_map = {
		{"\u0101" , "a:"},
		{"\u0113" , "e:"},
		{"\u012B" , "i:"},
		{"\u014D" , "o:"},
		{"\u016B" , "u:"},
		{"[[n]]"  , "N" },
		{"~"      , "q" }
	};
	for (const auto& x : long_map) {
		input.findAndReplace(x.first, x.second);
	}

	// 変換結果取得
	std::vector<char> romaji( input.length() );
	input.extract(0, input.length(), &romaji[0], "utf8");

	// Julius の voca 形式へ整形
	std::string result(romaji.begin(), romaji.end());
	std::map<std::string, std::string> regex_map = {
		{"[aiueoNq]:?" , "$0 "},
		{"[^aiueoNq]+" , "$0 "},
		{"[^a-zN:@]"   , ""   },
		{" +"          , " "  },
	};
	for (const auto& x : regex_map) {
		boost::regex r(x.first);
		result = boost::regex_replace(result, r, x.second, boost::match_default | boost::format_all);
	}

	return scope.Close( String::New(result.c_str()) );
};

void init(Handle<Object> target) {
	assert( uv_mutex_init(&m) == 0 );
	NODE_SET_METHOD(target, "kana2voca", kana2voca);
	NODE_SET_METHOD(target, "kana2vocaSync", kana2voca_sync);
}

NODE_MODULE(kana2voca, init)
