//============================================================================
// Name        : Define.h
// Version     : 1.0
// Date        : 2010/03/16
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#ifndef DEFINE_H_
#define DEFINE_H_

//----------------------------------------------------------------------------
// 依存ライブラリを指定します、基本的にSTLのみを利用します
//----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

using namespace std;
class CompositeNode;

//----------------------------------------------------------------------------
// 名前簡潔の為のエイリアス定義を行います
//----------------------------------------------------------------------------
/*! @brief 確率の浮動小数点精度を定義します */
typedef double UD;

/*! @brief 文字列集合のpairを定義します */
typedef pair<string, string> COND_PAIR;

/*! @brief 条件付き確率の条件部分の型を定義します */
typedef vector<COND_PAIR> COND;

/*! @brief 文字列集合のエイリアスを定義します */
typedef vector<string> CHARS;

/*! @brief 1行に相当するキーとタプルの関係を定義します */
typedef map<string, string> LINE;

/*! @brief 列名と実データの関係を定義します */
typedef map<string, CHARS*> VALUES;

/*! @brief VALUESのpairを定義します */
typedef pair<string, CHARS*> VALUES_PAIR;

/*! @brief 確率と実数値の関係を定義します、これはP(A=a1|B=b1)=0.5の関係に等しいです */
typedef map<string, UD>	PROBS;

/*! @brief PROBSのpairを定義します */
typedef pair<string, UD> PROBS_PAIR;

/*! @brief ノードの親子関係を保持する型を定義します */
typedef map<string, vector<string>* > RELATES;

/*! @brief RELATESのpairを定義します */
typedef pair<string, vector<string>* > RELATES_PAIR;

/*! @brief ソートを行う為のpairを定義します（その１：対象のデータ型が少し異なります） */
typedef pair<UD, string> PSORT;

/*! @brief ソートを行う為のpairを定義します（その２：対象のデータ型が少し異なります） */
typedef pair<string, UD> SSORT;

/*! @brief 確率集合の代理名を定義します（順序あり） */
typedef vector<SSORT> PROBO;

/*! @brief ノード集合の代理名を定義します */
typedef map<string, CompositeNode*> NODES;

/*! @brief 頻度を保持する代理名を定義します */
typedef map<string, vector<int>* > FREQ;

//----------------------------------------------------------------------------
// 本ソフトウェア特有の定義、ファイル名などを指定します
//----------------------------------------------------------------------------
/*! @brief 論理上のルートノード名を定義します */
#define ROOT_NODE string("Composite")

/*! @brief ノード親子関係定義ファイル名を定義します */
#define RELATION_FILE "Nodes.csv"

//----------------------------------------------------------------------------
// 本ソフトウェア特有の静的共通処理を定義します
//----------------------------------------------------------------------------
/*!
 * @brief 現在時刻をマイクロ秒単位で取得します
 * @return double 現在時刻(ミリ秒)
 */
inline double nowtime() {
	// 現在経過マイクロ秒を取得して返します(Linux用)
    /*
	struct timeval nowtime;
    char now[64];
    gettimeofday(&nowtime, NULL);
    sprintf(now, "%d.%06d", (int)nowtime.tv_sec, (int)nowtime.tv_usec);
    return strtod(now, NULL);
    */
	return 0.0;
}

/*!
 * @brief ソート手順（降順）を定義します
 */
struct DSORT {
    bool operator()(const PSORT& x, const PSORT& y) const {
        return x.first > y.first;
    }
};

/*!
 * @brief ログ出力用に線を引きます
 */
#define LINE(x) { for (int i = 0; i < 70; i++) { cout << x; } cout << endl; }

#endif /* DEFINE_H_ */
