//============================================================================
// Name        : ControllerInvoke.h
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#ifndef CONTROLLERINVOKE_H_
#define CONTROLLERINVOKE_H_

#include "ControllerBase.h"

/*!
 * @brief シンプルにBNを起動する処理を行います
 */
class ControllerInvoke : public ControllerBase {

public:
	/*!
	 * @brief
	 */
	ControllerInvoke();

	/*!
	 * @brief
	 */
	virtual ~ControllerInvoke();

	/*!
	 * @brief 主処理を呼び出します
	 */
	int doProcessing(int argc, char **argv);

protected:
	/*!
	 * @brief 命令文を分解して返します
	 */
	int parseCommand(string source, string *comm, string *targetn, COND *condition);

	/*!
	 * @brief 条件を分解して返します
	 */
	inline int parseCondition(string source, string *key, string *value);

};

#endif /* CONTROLLERINVOKE_H_ */
