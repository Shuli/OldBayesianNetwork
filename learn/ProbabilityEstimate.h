//============================================================================
// Name        : ProbabilityEstimate.h
// Version     : 1.0
// Date        : 2010/04/14
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#ifndef PROBABILITYFREQ_H_
#define PROBABILITYFREQ_H_

#include "ProbabilityBase.h"

/*!
 * @brief 頻度を元にした条件付き確率の推定を行い、その正答率を返します
 */
class ProbabilityEstimate: public ProbabilityBase {

public:
	/*!
	 * @brief
	 */
	ProbabilityEstimate();

	/*!
	 * @brief 読み込み対象データファイル名を必須引数とします
	 * @param[in] string 読み込み対象ファイル名(UTF-8のみ)
	 */
	ProbabilityEstimate(string file) {this->file = file;}

	/*!
	 * @brief
	 */
	virtual ~ProbabilityEstimate();

public:
	/*!
	 * @brief 頻度を元にした条件付き確率の推定を行います
	 * @param[in]  double 推定方法重み
	 * @param[in]  string 対象ノード名
	 * @param[in]  string 対象ユーザノード(例：UserID)
	 * @param[in]  string 対象ユーザ値(例：でかぷり夫)
	 * @param[in]  COND*  対象ノードの頻度を求める際の検索条件(ユーザ名を含めない)
	 * @param[in]  long   全データで先頭から対象とする行数
	 * @param[out] PROB*  対象ノードの結果の確率
	 */
	int learn(UD weight, string targetn, string usern, string users, COND *condition, long count, vector<int> *result);

protected:
	/*!
	 * @brief 最尤推定かMAP推定を行います
	 * @param[in] double  最尤推定=0,MAP推定=1.0
	 * @param[in] double  元となる条件付き確率
	 * @param[in] double  子の頻度
	 * @param[in] double  親の頻度
	 * @param[in] double* 求められた推定値
	 */
	int estimate(UD weight, UD cpt, UD child, UD parent, UD *result);

	/*!
	 * @brief 読み込んだ行が指定検索条件を満たすか返します
	 * @param[in]  COND* 検索条件
	 * @param[in]  LINE* データの1行
	 * @param[out] bool* 合致有無
	 */
	int isCondition(COND *condition, LINE *line, bool *result);

	/**
	 * 推定結果が一致したか返します
	 * @param[in]  vector<PSORT> 確率の降順で並べられた推定要素
	 * @param[in]  string        正解要素
	 * @param[out] bool          結果
	 */
	int isMatch(vector<PSORT> *estimate, string answer, bool *result);

	/*!
	 * @brief 同率1位の文言を作成します
	 * @param[in]  vector<PSORT> 確率の降順で並べられた推定要素
	 * @param[out] string        表示用文言
	 */
	int getFirst(vector<PSORT> *estimate, string *result);

};

#endif /* PROBABILITYFREQ_H_ */
