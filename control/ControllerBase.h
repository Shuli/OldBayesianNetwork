//============================================================================
// Name        : ControllerBase.h
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#ifndef CONTROLLERBASE_H_
#define CONTROLLERBASE_H_

#include "CompositeBase.h"
#include "CompositeK2.h"

/*!
 * @brief 利用者が入力した命令に適した処理の基本処理を定義します
 */
class ControllerBase {

public:
	/*!
	 * @brief
	 */
	ControllerBase();

	/*!
	 * @brief
	 */
	virtual ~ControllerBase();

	/*!
	 * @brief 主処理を呼び出します
	 */
	int doProcessing(int argc, char **argv);

};

#endif /* CONTROLLERBASE_H_ */
