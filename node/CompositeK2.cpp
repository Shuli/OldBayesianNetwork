//============================================================================
// Name        : CompositeK2.cpp
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "CompositeK2.h"

/*!
 * 指定実データからネットワーク構造を作成し、その定義をファイル出力します
 */
int CompositeK2::createNodeRelation() {
#ifdef VERBOSE
	// ログ出力
	LINE("=");
	cout << "[CompositeK2::createNodeRelation]K2 Algorithm Start" << endl;
	LINE("=");
#endif
	map< string, vector<string>* > relations; // K2アルゴリズムで作成される親子関係(Map-Key=親,子集合)
	CHARS titles(*base->cnames()); 			  // ノード名集合
	// 自身のノード(K2でいうi)を走査します
	for (CHARS::iterator iter1 = titles.begin(); iter1 != titles.end(); iter1++) {
		// K2に必要な変数を用意します
		double r = 0.0, p = 0.0;
#ifdef VERBOSE
		// 自身のBDMを求めます
		LINE("-");
		cout << "[CompositeK2::createNodeRelation]Target Node <- " << *iter1 << endl;
#endif
		if (calSelfBDM(*iter1, &r, &p) != 0) {
			cout << "[CompositeK2::createNodeRelation]Function of calSelfBDM Failure" << endl;
			return 2;
		}
#ifdef VERBOSE
		LINE("-");
		cout << "[CompositeK2::createNodeRelation]Pa(" << *iter1 << "|φ)=" << p << endl;
		LINE("-");
#endif

		// 処理対象を作成します
		PROBS pn; CHARS parent, targets;
		parent.push_back(*iter1); // 処理対象を代入します
		if (getParents(*iter1, &titles, &targets) != 0) {
			cout << "[CompositeK2::createNodeRelation]Function of getParents Failure" << endl;
			return 3;
		}
		CHARS::iterator iter2 = (targets.end() - 1);
#ifdef VERBOSE
		for (CHARS::iterator iterpnt = targets.begin(); iterpnt != targets.end(); iterpnt++) {
			cout << "[CompositeK2::createNodeRelation]Target Parents <- " << *iterpnt << endl;
		}
#endif

		// K2アルゴリズム(forward型)を実行します
		bool processing = true;
		while (processing) {
			// 全親候補のBDMを求めます
			if (calParentBDM(&parent, *iter1, r, iter2, targets.begin(), &pn) != 0) {
				cout << "[CompositeK2::createNodeRelation]Function of calParentBDM Failure" << endl;
				return 1;
			}
#ifdef VERBOSE
			if (pn.begin() != pn.end()) {
				LINE("-");
				for (PROBS::iterator iterp = pn.begin(); iterp != pn.end(); iterp++) {
					cout << "[CompositeK2::createNodeRelation]Pa(" << *iter1 << "|" << iterp->first << ")=" << iterp->second << endl;
				}
				LINE("-");
			}
#endif
			// 親のBDMで最も大きいBDMをみつけます
			double max = 0.0; string target;
			for (PROBS::iterator iterp = pn.begin(); iterp != pn.end(); iterp++) {
				if (max < iterp->second && p < iterp->second) {
					max = iterp->second;
					target = iterp->first;
				}
			}
			if (max > p) {
				// もしそれが独立を上回る場合、それを親と確定します
				parent.insert(parent.begin(), target);
#ifdef VERBOSE
				cout << "[CompositeK2::createNodeRelation]Add Condition <- " << target << endl;
				for (CHARS::iterator iterz = parent.begin(); iterz != parent.end(); iterz++) {
					cout << "[CompositeK2::createNodeRelation]Condition <- " << *iterz << endl;
				}
#endif
				string logp;
				for (CHARS::iterator itero = parent.begin(); itero < (parent.end() - 1); itero++) {
					logp += ((itero != parent.begin() ? string(",") : string("")) + *itero);
				}
#ifdef VERBOSE
				LINE("=");
#endif
				cout << "[CompositeK2::createNodeRelation]Commit Pa(" << *iter1 << "|{" << logp << "})" << endl;
				// 親子関係を保持します
				for (CHARS::iterator itero = parent.begin(); itero < (parent.end() - 1); itero++) {
					vector<string> *children;
					map<string, vector<string>* >::iterator iterr = relations.find(*itero);
					if (iterr == relations.end()) {
						// 親をキーとして新規追加します
						children = new vector<string>();
						relations.insert(pair<string, vector<string>* >(*itero, children));
						cout << "[CompositeK2::createNodeRelation]New Parent <- " << *itero << endl;
					} else {
						// 既存子集合を利用します
						children = iterr->second;
						cout << "[CompositeK2::createNodeRelation]Always Parent <- " << *itero << endl;
					}
					// 既に対象の子がいる場合は追加しません
					vector<string>::iterator iterw = find(children->begin(), children->end(), *iter1);
					if (iterw == children->end()) {
						children->push_back(*iter1);
						cout << "[CompositeK2::createNodeRelation]Add Child <- " << *iter1 << endl;
					} else {
						cout << "[CompositeK2::createNodeRelation]Always Child <- " << *iter1 << endl;
					}
				}

#ifdef VERBOSE
				LINE("=");
#endif

				// 親が確定した為、親集合からそれを取り除きます
				// A,B,D,CでPa(D|{B})となり、Pa(D|{B,C})の探索もありと解釈しています
				// 全ての親が完了した場合は処理を終了します
				CHARS::iterator found = find(targets.begin(), targets.end(), target);
				if (found == targets.end()) {
#ifdef VERBOSE
					cout << "[CompositeK2::createNodeRelation]Not Found Target <- " << target << endl;
#endif
					processing = false;
					break;
				}
				targets.erase(found);
				if (targets.begin() == targets.end()) {
#ifdef VERBOSE
					cout << "[CompositeK2::createNodeRelation]Not Found Parent" << endl;
#endif
					processing = false;
					break;
				}
				// 次の親に処理を行います
#ifdef VERBOSE
				for (CHARS::iterator iterpnt = targets.begin(); iterpnt != targets.end(); iterpnt++) {
					cout << "[CompositeK2::createNodeRelation]Target Parents <- " << *iterpnt << endl;
				}
#endif
				iter2 = (targets.end() - 1);
				p = max;
#ifdef VERBOSE
				cout << "[CompositeK2::createNodeRelation]Next Target Parent <- " << *iter2 << endl;
#endif
			} else {
				// 下回る場合は候補なしとして処理を終了します
				processing = false;
			}
		}
	}
	// 作成した親子関係をファイルに書き出します
	if (createNodeDefine(RELATION_FILE, &relations, &titles) != 0) {
		cout << "[CompositeK2::createNodeRelation]Function of createNodeDefine Failure" << endl;
		return 4;
	}
	return 0;
}

/*!
 * @brief 親子関係をその定義ファイルに書き出します
 */
int CompositeK2::createNodeDefine(string file, RELATES *relations, CHARS *titles) {
	try {
		ofstream ofs(file.c_str(), ios::out);
		// タイトル行を書き込みます
		for (CHARS::iterator iter = titles->begin(); iter != titles->end(); iter++) {
			ofs << "," << *iter;
		}
		ofs << endl;
		// 親子関係を書き込みます
		for (CHARS::iterator iter1 = titles->begin(); iter1 != titles->end(); iter1++) {
			// 見出し列を書き出します
			ofs << *iter1;
			// 親がない場合は、全て0を書き出します
			RELATES::iterator result = relations->find(*iter1);
			if (result == relations->end()) {
				for (CHARS::iterator iter2 = titles->begin(); iter2 != titles->end(); iter2++) {
					ofs << ",0";
				}
				ofs << endl;
			} else {
				// 親子関係を書き出します
				CHARS *line = result->second;
				for (CHARS::iterator iter2 = titles->begin(); iter2 != titles->end(); iter2++) {
					CHARS::iterator result = find(line->begin(), line->end(), *iter2);
					if (result == line->end()) ofs << ",0";
					else ofs << ",1";
				}
				ofs << endl;
			}
		}
		ofs.close();

	} catch (...) {
		cout << "[CompositeK2::createNodeDefine]Nodes.csv Create Failure" << endl;
		return 1;
	}

	return 0;
}

/*!
 * @brief 元集合から対象となる親集合を取得します
 */
int CompositeK2::getParents(string current, CHARS *titles, CHARS *parents) {
	// 現在位置から先頭(配列番号が若い)は全て親候補です
	parents->clear();
	for (CHARS::iterator iter = titles->begin(); iter != titles->end(); iter++) {
		if (*iter == current) break;
		parents->push_back(*iter);
	}
	return 0;
}

/*!
 * @brief 親集合に関して全てのBDMを算出します
 */
int CompositeK2::calParentBDM(CHARS *current, string target, double r, CHARS::iterator iter1, CHARS::iterator itere, PROBS *nums) {
#ifdef VERBOSE
	// ログ出力
	LINE("-");
	cout << "[CompositeK2::calParentBDM]Parent Processing, Target Node <- " << target << endl;
#endif

	// ノードの全ての親候補ノードを走査します
	nums->clear();
	for (CHARS::iterator iter2 = iter1; iter2 > itere - 1; iter2--) {
		// P(・|parents), 親を表す集合を定義します
		CHARS parents(*current);
		parents.insert(parents.begin(), *iter2);
#ifdef VERBOSE
		// ログ出力
		for (CHARS::iterator iterp = parents.begin(); iterp != parents.end(); iterp++) {
			cout << "[CompositeK2::calParentBDM]Condition <- " << *iterp << endl;
		}
#endif
		// 自ノードと親ノードの組からその個数を取得します
		COND condition; PROBS result;
		if (calPPattern(*iter1, &condition, &result, parents.begin(), parents.end(), true) != 0) {
			cout << "[CompositeK2::calParentBDM]Function of calPPattern Failure" << endl;
			return 2;
		}
		// 自ノードの状態数と親ノードの状態数を取得します
		double t = result.size();
#ifdef VERBOSE
		cout << "[CompositeK2::calParentBDM]Result Number <- " << t << endl;
#endif
		double q = t / r; // 親ノードの状態数(ノードの組数)
#ifdef VERBOSE
		cout << "[CompositeK2::calParentBDM]Parent Number <- " << q << endl;
#endif
		// Nの小計を求めます(qとkを元に小計を求めます)
		PROBS::iterator itert = result.begin();
		vector<double> nj;
		for (double j = 0; j < q; j++) {
			double subtotal = 0.0;
			for (double k = 0; k < r; k++, itert++) subtotal += itert->second;
			// n_jの値を保持します
			nj.push_back(subtotal);
#ifdef VERBOSE
			cout << "[CompositeK2::calParentBDM]Subtotal(Condition), Nj(" << (j + 1) << ") <- " << subtotal << endl;
#endif
		}
		// BDMを求めます
		COND calc; double p;
		if (calcBDM(q, r, &nj, &result, &calc, &p) != 0) {
			cout << "[CompositeK2::calParentBDM]Function of calcBDM Failure" << endl;
			return 1;
		}
#ifdef VERBOSE
		for (COND::iterator iterc = calc.begin(); iterc != calc.end(); iterc++) {
			cout << "[CompositeK2::calParentBDM]" << iterc->first << "=" << iterc->second << endl;
		}
#endif
		// 値を返します
		nums->insert(pair<string, double>(*iter2, p));
	}
	return 0;
}

/*!
 * @brief 自身のBDMを算出します
 */
int CompositeK2::calSelfBDM(string current, double *r, double *result) {
#ifdef VERBOSE
	LINE("-");
	cout << "[CompositeK2::calSelfBDM]Myself Processing, Target Node <- " << current << endl;
#endif
	// 自ノードの状態数を取得します
	PROBS temp; long total = 0;
	base->prob(current, &temp, &total);
	// 自ノードの状態数を保持します / 親ノードの状態数は自ノード独立の為、1固定です
	*r = temp.size();
#ifdef VERBOSE
	for (PROBS::iterator iter3 = temp.begin(); iter3 != temp.end(); iter3++) {
		cout << "[CompositeK2::calSelfBDM]Result <- (" << current << "=" << iter3->first << ")=" << iter3->second << endl;
	}
	cout << "[CompositeK2::calSelfBDM]Self Number <- " << *r << endl;
#endif
	// Nを算出します(シンプルに状態数の小計を求めます)
	PROBS::iterator itert = temp.begin();
	vector<double> nj;
	double subtotal = 0.0;
	for (unsigned int j = 0; j < temp.size(); j++) {
		subtotal += itert->second;
#ifdef VERBOSE
		cout << "[CompositeK2::calSelfBDM]Subtotal(Self), Nk(" << (j + 1) << ") <- " << itert->second << endl;
#endif
		itert++;
	}
	// n_iの値を保持します
	nj.push_back(subtotal);
#ifdef VERBOSE
	cout << "[CompositeK2::calSelfBDM]Subtotal(Self), Nj(1) <- " << subtotal << endl;
#endif
	// BDMを求めます
	COND calc; double p;
	if (calcBDM(1, *r, &nj, &temp, &calc, &p) != 0) {
		cout << "[CompositeK2::calSelfBDM]Function of calcBDM Failure" << endl;
		return 1;
	}
#ifdef VERBOSE
	for (COND::iterator iterc = calc.begin(); iterc != calc.end(); iterc++) {
		cout << "[CompositeK2::calSelfBDM]" << iterc->first << "=" << iterc->second << endl;
	}
#endif
	*result = p;
	return 0;
}

/*!
 * 既に親子関係が決定されている組(自ノードのみ含む)と親との成績を計算して返します
 */
inline double CompositeK2::calcBDM(double q, double r, vector<double> *nj, PROBS *nk, COND *calc, double *result) {
	*result = -1.0;
	calc->clear();
	PROBS::iterator iterk = nk->begin();
	vector<double>::iterator iterj = nj->begin();

	for (double j = 0; j < q; j++, iterj++) {
		double subtotal1 = -1.0, subtotal2 = 0.0;
		// π_j {(r - 1) / (Nk + r - 1)}を求めます
		double temp1 = 0.0, temp2 = 0.0;
		reduce((r - 1), &temp1);
		reduce((*iterj + r - 1), &temp2);

#ifdef VERBOSE
		// ログ出力用
		cout << "[CompositeK2::calcBDM]" << (j != 0 ? "  * " : "P = ") << "(" << r << " - 1)! / (" << *iterj << " + " << r << " - 1)! * ";
		stringstream rs1, rs2, rs01, rs02;
		rs1 << r, rs2 << *iterj, rs01 << temp1, rs02 << temp2;
		calc->push_back(COND_PAIR( string("((") + rs1.str() + string(" - 1)!)"), rs01.str() ));
		calc->push_back(COND_PAIR( string("((") + rs2.str() + string(" + ") + rs1.str() +  string(" - 1)!)"), rs02.str() ));
#endif

		// π_k {Nk!}を求めます
#ifdef VERBOSE
		cout << "(";
#endif
		for (double k = 0; k < r; k++, iterk++) {
			// Nk!を求めます
			double temp = 0.0; // 0初期化は必ず行う事
			reduce(iterk->second, &temp);
			if (temp == 0.0) temp = 1.0; // 0! -> 1
			subtotal1 = (subtotal1 == -1.0 ? temp : subtotal1 * temp);
#ifdef VERBOSE
			// ログ出力用
			cout << (k != 0 ? " * " : "") << iterk->second << "!";
			stringstream rs3, rs03; rs3 << iterk->second, rs03 << temp;
			calc->push_back(COND_PAIR( string("(") + rs3.str() + string("!)"), rs03.str() ));
#endif
		}
#ifdef VERBOSE
		cout << ")" << endl;
#endif
		// それらの積算を求めます
		subtotal2 = temp1 / temp2 * subtotal1;
#ifdef VERBOSE
		// ログ出力用
		stringstream ts1, ts2; ts1 << subtotal1, ts2 << subtotal2;
		calc->push_back(COND_PAIR( string("Sub Result1"), rs01.str() ));
		calc->push_back(COND_PAIR( string("Sub Result2"), rs02.str() ));
		calc->push_back(COND_PAIR( string("Sub Result3"), ts1.str() ));
		calc->push_back(COND_PAIR( string("Sub Result4"), ts2.str() ));
#endif
		*result = (*result == -1.0 ? subtotal2 : *result * subtotal2);
	}
#ifdef VERBOSE
	stringstream ts3; ts3 << *result;
	calc->push_back(COND_PAIR( string("Result"), ts3.str() ));
	cout << "[CompositeK2::calcBDM]" << "P = " << *result << endl;
#endif
	return 0.0;
}

/*!
 * 親決定済みと親候補から親対象を自身の親としてよいかその条件の組を作成します
 */
int CompositeK2::calPPattern(string current, COND *cond, PROBS *probs, CHARS::iterator bn, CHARS::iterator en, bool f) {
	// 親決定済みと親候補から親対象を自身の親としてよいかその条件の組を作成します
	if (!f) bn++;
	if (bn == en) {
		// 一意文言の作成
		string valuep;
		for (COND::iterator iter = cond->begin(); iter != cond->end(); iter++) {
			valuep += iter->first + "=" + iter->second + ",";
		}
		valuep.erase(valuep.end() - 1);
		// 件数を求めます
		PROBS temp; long total;
		base->prob(current, cond, &temp, &total, false); // False->件数0の場合、0として扱う
#ifdef VERBOSE
		cout << "[CompositeK2::calPPattern]Condition(Subtotal::Nk) <- (" << valuep << ")=" << total << endl;
#endif
		probs->insert(pair<string, double>(valuep, total));
		return 0;
	}
	// 全処理対象ノードの全状態について処理を行います
	PROBS values; long total;
	base->prob(*bn, &values, &total);
	for (PROBS::iterator iter = values.begin(); iter != values.end(); iter++) {
		COND condc(*cond);
		condc.push_back(COND_PAIR(*bn, iter->first));
		if (calPPattern(current, &condc, probs, bn, en, false) != 0) {
			cout << "[CompositeK2::calPPattern]Function of calPPattern Failure" << endl;
			return 1;
		}
	}
	return 0;
}


/*!
* @brief 階乗を求めて返します
* @param[in] 桁数
* @param[in] 計算結果
*/
inline void CompositeK2::reduce(double s, double *r) {
	*r = (*r <= CEPS) ? s : *r * s;
	if ((--s) - 1.0 <= CEPS) return;
	reduce(s, r);
}

