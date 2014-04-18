//============================================================================
// Name        : CompositeBase.h
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "CompositeNode.h"

/*!
 * @brief BayesianNetwork全体に関わる処理、及びUI部分を受け持ちます
 */
class CompositeBase {

private:
	/*!
	 * @brief BN構造と情報を保持した確率を必須引数とします
	 */
	CompositeBase();

public:
	/*!
	 * @brief BN構造とCPT引数を元にBN処理を行います
	 * @param[in] CompositeParse*  BN構造であるXML-Document
	 * @param[in] ProbabilityBase* データを保持した確率処理(CPTを提供)
	 */
	CompositeBase(ProbabilityBase *vfile, string relations);

public:
	/*!
	 * @brief 構造定義ファイル名を保持します
	 */
	string relations;

	/*!
	 * @brief BayesianNetwork用全ノードを保持します
	 */
    NODES nodes;

	/*!
	 * @brief BayesianNetworkの最大階層数を保持します
	 */
	int maxDepth;

protected:
	/*!
	 * @brief データファイル名(実データ)を保持します
	 */
	ProbabilityBase *vfile;

protected:
    /*!
     * @brief BayesianNetwokを作成します
     * @return 0=正常終了
     */
    int invoke();

	/*!
	 * @brief BIFをBN構造CSVに変換します
     * @return 0=正常終了
	 */
	int convertBifToCsv();

	/*!
	 * @brief BayesianNetworkを構造情報を元に構築します
     * @return 0=正常終了
	 */
	int createNetwork();

	/*!
	 * @brief 構築されたBayesianNetworkから各ノードの階層レベルを定義します
	 * @param[in] int 			  現在の階層レベル
	 * @param[in] CompositeNode* 次の処理となるノードの参照
     * @return 0=正常終了
	 */
	int createDepth(int depth, CompositeNode *node);

    /*
     * @brief BayesianNetworkに初期値を設定します
     * @return 0=正常終了
     */
	int format();

public:
	/*!
	 * @brief 指定ノードの指定要素にエビデンスを与えます
	 * @param[in] string 対象とするノード名
	 * @param[in] string 対象とするノード名の要素
     * @return 0=正常終了
	 */
	int setProb(string targetn, string targets);

	/*!
	 * @brief BPを用いた推定（又は事後）確率を計算します
	 * @param[in] string 確率伝播の起点とするノード名
     * @return 0=正常終了
	 */
	int calProbs(string targetn);

	/*!
	 * @brief BPを用いた推定（又は事後）確率を返します（全ノードの指定状態の確率を返します）
	 * @param[in] string 対象とするノード名
	 * @param[in] string 対象ノードの確率
     * @return 0=正常終了
	 */
	int getProb(string targets, PROBS *probs);

	/*!
	 * @brief 現在の状態をログ出力します
	 */
	int now(string title);

};



