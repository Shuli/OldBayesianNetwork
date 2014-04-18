//============================================================================
// Name        : ProbabilityEstimate.cpp
// Version     : 1.0
// Date        : 2010/04/14
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "ProbabilityEstimate.h"

/*!
 * @brief 初期処理を定義します
 */
ProbabilityEstimate::ProbabilityEstimate() {
}

/*!
 * @brief 終了処理を定義します
 */
ProbabilityEstimate::~ProbabilityEstimate() {
}

/**
 * 推定結果が一致したか返します
 * @param[in]  vector<PSORT> 確率の降順で並べられた推定要素
 * @param[in]  string        正解要素
 * @param[out] bool          結果
 */
int ProbabilityEstimate::isMatch(vector<PSORT> *estimate, string answer, bool *result) {
	// 5位までの正解を正答としてカウントします
	for (unsigned int i = 0, row = 0; i < estimate->size(); i++) {
		if (row < 5) {
			if ((*estimate)[i].second == answer) {
				*result = true;
				return 0;
			}
		} else {
			break;
		}
		row++;
	}
	*result = false;
	// 以下は、1位のみを正答とした場合の処理です
#ifdef ONLY_FIRST
	// 最大確率を求めます
	double max = -1;
	for (unsigned int i = 0; i < estimate->size(); i++) {
		if ((*estimate)[i].first >= max) {
			max = (*estimate)[i].first;
		} else {
			break;
		}
	}
	// 最大確率を持つ同率一位を探します
	*result = false;
	for (unsigned int i = 0; i < estimate->size(); i++) {
		if ((*estimate)[i].first == max) {
			if ((*estimate)[i].second == answer) {
				*result = true;
			}
		} else {
			break;
		}
	}
#endif
	return 0;
}

/*!
 * @brief 同率1位の文言を作成します
 * @param[in]  vector<PSORT> 確率の降順で並べられた推定要素
 * @param[out] string        表示用文言
 */
int ProbabilityEstimate::getFirst(vector<PSORT> *estimate, string *result) {
	// 最大確率を求めます
	double max = -1;
	for (unsigned int i = 0; i < estimate->size(); i++) {
		if ((*estimate)[i].first >= max) {
			max = (*estimate)[i].first;
		} else {
			break;
		}
	}
	// 最大確率を持つ同率一位を探します
	*result = "";
	for (unsigned int i = 0; i < estimate->size(); i++) {
		if ((*estimate)[i].first == max) {
			*result += (result->length() > 0 ? "," : "") + (*estimate)[i].second;
		} else {
			break;
		}
	}
	*result = "{" + *result + "}";
	return 0;
}

/*!
 * @brief 読み込んだ行が指定検索条件を満たすか返します
 * @param[in]  COND* 検索条件
 * @param[in]  LINE* データの1行
 * @param[out] bool* 合致有無
 */
int ProbabilityEstimate::isCondition(COND *condition, LINE *line, bool *result) {
	for (COND::iterator iter = condition->begin(); iter != condition->end(); iter++) {
		LINE::iterator ifound = line->find(iter->first);
		if (ifound == line->end()) {
			printf("Not found the find condition on csv data(%s)\n", iter->first.c_str());
			return 1;
		}
		// 検索条件に合致しません
		if (iter->second != ifound->second) {
			*result = false;
			return 0;
		}
	}
	// 全ての検索条件に合致しました
	*result = true;
	return 0;
}

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
int ProbabilityEstimate::learn(UD weight, string targetn, string usern, string users, COND *condition, long rowcount, vector<int> *result) {
	// データを指定行数読み込みます
	reload();
	load(rowcount);

	// 検索条件に個人ユーザを含めます
	COND condp(*condition);
	condp.push_back(COND_PAIR(usern, users));

	// データの最後まで読み込み、推定値との比較を行います
	LINE line;
	vector<PSORT> sorting;
	while (read(&line) == 0) {
		// 指定条件に合致しない行の場合は、推定処理を行いません
		bool match = false;
		isCondition(&condp, &line, &match);
		if (match == true) {
			// 個人CPT(頻度=件数)を求めます
			// 個人とは指定ユーザの情報のみを対象とした場合の指定ノードの頻度を指します
			PROBS targetp; long parentp;
			this->prob(targetn, &condp, &targetp, &parentp);
			// 内容確認用の表示を行います
			if (targetp.begin() != targetp.end()) cout << "[cpt-personal]";
			for (PROBS::iterator iter = targetp.begin(); iter != targetp.end(); iter++) {
				cout << (iter != targetp.begin() ? "," : "") << iter->second << "/" << parentp << "=" << iter->first;
			}
			if (targetp.begin() != targetp.end()) cout << endl;

			// 全体CPT(頻度=件数)を求めます
			// 全体とは全ユーザを対象とした場合の指定ノードの頻度を指します
			PROBS targeta; long parenta;
			this->prob(targetn, condition, &targeta, &parenta);
			if (targeta.begin() != targeta.end()) cout << "[cpt-all-users]";
			for (PROBS::iterator iter = targeta.begin(); iter != targeta.end(); iter++) {
				cout << (iter != targeta.begin() ? "," : "") << iter->second << "/" << parenta << "=" << iter->first;
			}
			if (targeta.begin() != targeta.end()) cout << endl;

			// 全体と個人で該当する要素名の和集合を作成します
			vector<string> names;
			// 全体
			for (PROBS::iterator iter = targeta.begin(); iter != targeta.end(); iter++) {
				vector<string>::iterator ifilt = find(names.begin(), names.end(), iter->first);
				if (ifilt == names.end()) names.push_back(iter->first);
			}
			// 個人
			for (PROBS::iterator iter = targetp.begin(); iter != targetp.end(); iter++) {
				vector<string>::iterator ifilt = find(names.begin(), names.end(), iter->first);
				if (ifilt == names.end()) names.push_back(iter->first);
			}

			// 現時点での推定値を求めます
			PROBS resultp;
			for (vector<string>::iterator iter = names.begin(); iter != names.end(); iter++) {
				// 全体：該当要素の値を取得します
				UD childa = 0.0;
				PROBS::iterator ifound1 = targeta.find(*iter);
				if (ifound1 != targeta.end()) childa = ifound1->second;
				// 個人：該当要素の値を取得します
				UD childp = 0.0;
				PROBS::iterator ifound2 = targetp.find(*iter);
				if (ifound2 != targetp.end()) childp = ifound2->second;

				// 全体CPTを求めます
				UD cpta = (parenta == 0.0 ? 0.0 : childa / parenta);
				// 推定値を求めます
				UD estimaten = 0.0;
				if (parentp > 0.0) estimate(weight, cpta, childp, parentp, &estimaten);
				// 値を保持します
				resultp.insert(pair<string, UD>(*iter, estimaten));
			}
			// 結果(TOP5)を表示します
			sorting.clear();
			for (PROBS::iterator iter = resultp.begin(); iter != resultp.end(); iter++) {
				sorting.push_back(PSORT(iter->second, iter->first));
			}
			sort(sorting.begin(), sorting.end(), DSORT());
			for (unsigned int i = 0, row = 0; i < sorting.size(); i++) {
				if (row < 5) {
					printf("[Estimate]%f=P(%s=%s)\n", sorting[i].first, targetn.c_str(), sorting[i].second.c_str());
				} else {
					break;
				}
				row++;
			}

			// lineが0ではない(一度でも推定した)場合は、正答を求めます
			LINE::iterator iter = line.find(targetn);
			if (iter != line.end()) {
				// 正答数を記録します
				bool right; string printp;
				// 同率1位のマッチングを判定します
				isMatch(&sorting, iter->second, &right);
				int answer = (right == true ? 1 : 0);
				result->push_back(answer);
				getFirst(&sorting, &printp);
				cout << "[Judgement]" << (answer == 1 ? "OK" : "NG") << "=A(estimate=" << printp << ",answer=" << iter->second << ")" << endl;
			}
		}
		// 読み込んだ行を追加します
		add(&line);
	}
	return 0;
}

/*!
 * @brief 最尤推定かMAP推定を行います
 * @param[in] double  最尤推定=0,MAP推定=1.0
 * @param[in] double  元となる条件付き確率
 * @param[in] double  子の頻度
 * @param[in] double  親の頻度
 * @param[in] double* 求められた推定値
 */
int ProbabilityEstimate::estimate(UD weight, UD cpt, UD child, UD parent, UD *result) {
	UD c = (weight * cpt) + (parent * (child / parent));
	UD p = weight + parent;
	*result = c / p;
	return 0;
}

