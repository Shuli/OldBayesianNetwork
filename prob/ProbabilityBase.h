//============================================================================
// Name        : ProbabilityBase.h
// Version     : 1.0
// Date        : 2010/04/14
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#ifndef PROBABILITY_BASE_H_
#define PROBABILITY_BASE_H_

#include "ProbabilityParse.h"

/*!
 * @brief CSVデータ、又はBIFから事前確率と、条件付き確率を求めて保持します
 */
class ProbabilityBase {

protected:
	/*!
	 * @brief デフォルトコンストラクタは公開しません
	 * */
	ProbabilityBase() {}

public:
	/*!
	 * @brief 読み込み対象ファイル名を必須引数とします
	 * @param[in] string 読み込み対象ファイル名
	 */
	ProbabilityBase(string file);

	/*!
	 * @brief 終了時にはファイルを閉じます
	 */
	~ProbabilityBase() { if (this->ifs.is_open()) this->ifs.close(); }

public:


protected:
	/*!
	 * @brief 実データの列数を保持します
	 */
	long cols;

	/*!
	 * @brief 実データの行数を保持します
	 */
	long rows;

	/*!
	 * @brief 一列の一意値データをキャッシュ保持します
	 */
	RELATES cashes;

	/*!
	 * @brief タイトルラベルを保持します
	 */
	CHARS titles;

	/*!
	 * @brief 実データ本体を保持します
	 */
	VALUES vals;

protected:
	/*!
	 * @brief 読み込み対象ファイル名を保持します
	 */
	string file;

	/*!
	 * @brief ファイル参照を保持します
	 */
	ifstream ifs;

	/*!
	 * @brief load処理時に読み込む最大行数を保持します
	 */
	long max;

	/*!
	 * @brief 現在参照中の行番号を保持します
	 */
	long now;

public:
	/*!
	 * @brief 対象CSVファイルを指定行数分、読み込みます
	 * @param[in] 読み込み行数(0以下の場合は全ての行)
	 */
	int load(long max);

	/*!
	 * @brief 対象CSVファイルを全て読み込みます
	 */
	int load();

	/*!
	 * @brief ファイルを開いた状態に戻します
	 */
	int reload();

	/*!
	 * @brief 現在位置から1行単位(keyは列タイトル)で情報を提供します
	 * @param[out] map<string, string>* 読み込んだデータの格納領域
	 */
	int read(map<string, string> *line);

	/*!
	 * @brief CSV形式の行データを末尾に追加します
	 * @param[in] LINE 行データ
	 */
	int add(LINE *row);

	/*!
	 * @brief 指定条件を満たす指定要素の全件数を返します
	 * @param[in]  string 					         対象要素名
	 * @param[in]  vector<pair<string, string>> 検索条件
	 * @param[out] map<string, double>          対象要素の条件付き確率
	 * @param[out] long*                        全件数(NULLの場合は確率を返します)
	 */
	int prob(string variable, COND *condition, PROBS *result, long *total);

	/*!
	 * @brief 指定条件を満たす指定要素の全件数を返します
	 * @param[in]  string 					         対象要素名
	 * @param[in]  vector<pair<string, string>> 検索条件
	 * @param[out] map<string, double>          対象要素の条件付き確率
	 * @param[out] long*                        全件数(NULLの場合は確率を返します)
	 * @param[in]  bool                         対象件数が0件の場合、Freq(一様分布)を与えるか否か
	 */
	int prob(string variable, COND *condition, PROBS *result, long *total, bool freq);

	/*!
	 * @brief 指定要素の全てのデータの件数を返します
	 * @param[in]  string 					         対象要素名
	 * @param[out] map<string, double>          対象要素の条件付き確率
	 * @param[out] long*                        全件数(NULLの場合は確率を返します)
	 */
	int prob(string variable, PROBS *result, long *total);

	/*!
	 * @brief 指定要素の全てのデータの件数を返します
	 * @param[in]  string 					         対象要素名
	 * @param[out] map<string, double>          対象要素の条件付き確率
	 * @param[out] long*                        全件数(NULLの場合は確率を返します)
	 */
	int prob(string variable, PROBS *result, long *total, bool num);

	/*!
	 * @brief 速度向上用に一つ前の対象列の個数マップを保持します
	 */
	int cash(string variable, CHARS *result);

	/*!
	 * @brief 現在参照している行番号を返します
	 */
	long nowcnt() { return this->now; }

	/*!
	 * @brief 指定ファイルの全体行数を返します
	 */
	long rowcnt() { return this->rows; }

	/*!
	 * @brief タイトル集合を返します
	 */
	CHARS *cnames() { return &titles; }

protected:
	/*!
	 * @brief 指定列データから一意な値を作成します
	 * @param[in]  string 		     対象要素名
	 * @param[out] vector<string> 指定要素名の一意な名前
	 */
	int uniq(string variable, CHARS *element);

	/*!
	 * @brief 指定要素の全てのデータの件数を返します
	 * @param[in]  string 					         対象要素名
	 * @param[out] map<string, double>          対象要素の条件付き確率
	 * @param[out] long*                        全件数(NULLの場合は確率を返します)
	 */
	int probConcrete(string variable, PROBS *result, long *total, bool num);

};

#endif /* PROBABILITY_BASE_H_ */
