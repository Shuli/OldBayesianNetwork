//============================================================================
// Name        : CompositeBase.cpp
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "CompositeBase.h"

/*!
 * @brief BN構造とCPT引数を元にBN処理を行います
 * @param[in] ProbabilityBase* データを保持した確率処理(CPTを提供)
 */
CompositeBase::CompositeBase(ProbabilityBase *vfile, string relations) {
	// CPT処理とパース処理を保持します
	this->vfile = vfile;
	// ファイル名を保持します
	this->relations = relations;
	// BNを作成します
	invoke();
}

/*!
 * @brief BayesianNetworkを作成します
 * @return 0=正常終了
 */
int CompositeBase::invoke() {
	// 各ノードの親子関係と隣接はCSVファイルで手動定義します
	// 同時に事前確率も読み込んで保持しています
	LINE("=");
	cout << "Starting the Belif Propagation" << endl;
	LINE("=");
	LINE("=");
	cout << "Creating Network" << endl;
	LINE("=");
	createNetwork();
    // 各ノードに階層レベルを設定します
    for (NODES::iterator iter = nodes.begin(); iter != nodes.end(); iter++) {
        int depth = 0;
        createDepth(depth, iter->second);
    }
    for (NODES::iterator iter = nodes.begin(); iter != nodes.end(); iter++) {
		if (maxDepth < iter->second->depth) maxDepth = iter->second->depth;
	}
    // 構築したBayesianNetworkを初期化します
    format();
    return 0;
}

/*!
 * @brief BayesianNetworkを構造情報を元に構築します
 * @return 0=正常終了
 */
int CompositeBase::createNetwork() {
#ifdef TIME
    double begin = nowtime();
#endif
    // 既にnodesが存在する場合は、解放します
    while (nodes.begin() != nodes.end()) {
    	NODES::iterator iter = nodes.begin();
    	const char *targete = iter->first.c_str();
    	CompositeNode *targetn = iter->second;
     	free((void*)targete);
     	delete targetn;
     	nodes.erase(nodes.begin());
    }

    // ノード構造定義CSVファイル（名前+.csv）を読み込んで保持します
    ifstream fin(relations.c_str(), ios::in);
    if (!fin) return 1;

    // 事前確率を保持した確率変数を関連付けます
    string line;
    int row = 0;
    vector<CompositeNode*> targets;
	ProbabilityParse parse(fin);
	while (!parse.isEof()) {
        // １行目はタイトル行です
        int col = 0;
        // タイトル行を元に要素を作成します
		while (!parse.isBreak()) {
        	// 1データを取得します
        	string element;
			parse >> element;
            if (col != 0) {
                if (row == 0) {
                    // タイトル行を元に要素を作成します
                    // ノードの新規作成
					// 但し、ノードは試験ケース毎に定義しない（簡易試験の為）
					CompositeNode *target = new CompositeNode(element, vfile);
                    // 生成ノード列挙登録
					nodes.insert(pair<string, CompositeNode*>(element, target));
                    targets.push_back(target);
					printf("[CompositeBase::createNetwork]Created %s\n", element.c_str());
                } else {
			        // 0!=親子関係ありとして関連を作成します
				    int depth = atoi(element.c_str());
					if (depth != 0) {
                        // 親子関係を作成します
						targets[col - 1]->addParent(targets[row - 1]);
                        targets[row - 1]->addChild (targets[col - 1]);
						printf("[CompositeBase::createNetwork]Created %s part-of %s\n", targets[col - 1]->name.c_str(), targets[row - 1]->name.c_str());
					}
				}
			}
			col++;
		}
		parse >> endl;
		row++;
    }
    fin.close();
    targets.clear();
#ifdef TIME
    printf("%f=createNodeWithPrior\n", (nowtime() - begin) / 1000.0);
#endif
    return 0;
}

/*!
 * @brief BayesianNetworkの各ノードに階層レベルを設定します
 * @param[in] int   階層レベル
 * @param[in] CompositeNode* 対象ノード
 * @return 0=正常終了
 */
int CompositeBase::createDepth(int depth, CompositeNode *node) {
     // 自ノードに階層レベルを定義します
     if (node->depth != 0) return 0; // 多重定義を防止します
     node->depth = ++depth;
     // 子ノードに階層レベル+1を定義します
     if (node->children.empty()) return 0;
     NODES *children = &node->children;
     for (NODES::iterator iter = children->begin(); iter != children->end(); iter++) {
         createDepth(depth, iter->second);
     }
     return 0;
}

/*!
 * @brief BPの処理用に全てのノードを初期化します
 * @return 0=正常終了
 */
int CompositeBase::format() {
	#ifdef TIME
	    double begin = nowtime();
	#endif
	// BP用変数が変動済みの場合を考慮して一度初期化します
	for (NODES::iterator iter1 = nodes.begin(); iter1 != nodes.end(); iter1++) {
		// 対象ノードを引き当てます
		CompositeNode *target = iter1->second;
		// 対象ノードの子供に保持されているπ・λメッセージを初期化します
		for (NODES::iterator iter3 = target->children.begin(); iter3 != target->children.end(); iter3++) {
			CompositeNode *child = iter3->second;
			child->msgLambda.clear();
			child->msgPai.clear();
		}
		// 対象ノードに保持されている事後確率、π・λエビデンスを初期化します
		target->posterior.clear();
		target->eviLambda.clear();
		target->eviPai.clear();
	}
	// BP用変数を初期化します
	UD prior;
	for (NODES::iterator iter1 = nodes.begin(); iter1 != nodes.end(); iter1++) {
		// 対象ノードを引き当てます
		CompositeNode *target = iter1->second;
		for (CHARS::iterator iter2 = target->elements.begin(); iter2 != target->elements.end(); iter2++) {
			// 全ての事後確率を事前確率として初期化します
			if (target->calPrior(*iter2, &prior) != 0) {
				printf("%s not found prior value for %s(:2)\n", target->name.c_str(), iter2->c_str());
				return 1;
			}
			target->posterior.insert(pair<string, UD>(*iter2, prior));
			// 全てのλエビデンスを1.0に初期化します
			target->eviLambda.insert(pair<string, UD>(*iter2, 1.0));
			// 全てのπエビデンスを初期化します
			if (target->parents.empty()) {
				// 全ての根ノードにおいてπエビデンスを事前確率（P(X)）に初期化します
				target->eviPai.insert(pair<string, UD>(*iter2, prior));
			} else {
				// （教科書に記述なし、又は自分の理解不足）
				// 全てのπエビデンスを1.0(事前確率）にして計算してみます
				target->eviPai.insert(pair<string, UD>(*iter2, 1.0));
			}
			// πメッセージ、λメッセージは子供に自分の名前で代入します
			for (NODES::iterator iter3 = target->children.begin(); iter3 != target->children.end(); iter3++) {
				CompositeNode *child = iter3->second;
				// 全てのλメッセージを1.0に初期化します
				child->msgLambda.insert(pair<string, UD>(*iter2, 1.0));
				// 全てのπメッセージを1.0に初期化します
				child->msgPai.insert(pair<string, UD>(*iter2, prior/*1.0*/));
			}
		}
	}
	// 現在のＢＮの状態を表示します
	this->now("Initialized");

	// 階層番号を元に各階層の全ノードを処理してより次階層を処理します
	// 確率を伝播させずにネットワーク上の階層レベルを用いて個々のノードで初期化を行います
	// πメッセージを伝播させます
	for (int i = 1; i <= maxDepth; i++) {
		for (NODES::iterator iter1 = nodes.begin(); iter1 != nodes.end(); iter1++) {
			CompositeNode *target = iter1->second;
			if (target->depth == i) {
				// πエビデンスを更新します
				if (target->calEviPai() != 0) return 2;
				// 事後確率を更新します
				if (target->calProb() != 0) return 3;
				// 初期化(format)時に既にπメッセージ伝播と同じ値を設定しています
			}
		}
	}
	// λメッセージの伝播は初期化時には必要ありません
	// 現在のＢＮの状態を表示します
	this->now("P Message Transfermed(for init)");

#ifdef TIME
	printf("%f=format\n", (nowtime() - begin) / 1000.0);
#endif
	for (int i = 0; i < 70; i++) cout << "-";
	cout << endl;

	return 0;
}

/*!
 * @brief 指定ノードの指定要素にエビデンスを与えます
 * @param[in]  string        対象ノード名を指定します
 * @param[in]  string        対象ノードの状態名を指定します
 * @return 0=正常終了
 */
int CompositeBase::setProb(string targetn, string targets) {
	// 指定ノードのエビデンスが決定された為、
	// πメッセージとλメッセージを伝播させます
	// 指定ノードを取得します
	NODES::iterator iter1 = nodes.find(targetn);
	if (iter1 == nodes.end()) {
		printf("Composite did not get the node of %s(calProb:1)\n", targetn.c_str());
		return 1;
	}
	// 自身の状態のλエビデンスを1.0に更新します
	// それ以外のλエビデンスを0.0に更新します
	CompositeNode *target = iter1->second;
	for (PROBS::iterator iter2 = target->eviLambda.begin(); iter2 != target->eviLambda.end(); iter2++) {
		iter2->second = (iter2->first == targets ? 1.0 : 0.0);
	}
	// 同様に事後確率も更新します
	for (PROBS::iterator iter2 = target->posterior.begin(); iter2 != target->posterior.end(); iter2++) {
		iter2->second = (iter2->first == targets ? 1.0 : 0.0);
	}
	// πメッセージの値πVi(X)のX*のみ1.0にして他を0.0に更新します
	for (NODES::iterator iter2 = target->children.begin(); iter2 != target->children.end(); iter2++) {
		CompositeNode *child = iter2->second;
		for (CHARS::iterator iter3 = target->elements.begin(); iter3 != target->elements.end(); iter3++) {
			PROBS::iterator iter4 = child->msgPai.find(*iter3);
			if (iter4 == child->msgPai.end()) {
				printf("%s not found msg of pai for %s(calProb:2)\n", child->name.c_str(), iter3->c_str());
				return 2;
			}
			iter4->second = (*iter3 == targets ? 1.0 : 0.0);
		}
	}
	now(string("Evidence(") + targetn + string("=") + targets + string(")"));
	return 0;
}

/*!
 * @brief BPを用いた推定（又は事後）確率を計算します
 * @param[in] string 対象ノード名を指定します
 */
int CompositeBase::calProbs(string targetn) {
	// 対象ノードを取得します
	NODES::iterator iter1 = nodes.find(targetn);
	if (iter1 == nodes.end()) {
		printf("Composite did not get the node of %s(calProb:1)\n", targetn.c_str());
		return 1;
	}
	// メッセージ伝播を行う前に各ノードの受信状態を受付可能に初期化します
	for (NODES::iterator all = nodes.begin(); all != nodes.end(); all++) {
		all->second->recv = true;
	}
	// 計算時間の計測を開始します
	double begin = nowtime();
	// 指定ノードを中心にλメッセージを優先して、π・λメッセージを伝播させます
	CompositeNode *target = iter1->second;
	target->transMessage(NULL);

	// 計算時間を表示します
	printf("Caluculate times for all probs(%fsec)\n",  (nowtime() - begin));
	return 0;
}

/*!
 * @brief BPを用いた推定（又は事後）確率を返します （対象ノードの全ての状態の確率を返します）
 * @param[in]  string              確率を取得したい対象ノード名
 * @param[OUT] map<string, double> 各状態の確率
 */
int CompositeBase::getProb(string targetn, PROBS *probs) {
	// 対象ノードを取得します
	NODES::iterator iter = nodes.find(targetn);
	if (iter == nodes.end()) {
		printf("Composite did not get the target node of %s(getProb)\n", targetn.c_str());
		return 1;
	}
	CompositeNode *target = iter->second;
	// ノード内の確率を返します
	probs->clear();
	for (PROBS::iterator iter = target->posterior.begin(); iter != target->posterior.end(); iter++) {
		probs->insert(pair<string, UD>(iter->first, iter->second));
	}
	return 0;
}

/*!
 * @brief 現在の状態をログ出力します
 */
int CompositeBase::now(string title) {
	for (NODES::iterator iter1 = nodes.begin(); iter1 != nodes.end(); iter1++) {
		CompositeNode *target = iter1->second;
		target->now(title);
	}
	return 0;
}



