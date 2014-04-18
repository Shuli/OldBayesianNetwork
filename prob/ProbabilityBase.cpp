//============================================================================
// Name        : ProbabilityBase.cpp
// Version     : 1.0
// Date        : 2010/04/14
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "ProbabilityBase.h"
#include "ProbabilityParse.h"

/*!
 * @brief 読み込み対象ファイル名を必須引数とします
 * @param[in] string 読み込み対象ファイル名
 */
ProbabilityBase::ProbabilityBase(string target) {
	this->file = target;
	this->now = 0;
	this->cols = -1;
	this->rows = -1;
	titles.clear();
	vals.clear();
}

/*!
 * @brief CSV形式の行データを末尾に追加します
 * @param[in] LINE 行データ
 */
int ProbabilityBase::add(LINE *row) {
	for (LINE::iterator iter = row->begin(); iter != row->end(); iter++) {
		VALUES::iterator target = vals.find(iter->first);
		target->second->push_back(iter->second);
	}
	return 0;
}

/*!
 * @brief 対象CSVファイルを「全て」読み込みます
 */
int ProbabilityBase::load() {
	return load(-1);
}

/*!
 * @brief ファイル参照をやり直して先頭に戻します
 */
int ProbabilityBase::reload() {
	if (ifs.is_open()) ifs.close();
	ifs.open(this->file.c_str(), ios::in);
	titles.clear();
	vals.clear();
	cashes.clear();
	return 0;
}

/*!
 * @brief 対象CSVファイルを指定行数まで(含む)読み込みます
 * @param[in] string 対象ファイル名
 */
int ProbabilityBase::load(long max)
{
	this->max = max;
	this->now = 0;
	titles.clear();
	try {
		if (!ifs.is_open()) ifs.open(file.c_str(), ios::in);
		ProbabilityParse parse(ifs);
		// タイトルを読み取ります
		while (!parse.isBreak()) {
			string title;
			parse >> title;
			titles.push_back(title);
		}
		parse >> endl;

		// 表全体を構成します
		int colsize = titles.size();
		CHARS *cols[colsize];
		for (int i = 0; i < colsize; i++) {
			cols[i] = new CHARS();
			vals.insert(VALUES_PAIR(titles[i], cols[i]));
		}
		// 実データを取得します
		int rowsize = 0;
		while (!parse.isEof()) {
			int target = 0;
			while (!parse.isBreak()) {
				string value;
				parse >> value;
				// 各列に行を追加します
				cols[target]->push_back(value);
				target++;
			}
			parse >> endl;
			rowsize++;
			// 最大件数が指定されている場合、そこまで読み込みます
			if (max > 0 && rowsize >= max) break;
		}
		// 件数を記録します
		this->cols = colsize;
		this->rows = rowsize;
		this->now  = rowsize;
		// 最後まで読み込んだ場合は、ファイル参照を破棄します
		if (parse.isEof()) ifs.close();

	} catch (...) {
		printf("too many file size(%s)\n", file.c_str());
		return 1;
	}
	return 0;
}

/*!
 * @brief 現在位置から1行単位(keyは列タイトル)で情報を提供します
 * @param[out] map<string, string>* 読み込んだデータの格納領域
 */
int ProbabilityBase::read(LINE *line) {
	// 既に読み終えている場合は、エラーとします
	if (!ifs.is_open()) {
		printf("already file closed(%s)\n", file.c_str());
		return 1;
	}
	// 行を続きから読み込みます
	ProbabilityParse parse(ifs);
	// 戻り値保持領域を作成します
	line->clear();
	cashes.clear();
	// 実データを取得します
	CHARS::iterator iter = titles.begin();
	while (!parse.isBreak()) {
		string value;
		parse >> value;
		// 各列に行を追加します
		line->insert(pair<string, string>(*iter, value));
		iter++;
	}
	parse >> endl;
	// 終端判定を行います
	if (parse.isEof()) ifs.close();
	now++;
	return 0;
}

/*!
 * @brief 指定条件を満たす指定要素の全件数を返します
 * @param[in]  string 					         対象要素名
 * @param[out] map<string, double>          対象要素の件数
 * @param[out] long                         検索条件に合致する件数
 */
int ProbabilityBase::prob(string variable, PROBS *result, long *total) {
	return probConcrete(variable, result, total, false);
}

/*!
 * @brief 指定条件を満たす指定要素の全件数を返します
 * @param[in]  string 					         対象要素名
 * @param[out] map<string, double>          対象要素の件数
 * @param[out] long                         検索条件に合致する件数
 */
int ProbabilityBase::prob(string variable, PROBS *result, long *total, bool num) {
	return probConcrete(variable, result, total, num);
}

/*!
 * @brief 指定条件を満たす指定要素の全件数を返します
 * @param[in]  string 					         対象要素名
 * @param[in]  vector<pair<string, string>> 検索条件
 * @param[out] map<string, double>          対象要素の件数
 * @param[out] long                         検索条件に合致する件数
 * @param[in]  bool                         対象件数が0件の場合、Freq(一様分布)を与えるか否か
 */
int ProbabilityBase::prob(string variable, COND *condition, PROBS *result, long *total) {
	return prob(variable, condition, result, total, true);
}

/*!
 * @brief 指定条件を満たす指定要素の全件数を返します
 * @param[in]  string 					         対象要素名
 * @param[in]  vector<pair<string, string>> 検索条件
 * @param[out] map<string, double>          対象要素の件数
 * @param[out] long                         検索条件に合致する件数
 * @param[in]  bool                         対象件数が0件の場合、Freq(一様分布)を与えるか否か
 */
int ProbabilityBase::prob(string variable, COND *condition, PROBS *result, long *total, bool freq) {
	long count = 0;
	vector<long> indexs;
	for (COND::iterator icond = condition->begin(); icond != condition->end(); icond++) {
		// 対象列を特定します
		VALUES::iterator icol = vals.find(icond->first);
		if (icol == vals.end()) {
			cout << "[ProbabilityBase::cnt]not found key for csv(" << icond->first << ")" << endl;
			return 1;
		}
		// 対象行に指定条件が存在するか精査します(全精査する前に)
		CHARS *rows = icol->second;
		CHARS::iterator ifilt = find(rows->begin(), rows->end(), icond->second);
		if (ifilt == rows->end()) {
			// 該当なしを返します
			cout << "[ProbabilityBase::cnt]not found value for csv(" << icond->second << ")" << endl;
			return 2;
		}
		// 対象行を特定します
		if (count == 0) {
			// 初回は全行精査と検索対象値(配列番号)の作成を行います
			for (unsigned long row = 0; row < rows->size(); row++) {
				if ((*rows)[row] == icond->second) {
					indexs.push_back(row);
				}
			}
		} else {
			// 初回以外は対象行を絞り込みます
			for (vector<long>::iterator irow = indexs.begin(); irow != indexs.end(); irow++) {
				if ((*rows)[*irow] != icond->second) {
					// 対象から除外します
					indexs.erase(irow);
					irow--;
				}
			}
		}
		count++;
	}

	// 条件に合致する要素の確率、要素/条件合致数を求めます
	// 条件に合致する件数を返します
	*total = indexs.size();
	// 対象列を特定します
	VALUES::iterator inode = vals.find(variable);
	if (inode == vals.end()) {
		cout << "[ProbabilityBase::cnt]not found key for csv(" << variable << ")" << endl;
		return 3;
	}
	CHARS *rows = inode->second;
	// 件数を記録します
	result->clear();
	// 一意な要素名を取得します
	CHARS elements;
	uniq(variable, &elements); // 対象行のUniqでは0件要素が求められない為、全件に変更(処理が遅くなります)
	for (CHARS::iterator ielement = elements.begin(); ielement != elements.end(); ielement++) {
		double child = 0;
		// 条件ありの場合は、取得した行番号の合致精査により件数を特定します
		for (vector<long>::iterator iindex = indexs.begin(); iindex != indexs.end(); iindex++) {
			if ((*rows)[*iindex] == *ielement) {
				child++;
			}
		}
		// 件数を求められている場合は、子の件数を保持します
		result->insert(PROBS_PAIR(*ielement, child));
	}

	// もし、状態の総数が0の場合、一様分布を与えます(全て1件を設定し、合計数をその合計とします)
	if (freq) {
		double sum = 0;
		for (PROBS::iterator iter = result->begin(); iter != result->end(); iter++) {
			sum += iter->second;
		}
		if (sum <= 0) {
			// 0以下の場合、一様分布を与えます
			*total = 0;
			for (PROBS::iterator iter = result->begin(); iter != result->end(); iter++) {
				iter->second = 1;
				(*total)++;
			}
		}
	}

	return 0;
}

/*!
 * @brief 指定条件を満たす指定要素の全件数を返します
 * @param[in]  string 					         対象要素名
 * @param[out] map<string, double>          対象要素の件数
 * @param[out] long                         検索条件に合致する件数
 */
int ProbabilityBase::probConcrete(string variable, PROBS *result, long *total, bool num)
{
	// 条件に合致する要素の確率、要素/条件合致数を求めます
	// 条件なしの場合は全件数を使用します
	*total = this->rows;
	// 件数だけの場合はこの時点で処理を中断します
	if (num) return 0;
	// 対象列を特定します
	VALUES::iterator inode = vals.find(variable);
	if (inode == vals.end()) {
		cout << "[ProbabilityBase::cnt]not found key for csv(" << variable << ")" << endl;
		return 3;
	}
	CHARS *rows = inode->second;
	// 件数を記録します
	result->clear();
	// 一意な要素名を取得します
	CHARS elements;
	uniq(variable, &elements);
	for (CHARS::iterator ielement = elements.begin(); ielement != elements.end(); ielement++) {
		double child = 0;
		// 条件なしの場合は、指定要素に合致する件数を全件からカウントします
		for (long r = 0; r < this->rows; r++) {
			if ((*rows)[r] == *ielement) {
				child++;
			}
		}
		// 件数を求められている場合は、子の件数を保持します
		result->insert(PROBS_PAIR(*ielement, child));
	}
	// 条件に合致する件数を返します
	return 0;
}

/*!
 * @brief 指定列データから一意な値を作成します
 * @param[in]  string 		     対象要素名
 * @param[out] vector<string> 指定要素名の一意な名前
 */
int ProbabilityBase::uniq(string variable, CHARS *element) {
	// キャッシュを使います
	RELATES::iterator iterc = cashes.find(variable);
	if (iterc != cashes.end()) {
		CHARS *elementc = iterc->second;
		element->assign(elementc->begin(), elementc->end());
		return 0;
	}
	// 対象列を取得します
	VALUES::iterator irow = vals.find(variable);
	if (irow == vals.end()) {
		cout << "[ProbabilityBase::uniq]not found unique value(" << variable << ")" << endl;
		return 2;
	}
	CHARS *rows = irow->second;
	// 検索に合致した行番号の要素名を取得、その中から一意な名前を獲得します
	for (CHARS::iterator iline = rows->begin(); iline != rows->end(); iline++) {
		// 該当列の実データを取得します
		string value = *iline;
		// 新規要素名の場合は追加します
		CHARS::iterator ifind = find(element->begin(), element->end(), value);
		if (ifind == element->end()) {
			element->push_back(value);
		}
	}
	// キャッシュに保持します
	CHARS *elementn = new CHARS(*element);
	cashes.insert(RELATES_PAIR(variable, elementn));
	return 0;
}
