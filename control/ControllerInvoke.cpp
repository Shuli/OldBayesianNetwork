//============================================================================
// Name        : ControllerInvoke.cpp
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "ControllerInvoke.h"

ControllerInvoke::ControllerInvoke() {
}

ControllerInvoke::~ControllerInvoke() {
}

/*!
 * @brief 主処理を呼び出します
 */
int ControllerInvoke::doProcessing(int argc, char **argv) {
	// 表題を表示します
	LINE("=");
	cout << "Starting the Bayesian Network Processsing" << endl;
	LINE("=");
	// 入力値を保持します
	string relation;
	if (argc == 2) {
		relation = RELATION_FILE; // デフォルトのファイル名を利用
	} else if (argc > 2) {
		relation = argv[2]; // 指定されたファイル名を利用
	} else {
		cout << "[ControllerInvoke::doProcessing]Usage:./network(.exe) [CSV-File] [Relations-File(Optional)]" << endl;
		return 1;
	}
	cout << "[ControllerInvoke::doProcessing]Relation <- " << relation << endl;

	// 簡単な推定デモを行います
	string csv(argv[1]);
	ProbabilityBase base(csv);
	base.load();

	// データからBDMを用いてノード構造を作成します
	if (argc == 2) {
		// Nodeの構造ファイルが指定されない場合は、データから作成します
		CompositeK2 k2(&base);
		k2.createNodeRelation();
		cout << "[ControllerInvoke::doProcessing]Nodes.csv Created" << endl;
	} else {
		cout << "[ControllerInvoke::doProcessing]Nodes.csv Not Created" << endl;
	}

	// 命令入力の受付
	LINE("=");
	cout << "Waiting for Command..." << endl;
	LINE("=");
	string source = "Init";
	CompositeBase *node;

	// 命令文の取得
	getline(cin, source);
	cout << "[ControllerInvoke::doProcessing]Input <-" << source << endl;

	// コマンドをパースします
	string comm, targetn; COND condition;
	parseCommand(source, &comm, &targetn, &condition);

	// BPで推定します
	node = new CompositeBase(&base, relation);
	string targetc;
	for (COND::iterator iter = condition.begin(); iter != condition.end(); iter++) {
		node->setProb(iter->first, iter->second);
		cout << "[ControllerInvoke::doProcessing]Evidence:" << iter->first << "=" << iter->second << endl;
		targetc = iter->first;
	}
	cout << "[ControllerInvoke::doProcessing]Evidence:" << targetc << endl;
	node->calProbs(targetc);

	// 結果を表示します
	PROBS result;
	node->getProb(targetn, &result);
	LINE("=");
	cout << "Result" << endl;
	LINE("=");
	for (PROBS::iterator iter = result.begin(); iter != result.end(); iter++) {
		cout << "[ControllerInvoke::doProcessing]" << iter->second << "(" << iter->first << ")" << endl;
	}
	delete node;

	return 0;
}

/*!
 * @brief 命令文を分解して返します
 */
int ControllerInvoke::parseCommand(string source, string *comm, string *targetn, COND *condition) {
	condition->clear();
	// コマンドを取得します
	unsigned int pos1 = source.find("("), pos2 = source.find(")");
	if (pos1 == string::npos || pos2 == string::npos) {
		cout << "[ControllerInvoke::parseCommand]Invalid Command <- " << source << endl;
		return 1;
	}
	*comm = source.substr(0, pos1);
	cout << "[ControllerInvoke::parseCommand]Command <- " << *comm << endl;

	// 条件部分前の対象ノードを取得します
	string next = source.substr(pos1 + 1);
	unsigned int pos = source.find("|");
	if (pos == string::npos) {
		// 対象ノードの取得
		pos = next.find(")");
		if (pos == string::npos) {
			cout << "[ControllerInvoke::parseCommand]Invalid Command <- " << source << endl;
			return 2;
		}
		// 「=」以降は省略します
		next = next.substr(0, pos);
		pos = next.find("=");
		if (pos != string::npos) {
			next = next.substr(0, pos);
			cout << "[ControllerInvoke::parseCommand]Revise Node <- " << next << endl;
		}
		*targetn = next;
		cout << "[ControllerInvoke::parseCommand]Target Node <- " << *targetn << endl;
		return 0;
	}

	// 「|」がある場合の対象ノードを取得します
	pos = next.find("|");
	string before = next.substr(0, pos);
	pos1 = before.find("=");
	if (pos1 != string::npos) {
		// 「|」以前の「=」以降は省略します
		before = before.substr(0, pos1);
		cout << "[ControllerInvoke::parseCommand]Revise Node <- " << before << endl;
	}
	*targetn = before;
	cout << "[ControllerInvoke::parseCommand]Target Node <- " << *targetn << endl;

	// 「|」の条件以降を処理します
	next = next.substr(pos + 1);
	do {
		pos = next.find(",");
		if (pos == string::npos) {
			// 「,」がない場合
			pos1 = next.find(")");
			if (pos1 == string::npos) {
				cout << "[ControllerInvoke::parseCommand]Invalid Command <- " << next << endl;
				return 3;
			}
			string cond = next.substr(0, pos1);
			cout << "[ControllerInvoke::parseCommand]Condition <- " << cond << endl;
			string key, value;
			if (parseCondition(cond, &key, &value) != 0) return 4;
			condition->push_back(COND_PAIR(key, value));

		} else {
			// 「,」がある場合
			pos1 = next.find(",");
			string cond = next.substr(0, pos1);
			cout << "[ControllerInvoke::parseCommand]Condition <- " << cond << endl;
			string key, value;
			if (parseCondition(cond, &key, &value) != 0) return 4;
			condition->push_back(COND_PAIR(key, value));
			next = next.substr(pos + 1);
		}

	} while (pos != string::npos);

	return 0;
}

/*!
 * @brief 条件を分解して返します
 */
inline int ControllerInvoke::parseCondition(string source, string *key, string *value) {
	unsigned int pos = source.find("=");
	if (pos == string::npos) {
		cout << "[ControllerInvoke::parseCommand]Invalid Condition <- " << source << endl;
		return 1;
	}
	*key = source.substr(0, pos);
	*value = source.substr(pos + 1);
	cout << "[ControllerInvoke::parseCommand]Key <- " << *key << endl;
	cout << "[ControllerInvoke::parseCommand]Value <- " << *value << endl;
	return 0;
}

/*!
 * @brief BN処理を開始します
 */
int main(int argc, char **argv) {
	ControllerInvoke invoke;
	invoke.doProcessing(argc, argv);
}
