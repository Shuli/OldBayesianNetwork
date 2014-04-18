//============================================================================
// Name        : ControllerLearn.cpp
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "ControllerLearn.h"
#include "ProbabilityEstimate.h"

ControllerLearn::ControllerLearn() {
}

ControllerLearn::~ControllerLearn() {
}

/*!
 * @brief 主処理を呼び出します
 */
int ControllerLearn::doProcessing(int argc, char **argv) {
	// 簡単な学習デモを記述します
	COND condition;
	condition.push_back(COND_PAIR("Test", "Test"));
	ProbabilityEstimate base("C:/test.csv");
	vector<int> result;
	base.learn(1.0, "Test1", "Test2", "Test3", &condition, 1, &result);

	for (unsigned int i = 0; i < result.size(); i++) {
		double total = 0;
		for (int j = 0; j < 10; j++) {
			if (i + j >= result.size() - 1) return 0;
			total += result[i + j];
		}
		cout << (total / 10.0) << endl;
	}
	return 0;
}

