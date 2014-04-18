//============================================================================
// Name        : ControllerLearn.h
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#ifndef CONTROLLERLEARN_H_
#define CONTROLLERLEARN_H_

#include "ControllerBase.h"

/*!
 * @brief CPTの学習を行う処理を行います
 */
class ControllerLearn : public ControllerBase {

public:
	/*!
	 * @brief
	 */
	ControllerLearn();

	/*!
	 * brief
	 */
	virtual ~ControllerLearn();

	/*!
	 * @brief 主処理を呼び出します
	 */
	int doProcessing(int argc, char **argv);

};

#endif /* CONTROLLERLEARN_H_ */
