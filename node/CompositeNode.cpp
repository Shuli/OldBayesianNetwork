//============================================================================
// Name        : CompositeNode.cpp
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "CompositeNode.h"

/*!
 * @brief ノードの名前と全データ処理への参照を必須引数とします
 * @param[in] string          本ノードの名前
 * @param[in] ProbabilityBase 全データを元にした条件付き確率処理
 */
CompositeNode::CompositeNode(string name, ProbabilityBase *cpt) {
    // ノード構造用情報を保持します
	// 基本情報を保持します
	this->name   = name;  // 自身の名前
	this->cpt    = cpt;   // 確率への参照
    this->depth  = 0;     // ネットワーク上所属する階層レベル
	// メッセージ受信を可能とします
	this->recv   = true;  // true=受信可能状態
	// 初期化処理を行います
	// 事前確率を求めます
	long total;
	cpt->prob(name, &prior, &total);
	// 要素名を作成します
	for (PROBS::iterator iter = prior.begin(); iter != prior.end(); iter++) {
		this->elements.push_back(iter->first);
		iter->second /= total; // 件数から確率への変換
	}
}

/*!
 * @brief 親要素を保持します
 * @param[in] CompositeNode* 親要素/Parents' element(node)
 */
int CompositeNode::addParent(CompositeNode *node) {
    parents.insert(pair<string, CompositeNode*>(node->name, node));
    return 0;
}

/*!
 * @brief 子要素を保持します
 * @param[in] CompositeNode* 子要素/Child element(Node)
 */
int CompositeNode::addChild(CompositeNode *node) {
    children.insert(pair<string, CompositeNode*>(node->name, node));
    return 0;
}

/*!
 * @brief 該当する事前確率を返します
 * @param[in]  string  対象要素名
 * @param[out] double* 該当事前確率
 */
int CompositeNode::calPrior(string target, UD *result) {
	// 指定要素の事前確率を求めます
	PROBS::iterator iter = prior.find(target);
	if (iter == prior.end()) return 1;
	*result = iter->second;
	return 0;
}

/*!
 * @brief 条件付き確率を求めます
 */
int CompositeNode::calCpt(string parent, string statep, string statec, UD *result) {
	// 検索条件を親ノード名=親ノード状態名とします
	COND cond;
	cond.push_back(COND_PAIR(parent, statep));
	// 確率を取得します
	PROBS probs; long total;
	cpt->prob(name, &cond, &probs, &total);
	// 該当する自分の状態の確率を返します
	PROBS::iterator iter = probs.find(statec);
	if (iter == probs.end()) *result = 0.0; // 該当なし時は確率0です
	else *result = iter->second / total;
#ifdef VERBOSE
	cout << "[CompositeNode::calCpt][TargetNode=" << name << "]" << "P(" << name << "=" << statec
		 << "|" << parent << "=" << statep << ")=" << *result << endl;
#endif
	return 0;
}

/*!
 * @brief πエビデンスを計算します
 */
int CompositeNode::calEviPai() {
	// 全ての親からのπメッセージを積算します、つまりΠπX(Ui)を求めます
	if (parents.empty()) {
		// 親が存在しない為、事前確率の積算を解とします
		for (CHARS::iterator iter2 = elements.begin(); iter2 != elements.end(); iter2++) {
			PROBS::iterator iter3 = prior.find(*iter2);
			if (iter3 == prior.end()) {
				printf("\t%s has not prior prob for %s(calEviPai:3)\n", name.c_str(), iter2->c_str());
				return 3;
			}
			// 算出した状態毎のπエビデンスを保持します
			PROBS::iterator statec = eviPai.find(*iter2);
			if (statec == eviPai.end()) {
				printf("\t%s not found evi pai %s(calEviPai:4)\n", name.c_str(), iter2->c_str());
				return 4;
			} else {
				statec->second = iter3->second;
#ifdef VERBOSE
				cout << "[CompositeNode::calEviPai][TargetNode=" << name << "]P(" << name << "=" << iter3->first << ")="
					 << iter3->second << "(NoParents→Prior)" << endl;
#endif
			}
		}
		// 正常終了します
		return 0;
	}

	// ∑ P(X|U1,...,Un)Ππx(Ui)を求めます(計を求めないのは各状態の確率を保持する為)
	PROBS probs; COND cond;
	calCptPai(&cond, &probs, parents.begin(), parents.end(), true);
	for (PROBS::iterator iter1 = probs.begin(); iter1 != probs.end(); iter1++) {
		PROBS::iterator statec = eviPai.find(iter1->first);
		if (statec == eviPai.end()) {
			printf("\t%s not found evi pai %s(calEviPai:2)\n", name.c_str(), iter1->first.c_str());
			return 2;
		} else {
			statec->second = iter1->second;
		}
#ifdef VERBOSE
		cout << "[CompositeNode::calEviPai][TargetNode=" << name << "]P(" << name << "=" << iter1->first << ")=" << statec->second << endl;
#endif
	}
	return 0;
}

/*!
 * @brief 条件付き確率P(X|Y1,...,Yn)を求めます（実処理）
 * @param[in]  string* 対象ノード名
 * @param[in]  string* 対象ノードの要素名
 * @param[in]  NODE*   対象親ノード
 * @param[in]  CHARS*  対象親ノードの要素
 * @param[out] PROBS*  算出条件付き確率∑n{P(CN=CE|PN1=PE1,...,PNn=PEn)}を保持します
 * @param[in]  bool    初回処理時のみポインタ移動を行いません
 */
int CompositeNode::calCptPai(COND *cond, PROBS *probs, NODES::iterator targetpn, NODES::iterator targetpe, bool first) {
	// 自身のノードの処理を行います
	if (!first) targetpn++;
	if (targetpn == targetpe) {
		// 条件付き確率を求め、状態毎に合計します
		PROBS result; long total;
		cpt->prob(name, cond, &result, &total);
		// ログ表示用処理
#ifdef VERBOSE
		string valuep;
		for (COND::iterator iter = cond->begin(); iter != cond->end(); iter++) {
			valuep += iter->first + "=" + iter->second + ",";
		}
		valuep.erase(valuep.end() - 1);
#endif
		// 指定条件でのπメッセージ積算を求めます
		UD parentp = 0.0;
		this->calPaiTotal(cond, &parentp);
		// 値を代入して返します
		for (PROBS::iterator iter = result.begin(); iter != result.end(); iter++) {
			PROBS::iterator targetp = probs->find(iter->first);
#ifdef VERBOSE
			cout << "[CompositeNode::calCptPai][TargetNode=" << name << "]P(" << name << "=" << iter->first << "|" << valuep << ")=" << (iter->second / total) << endl;
#endif
			double paim = (iter->second / total) * parentp;
			if (targetp == probs->end()) probs->insert(PROBS_PAIR(iter->first, paim));
			else targetp->second += (paim);
		}
		cond->clear();
		return 0;
	}
	// 全要素に対して処理を行います
	CompositeNode *next = targetpn->second;
	for (CHARS::iterator iter = next->elements.begin(); iter != next->elements.end(); iter++) {
		COND condc(*cond);
		condc.push_back(COND_PAIR(next->name, *iter));
		calCptPai(&condc, probs, targetpn, targetpe, false);
	}
	return 0;
}

/*
 * 親ノードの状態毎のΠメッセージ（ない場合は事前確率）の積算を返します
 */
int CompositeNode::calPaiTotal(COND *cond, double *result) {
	// 自身の親ノードに格納されている全てのπメッセージの積算を求めます
	UD sumPai = -1.0;
	for (COND::iterator iter1 = cond->begin(); iter1 != cond->end(); iter1++) {
		// 対象親ノードを取得します
		// 対象親ノードの状態名を持つπメッセージを取得します
		PROBS::iterator iter3 = msgPai.find(iter1->second);
		if (iter3 == msgPai.end()) {
			printf("\t%s(%s) not found msg pai for %s->%s(calParent:2)\n",
					name.c_str(), iter1->first.c_str(), iter1->first.c_str(), iter1->second.c_str());
			return 2;
		}
		// 積算を求めます
#ifdef VERBOSE
		cout << "[CompositeNode::calParent][TargetNode=" << name << "]P(" << iter1->first << "=" << iter1->second << ")=" << iter3->second << endl;
#endif
		if (sumPai == -1.0) sumPai = iter3->second;
		else sumPai *= iter3->second;
	}
#ifdef VERBOSE
	cout << "[CompositeNode::calParent][TargetNode=" << name << "]*P(" << name << ")=" << sumPai << endl;
#endif
	*result = sumPai;
	return 0;
}

/*!
 * @brief λエビデンスを計算します
 */
int CompositeNode::calEviLambda() {
	// 自身をX、子ノードをVとした場合、ΠλV(X)でλエビデンスを求めます
	for (CHARS::iterator iter1 = elements.begin(); iter1 != elements.end(); iter1++) {
		UD sumLambda = -1.0;
		for (NODES::iterator iter2 = children.begin(); iter2 != children.end(); iter2++) {
			// 自身の子ノードを探します
			CompositeNode *target = iter2->second;
			// λメッセージは子ノードが保持している自分の名前が付けられたλメッセージを取得します
			PROBS::iterator result = target->msgLambda.find(*iter1);
			if (result == target->msgLambda.end()) {
				printf("\t%s not found msg lambda of %s(calEviLambda:1)\n", name.c_str(), iter1->c_str());
				return 1;
			}
			// λメッセージの積算を求めます
#ifdef VERBOSE
			cout << "[CompositeNode::calEviLambda][TargetNode=" << name << "]L_" << target->name
					<< "(" << name << "=" << *iter1 << ")=" << result->second << endl;
#endif
			if (sumLambda == -1.0) sumLambda = result->second;
			else sumLambda *= result->second;
		}
		// 値がない場合は子がない為、一様分布1.0を与えます
		if (sumLambda == -1.0) sumLambda = 1.0;
		// 自身の状態毎にλエビデンスを保持します
		PROBS::iterator statec = eviLambda.find(*iter1);
		if (statec == eviLambda.end()) {
			printf("\t%s not found evi lambda of %s(calEviLambda:2)\n", name.c_str(), iter1->c_str());
			return 2;
		}
		// 葉ノードの場合、0に初期化してしまう為、計算を行いません
		statec->second = sumLambda;
		// ログ出力
#ifdef VERBOSE
		cout << "[CompositeNode::calEviLambda][TargetNode=" << name << "]*L(" << name << "=" << *iter1 << ")=" << sumLambda << endl;
#endif
	}
	return 0;
}

/*!
 * @brief 事後確率を求めます
 */
int CompositeNode::calProb() {
	// 正規化定数を求めます
	UD normal = 0;
	calNormal(&normal);
	// 各状態の事後確率を求めます
	for (CHARS::iterator iter = elements.begin(); iter != elements.end(); iter++) {
		// 本ノードの状態のπ(Xi)を取得します
		PROBS::iterator ep = eviPai.find(*iter);
		if (ep == eviPai.end()) {
			printf("\t%s has not pai msg for %s(calProb:1)\n", name.c_str(), (*iter).c_str());
			return 1;
		}
		// 本ノードの状態のλ(Xi)を取得します
		PROBS::iterator el = eviLambda.find(*iter);
		if (el == eviLambda.end()) {
			printf("\t%s has not lambda msg for %s(calProb:2)\n", name.c_str(), (*iter).c_str());
			return 2;
		}
		// αλ(X)π(X)を保持します
		PROBS::iterator target = posterior.find(*iter);
		if (target == posterior.end()) {
			printf("\t%s has not prob(pr) for %s(calProb:3)\n", name.c_str(), (*iter).c_str());
			return 3;
		}
		// 事後確率を更新します
		target->second = normal * el->second * ep->second;
#ifdef VERBOSE
		cout << "[CompositeNode::calProb][TargetNode=" << name << "]Pr(" << name << "=" << *iter << ")="
				<< normal << "*" << el->second << "*" << ep->second << "=" << target->second << endl;
#endif
	}
	return 0;
}

/*!
 * @brief 正規化定数を求めます
 * @param[out] double* 本ノードで求められた正規化定数です
 */
int CompositeNode::calNormal(UD *result) {
	*result = 0.0;
	// 全λ(X)とπ(X)の積算を合計します
	for (CHARS::iterator iter = elements.begin(); iter != elements.end(); iter++) {
		// 本ノードの状態のπ(Xi)を取得します
		PROBS::iterator ep = eviPai.find(*iter);
		if (ep == eviPai.end()) {
			printf("\t%s has not pai msg for %s(calNormal:1)\n", name.c_str(), (*iter).c_str());
			return 1;
		}
		// 本ノードの状態のλ(Xi)を取得します
		PROBS::iterator el = eviLambda.find(*iter);
		if (el == eviLambda.end()) {
			printf("\t%s has not lambda msg for %s(calNormal:2)\n", name.c_str(), (*iter).c_str());
			return 2;
		}
		*result += (ep->second * el->second);
#ifdef VERBOSE
		cout << "[CompositeNode::calNormal][TargetNode=" << name << "]*e=" << ep->second << ",Le=" << el->second << endl;
#endif
	}
#ifdef VERBOSE
	cout << "[CompositeNode::calNormal][TargetNode=" << name << "]normal=" << *result << endl;
#endif
	// 1.0/Pにより正規化します
	*result = *result == 0 ? 0 : (1.0 / *result);
	return 0;
}

/*!
 * @brief πメッセージを計算します/πメッセージ及びλメッセージは直接対象ノードに代入します
 * @param[in] string 送信ノード名
 */
int CompositeNode::calMsgPai(string sender) {
	// ルートノード時は処理しません
	if (sender == ROOT_NODE) return 0;

	// πメッセージを作成します
	// πメッセージは子が保持している自分へのλメッセージを取得して、子供に自分の名前でπメッセージを書き込みます
	// メッセージ格納対象ノードを取得します
	NODES::iterator result1 = children.find(sender);
	if (result1 == children.end()) {
		printf("\t%s not found child node of %s(calMsgPai:4)\n", name.c_str(), sender.c_str());
		return 4;
	}
	CompositeNode *child = result1->second;

	// 子ノード内の自分を探し出す
	for (CHARS::iterator iter2 = elements.begin(); iter2 != elements.end(); iter2++) {
		// 自身の事後確率を取得します
		PROBS::iterator ret1 = posterior.find(*iter2);
		if (ret1 == posterior.end()) {
			printf("\t%s not found prob of %s(calMsgPai:1)\n", name.c_str(), (*iter2).c_str());
			return 1;
		}
		// 子ノードに格納されている自身のλメッセージを取得します
		PROBS::iterator ret2 = child->msgLambda.find(*iter2);
		if (ret2 == child->msgLambda.end()) {
			printf("\t%s not found msg lambda of %s on %s(calMsgPai:2)\n", name.c_str(), iter2->c_str(), child->name.c_str());
			return 2;
		}
		// πメッセージを算出し、子ノードに自身の名前で格納します
		UD answer = ret2->second == 0 ? 0 : (ret1->second / ret2->second);
		// 対象以外のノードには算出確率を書き込みません
		PROBS::iterator temp = child->msgPai.find(*iter2);
		if (temp == child->msgPai.end()) {
			printf("\t%s not found msg pai of %s on %s(calMsgPai:3)\n", name.c_str(), iter2->c_str(), child->name.c_str());
			return 3;
		}
		temp->second = answer;
	}

	return 0;
}

/*!
 * @brief λメッセージを計算します
 * @param[in] string 送信ノード名
 */
int CompositeNode::calMsgLambda(string sender) {
	// ルートノード時は処理しません
	if (sender == ROOT_NODE) return 0;
	if (parents.size() <= 0) return 0;

	// 対象ノード用のλメッセージを作成します
	// λメッセージは親に書き込まれたπメッセージを元に親へのλメッセージを自分に親の名前で書き込みます
#ifdef VERBOSE
	cout << "[CompositeNode::calMsgLambda][TargetNode=" << name << "]begin calc for " << name << " by " << sender << endl;
#endif
	NODES::iterator partemp1 = parents.find(sender);
	if (partemp1 == parents.end()) {
		printf("%s not found parent node of %s(calMsgLambda:5)\n", name.c_str(), sender.c_str());
		return 5;
	}
	// 処理対象ノードを決定します
	NODES targets;
	if (parents.size() > 1) {
		// 複数の場合はその親「以外」を対象に条件付き確率を求めます
		// 1の場合は以下の処理で親を対象としなくて良い為、対象ノードを設定しません
		for (NODES::iterator iter = parents.begin(); iter != parents.end(); iter++) {
			if (iter->second->name != sender) {
				targets.insert(pair<string, CompositeNode*>(iter->second->name, iter->second));
			}
		}
	}

	// ∑ P(x|u1,...,un)λ(x)部分を求めます
	CompositeNode *parentn = partemp1->second;
	for (CHARS::iterator iter1 = parentn->elements.begin(); iter1 != parentn->elements.end(); iter1++) {
		UD subtotal = 0.0;
#ifdef VERBOSE
		cout << "[CompositeNode::calMsgLambda][TargetNode=" << name << "]target-condition->" << partemp1->second->name << "=" << *iter1 << endl;
#endif
		PROBS probs; COND cond;
		cond.push_back(COND_PAIR(parentn->name, *iter1));
		calCptLambda(*iter1, (targets.size() > 0), &cond, &probs, targets.begin(), targets.end(), true);
		for (PROBS::iterator iter2 = probs.begin(); iter2 != probs.end(); iter2++) {
#ifdef VERBOSE
			cout << "[CompositeNode::calMsgLambda][TargetNode=" << name << "]" << iter2->first << "=" << iter2->second << endl;
#endif
			subtotal += iter2->second;
		}
#ifdef VERBOSE
		cout << "[CompositeNode::calMsgLambda][TargetNode=" << name << "]" << partemp1->second->name << "=" << *iter1 << "->" << subtotal << endl;
#endif
		PROBS::iterator targetr = msgLambda.find(*iter1);
		if (targetr == msgLambda.end()) msgLambda.insert(pair<string, UD>(*iter1, subtotal));
		else targetr->second = subtotal;
	}
	return 0;
}

/*!
 * @brief 条件付き確率P(X|Y1,...,Yn)を求めます（実処理）
 * @param[in]  COND*   			条件付き確率の条件
 * @param[out] PROBS*  			算出条件付き確率∑n{P(CN=CE|PN1=PE1,...,PNn=PEn)}を保持します
 * @param[in]  NODES::iterator  処理開始ポインタ・その１
 * @param[in]  NODES::iterator  処理開始ポインタ・その２
 * @param[in]  bool    初回処理時のみポインタ移動を行いません
 */
int CompositeNode::calCptLambda(string parents, bool pai, COND *cond, PROBS *probs, NODES::iterator targetpn, NODES::iterator targetpe, bool first) {
	// 自身のノードの処理を行います
	if (!first) targetpn++;
	if (targetpn == targetpe) {
		// 条件付き確率を求め、状態毎に合計します
		PROBS result; long total;
		cpt->prob(name, cond, &result, &total);
		// ログ表示用処理
#ifdef VERBOSE
		string valuep;
		for (COND::iterator iter = cond->begin(); iter != cond->end(); iter++) {
			valuep += iter->first + "=" + iter->second + ",";
		}
		valuep.erase(valuep.end() - 1);
#endif
		// 自身のノード以外のπメッセージの積算を求めます
		double othert = -1.0;
		if (pai) {
			for (COND::iterator iter = cond->begin(); iter != cond->end(); iter++) {
				if (iter->second != parents) {
					// 該当するπメッセージを取得して積を求めます
					PROBS::iterator ipai = msgPai.find(iter->second);
					if (ipai == msgPai.end()) {
						printf("not found pai message(%s=%s) on %s node", iter->first.c_str(), iter->second.c_str(), name.c_str());
						return 1;
					}
#ifdef VERBOSE
					cout << "[CompositeNode::calCptLambda][TargetNode=" << name << "]Pm(" << iter->first << "=" << iter->second << ")=" << ipai->second << endl;
#endif
					if (othert == -1.0) othert = ipai->second;
					else othert *= ipai->second;
				}
			}
#ifdef VERBOSE
			cout << "[CompositeNode::calCptLambda][TargetNode=" << name << "]*Pm=" << othert << endl;
#endif
		}
		// 値を代入して返します
		for (PROBS::iterator iter = result.begin(); iter != result.end(); iter++) {
			PROBS::iterator targetp = probs->find(iter->first);
			// 対象ラムダエビデンスを取得します
			PROBS::iterator parentp = eviLambda.find(iter->first);
			if (parentp == eviLambda.end()) {
				printf("not found this lambda evidense(%s)\n", iter->first.c_str());
				return 1;
			}
			//　合計を求めます
			double paim = iter->second / total * parentp->second * (pai ? othert : 1.0);
			if (targetp == probs->end()) probs->insert(PROBS_PAIR(iter->first, paim));
			else targetp->second += (paim);
#ifdef VERBOSE
			cout << "[CompositeNode::calCptLambda][TargetNode=" << name << "]Le(" << name << "=" << iter->first << ")=" << parentp->second << endl;
			cout << "[CompositeNode::calCptLambda][TargetNode=" << name << "]P(" << name << "=" << iter->first << "|" << valuep << ")=" << (iter->second / total) << endl;
#endif
		}
		cond->clear();
		return 0;
	}
	// 全要素に対して処理を行います
	CompositeNode *next = targetpn->second;
	for (CHARS::iterator iter = next->elements.begin(); iter != next->elements.end(); iter++) {
		COND condc(*cond);
		condc.push_back(COND_PAIR(next->name, *iter));
		calCptLambda(parents, pai, &condc, probs, targetpn, targetpe, false);
	}
	return 0;
}

/*!
 * @brief 関連するノードに階層構造を利用してメッセージ転送します
 * @param[in] CompositeNode* 送信ノードへの参照
 */
int CompositeNode::transMessage(CompositeNode *sender) {
	// 転送ログを出力します
	// すでに処理済みの場合は処理を正常終了します
	if (recv == false) return 0;
	recv = false;

	// 親を優先して検索します
	for (NODES::iterator iter1 = parents.begin(); iter1 != parents.end(); iter1++) {
		CompositeNode *target1 = iter1->second;
		if (target1->recv) {
			// 計算時間の計測を開始します
			double begin = nowtime();
			// 移動前にメッセージ計算を行います
			if (!parents.empty()) calMsgLambda(target1->name);
			target1->calEviLambda();
			target1->calProb();
			now(string("Sending Lambda Message ") + name + string("->") + target1->name);
			target1->now(string("Sending Lambda Message ") + name + string("->") + target1->name);
			// 計算時間を表示します
			printf("%s caluculate lambda msg for %s(%fsec)\n", name.c_str(), target1->name.c_str(), (nowtime() - begin));
			// 処理を転送します
			target1->transMessage(this);
		}
	}
	// 親がない場合は子を検索します
	for (NODES::iterator iter2 = children.begin(); iter2 != children.end(); iter2++) {
		CompositeNode *target2 = iter2->second;
		if (target2->recv) {
			// 計算時間の計測を開始します
			double begin = nowtime();
			// 移動前にメッセージ計算を行います
			calMsgPai(target2->name);
			target2->calEviPai();
			target2->calProb();
			now("Sending Pai Message " + name + string("->") + target2->name);
			target2->now("Sending Pai Message " + name + string("->") + target2->name);
			// 計算時間を表示します
			printf("%s caluculate pai msg for %s(%fsec)\n", name.c_str(), target2->name.c_str(), (nowtime() - begin));
			// 処理を転送します
			target2->transMessage(this);
		}
	}

	return 0;
}

int CompositeNode::now(string title) {
	char text[1024];
	// タイトルの表示
	LINE("=");
	cout << title << endl;
	LINE("=");
	// サブタイトルの表示
	LINE("-");
	cout << "Target Node: " << name << endl;
	LINE("-");

	// 確率を出力します
	for (PROBS::iterator iter2 = prior.begin(); iter2 != prior.end(); iter2++) {
		sprintf(text, "%f=Pb(%s=%s)", iter2->second, name.c_str(), iter2->first.c_str());
		cout << "[CompositeBase::now]" << text << endl;
	}
	LINE("-");
	// 事後確率を出力します
	for (PROBS::iterator iter2 = posterior.begin(); iter2 != posterior.end(); iter2++) {
		sprintf(text, "%f=Pr(%s=%s)", iter2->second, name.c_str(), iter2->first.c_str());
		cout << "[CompositeBase::now]" << text << endl;
	}
	// πエビデンスを出力します
	LINE("-");
	for (PROBS::iterator iter2 = eviPai.begin(); iter2 != eviPai.end(); iter2++) {
		sprintf(text, "%f=Pe(%s=%s)", iter2->second, name.c_str(), iter2->first.c_str());
		cout << "[CompositeBase::now]" << text << endl;
	}
	// λエビデンスを出力します
	LINE("-");
	for (PROBS::iterator iter2 = eviLambda.begin(); iter2 != eviLambda.end(); iter2++) {
		sprintf(text, "%f=Le(%s=%s)", iter2->second, name.c_str(), iter2->first.c_str());
		cout << "[CompositeBase::now]" << text << endl;
	}
	// πメッセージを出力します
	LINE("-");
	for (PROBS::iterator iter2 = msgPai.begin(); iter2 != msgPai.end(); iter2++) {
		sprintf(text, "%f=Pm(%s)", iter2->second, iter2->first.c_str());
		cout << "[CompositeBase::now]" << text << endl;
	}
	// λメッセージを出力します
	LINE("-");
	for (PROBS::iterator iter2 = msgLambda.begin(); iter2 != msgLambda.end(); iter2++) {
		sprintf(text, "%f=Lm(%s)", iter2->second, iter2->first.c_str());
		cout << "[CompositeBase::now]" << text << endl;
	}
	LINE("-");
	return 0;
}

