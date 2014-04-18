//============================================================================
// Name	       : CompositeNode.h
// Version     : 1.0
// Description : Bayesian Network Processing in C++, Ansi-style
//============================================================================
#include "ProbabilityBase.h"

/*!
 * @brief Bayesian Network上の確率変数のノードを定義します
 */
class CompositeNode {
    friend class CompositeBase;

private:
    /*!
     * @brief デフォルトコンストラクタは公開しません
     */
    CompositeNode();

public:
    /*!
     * @brief ノードの名前と全データ処理への参照を必須引数とします
     * @param[in] string          本ノードの名前
     * @param[in] ProbabilityBase 全データを元にした条件付き確率処理
     */
    CompositeNode(string name, ProbabilityBase *cpt);

private:
    /*!
     * @brief 確率への参照を保持します
     */
    ProbabilityBase *cpt;

public:
    /*!
     * @brief 本ノード名を保持します
     */
    string name;

    /*!
     * @brief 一意な要素名を保持します
     */
    CHARS elements;

    /*!
     * @brief 親を保持します
     */
    NODES parents;

    /*!
     * @brief 子を保持します
     */
    NODES children;

    /*!
     * @brief 事前確率を保持します
     */
	PROBS prior;

	/*!
	 * @brief πエビデンス(π(X|e+)...親ノードを元にした証拠)を保持します
	 */
	PROBS eviPai;

	/*!
	 * @brief λエビデンス(λ(X|e-)...子ノードを元にした証拠)を保持します
	 */
	PROBS eviLambda;

	/*!
	 * @brief πメッセージ(πX(Ui|e+)...U→親ノード)を保持します
	 */
	PROBS msgPai;

	/*!
	 * @brief λメッセージ(λVi(X|e-)...V→子ノード)を保持します
	 */
	PROBS msgLambda;

	/*!
	 * @brief 事後確率（α*π(X)λ(X))を保持します
	 */
	PROBS posterior;

    /*!
     * @brief BayesianNetwork上での階層レベルを保持します
     */
    int depth;

	/*!
	 * @brief メッセージ受信可能状態を保持します
	 */
	bool recv;

public:
    /*!
     * @brief 親要素を保持します
     * @param[in] CompositeNode* 親要素/Parents' element(node)
     */
    int addParent(CompositeNode *node);

    /*!
     * @brief 子要素を保持します
     * @param[in] CompositeNode* 子要素/Child element(Node)
     */
    int addChild(CompositeNode *node);

    /*!
	 * @brief 関連するノードに階層構造を利用してメッセージ転送します
	 * @param[in] CompositeNode* 送信ノードへの参照
	 */
	int transMessage(CompositeNode *sender);

	/*!
	 * @brief 現在の情報をログ出力します
	 */
	int now(string title);

protected:
    /*!
	 * @brief 該当する事前確率を返します
	 * @param[in]  string  対象要素名
	 * @param[out] double* 該当事前確率
     */
    int calPrior(string target, UD *result);

	/*!
	 * @brief πメッセージを計算します/πメッセージ及びλメッセージは直接対象ノードに代入します
	 * @param[in] string 送信ノード名
	 */
	int calMsgPai(string sender);

	/*!
	 * @brief λメッセージを計算します
	 * @param[in] string 送信ノード名
	 */
	int calMsgLambda(string sender);

	/*!
	 * @brief πエビデンスを計算します
	 */
	int calEviPai();

	/*!
	 * @brief λエビデンスを計算します
	 */
	int calEviLambda();

	/*!
	 * @brief 事後確率を更新します
	 */
	int calProb();

	/*!
	 * @brief 条件付き確率を求めます
	 * @param[in]  string 対象親ノード名
	 * @param[in]  string 対象ノードの状態(要素)名
	 * @param[in]  string 対象子ノード名
	 * @param[out] UD*    P(子ノード=子ノード状態|親ノード=親ノード状態)を返します
	 */
	int calCpt(string parent, string statep, string statec, UD *result);

	/*!
	 * @brief 条件付き確率P(X|Y1,...,Yn)を求めます（実処理）
	 * @param[in]  COND*   			条件付き確率の条件
	 * @param[out] PROBS*  			算出条件付き確率∑n{P(CN=CE|PN1=PE1,...,PNn=PEn)}を保持します
     * @param[in]  NODES::iterator  処理開始ポインタ・その１
     * @param[in]  NODES::iterator  処理開始ポインタ・その２
	 * @param[in]  bool    初回処理時のみポインタ移動を行いません
	 */
	int calCptPai(COND *cond, PROBS *result, NODES::iterator targetpn, NODES::iterator targetpe, bool first);

	/*!
	 * @brief 条件付き確率P(X|Y1,...,Yn)を求めます（実処理）
	 * @param[in]  NODES*			処理対象とする親ノード(親が複数なら対象親以外の合計、1なら対象の親のデータを用いる)
	 * @param[in]  COND*   			条件付き確率の条件
	 * @param[out] PROBS*  			算出条件付き確率∑n{P(CN=CE|PN1=PE1,...,PNn=PEn)}を保持します
     * @param[in]  NODES::iterator  処理開始ポインタ・その１
     * @param[in]  NODES::iterator  処理開始ポインタ・その２
	 * @param[in]  bool    初回処理時のみポインタ移動を行いません
	 */
	int calCptLambda(string parents, bool pai, COND *cond, PROBS *result, NODES::iterator targetpn, NODES::iterator targetpe, bool first);

	/*
	 * 親ノードの状態毎の事前確率の積算を返します
	 * @param[in] COND*   条件付き確率の条件
	 * @param[in] double* 算出結果格納変数
	 */
	int calPaiTotal(COND *cond, double *result);

	/*!
	 * @brief 正規化定数を計算します
	 * @param[out] UD* 正規化係数を返します
	 */
	int calNormal(UD *result);

};



