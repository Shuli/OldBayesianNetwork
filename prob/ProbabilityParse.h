//============================================================================
// Name        : ProbabilityParse.h
// Version     : 1.0
// Date        : 2010/04/14
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#ifndef PROBABILITYPARSE_H_
#define PROBABILITYPARSE_H_

#include "BayesianDefine.h"

/*!
 * @brief CSVデータをカンマと改行でParseします
 */
class ProbabilityParse {
	/*!
	 * @brief マニュピュレータを定義します
	 */
    friend ProbabilityParse& endl(ProbabilityParse& src) { return src; }

private:
	/*!
	 * @brief 読み込みストリームを必須と引数します
	 */
	ProbabilityParse();

public:
	/*!
	 * @brief 読み込みストリームを必須引数とし初期化処理を行います
	 */
	ProbabilityParse(istream& ifs) : ifs(ifs) {
		ibreak = false;
		ieof = false;
	}

	/*!
	 * @brief 終了処理を行います
	 */
	virtual ~ProbabilityParse() {}

private:
	/*!
	 * @brief 現在参照しているファイルへの参照を保持します
	 */
	istream& ifs;
	/*!
	 * @brief 改行コードを現在読み込んでいるか否か保持します
	 */
	bool ibreak;
	/*!
	 * @brief EOFを現在読み込んでいるか否か保持します
	 */
	bool ieof;

public:
	/*!
	 * @brief カンマまでを読み込んで返します
	 */
	ProbabilityParse& operator >>(string& ret) {
		int c;
		while ((c = ifs.get()) != ',' && c != EOF && c != '\n') {
			ret.append(1, c);
		}
		ieof   = (c == EOF  ? true : false);
		ibreak = (ieof ? true : (c == '\n' ? true : false));
		if (ieof == true) ifs.clear();
		return *this;
	}

	/*!
	 * @brief endlの場合、改行有無を改行無しに更新します
	 */
	ProbabilityParse& operator >>(ProbabilityParse& (*pmanipulatorfunction)(ProbabilityParse&)) {
		ibreak = false;
		return (*pmanipulatorfunction)(*this);
	}

	/*!
	 * @brief 改行かEOFを返します
	 */
	bool isBreak() { return ibreak; }

	/*!
	 * @brief 改行かEOFを返します
	 */
	bool isEof() { return ieof;	};

};

#endif /* PROBABILITYPARSE_H_ */
