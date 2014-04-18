//============================================================================
// Name        : CompositeK2.h
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#ifndef COMPOSITEK2_H_
#define COMPOSITEK2_H_

#include "BayesianDefine.h"
#include "ProbabilityBase.h"

/*!
 * @brief 基本精度浮動小数点を定めます
 */
#define LD double
#define CEPS __DBL_EPSILON__

/*!
 * @brief K2アルゴリズムを用いて実データからBDMが最も高いネットワーク構造定義ファイルを作成します
 */
class CompositeK2 {
    //friend class CompositeBase;

private:
	/*!
	 * @brief 読み込み実データ
	 */
	ProbabilityBase *base;

private:
	/*!
	 * @brief 初期処理
	 */
	CompositeK2() {}

public:
	/*!
	 * @brief 処理対象実データを必須とします
	 */
	CompositeK2(ProbabilityBase *target) {
		base = target;
	}

public:
	/*!
	 * @brief 指定実データからネットワーク構造を作成し、その定義をファイル出力します
	 */
	int createNodeRelation();

protected:
	/*!
	 * @brief 階乗計算処理
	 */
	inline void reduce(double s, double *r);

	/*!
	 * @brief 既に親子関係が決定されている組(自ノードのみ含む)と親との成績を計算して返します
	 */
	inline double calcBDM(double q, double r, vector<double> *nj, PROBS *nk, COND *calc, double *result);

	/*!
	 * @brief 親子関係をその定義ファイルに書き出します
	 */
	int createNodeDefine(string file, RELATES *relations, vector<string> *titles);

	/*!
	 * @brief 元集合から対象となる親集合を取得します
	 */
	int getParents(string current, CHARS *titles, CHARS *parents);

	/*!
	 * @brief 親集合に関して全てのBDMを算出します
	 */
	int calParentBDM(CHARS *current, string target, double r, CHARS::iterator b, CHARS::iterator e, PROBS *result);

	/*!
	 * @brief 自身のBDMを算出します
	 */
	int calSelfBDM(string current, double *r, double *result);

	/*!
	 * @brief 親決定済みと親候補から親対象を自身の親としてよいかその条件の組を作成します
	 */
	int calPPattern(string current, COND *cond, PROBS *probs, CHARS::iterator bn, CHARS::iterator on, bool f);

};

#endif /* COMPOSITEK2_H_ */
