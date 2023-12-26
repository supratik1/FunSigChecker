#ifndef EXPRESSIONMANAGER_H
#define EXPRESSIONMANAGER_H

#include <iterator>
#include <algorithm>

#include "DagManager.h"
#include "Expression.h"
#include "ExpressionValue.h"
#include "ExpressionAsDAG.h"
#include "InferTypeOfExpression.h"
#include "helping_functions.h"
#include "Oprs.h"
#include "DAGSimplifier.h"
#include "ExpressionEvaluator.h"
#include "ConfigurationOptions.h"

#define CONSTRAINT_EXPR_NAME "constraint_operator%"

#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <algorithm>

#define EXPRESSION_TYPE DAG_EXPRESSION
/**
   Expression implemented in Dag representation.
   If you want to implement the expression in other format, set EXPRESSION_TYPE to corresponding type.
   e.g. following code will enable expressions represented as strings
   #define STRING_EXPRESSION 2
   #define EXPRESSION_TYPE STRING_EXPRESSION
 * *Note that the number 2 used here is different from the one used for DAG_EXPRESSION.
 *  Make sure that only one definition for EXPRESSION_TYPE
 */

#define CARRY_KEY "carry"
#define RESULT_KEY "result"

class t_ExpressionManager
{
private:
    static t_ExpressionManager *singleton;
    static set<string> toWeaken;
    static void updateWeakeningSet();
    vector<map<string, t_Expression *>> weakenTimeInfo;

    static bool performReplacementForInvalidBit;

public:
    static t_MemoryArrayBoundsInfo *getMemoryVariableDimensions(string memoryVariableName)
    {
        return ::getMemoryVariableDimensions(memoryVariableName);
    }
    static bool isAMemoryVariable(const string &variableName)
    {
        return ::isAMemoryVariable(variableName);
    }

    static bool addMemoryBoundsInfoToMemoryVariableInfo(t_MemoryArrayBoundsInfo *mem_bounds, bool performSLASHReplacement = true)
    {
        return ::addMemoryBoundsInfoToMemoryVariableInfo(mem_bounds, performSLASHReplacement);
    }

    static bool isPerformReplacementForInvalidBit()
    {
        return performReplacementForInvalidBit;
    }

    static void setPerformReplacementForInvalidBit(bool perform)
    {
        performReplacementForInvalidBit = perform;
    }

    // Used as callback for DAG simplifier.
    static t_DAGNode *createDagNodeLevelExpression(const std::string &label, const vector<t_DAGNode *> &children);

    void verifyWeakening()
    {
#ifdef WEAKEN_ONLY_ONCE
        if (!t_ConfigurationOptions::getConfig()->m_weakenFile.empty())
        {
            ofstream file("DidNotWeaken");
            copy(toWeaken.begin(), toWeaken.end(), ostream_iterator<string>(file, "\n"));
            // assert(toWeaken.size() == 0);
        }
#endif
    }

    static t_ExpressionManager *getSingleton()
    {
        return singleton;
    }
    t_Expression *weakenAtom(t_Expression *expr, const string &variable_name, int upper_bound, int lower_bound, int instance_number = -1);
    set<string> generateWeakenListWithTimeInfo(const string &lineSep = "\n", const string &fieldSep = ";");
    t_Expression *isWeakenedAtTime(const string &atomName, int time);

    static t_Expression *getCktExprFromWeakenedExpression(t_Expression *expr);

    static std::string getLabelOfWeakenedExpression(t_Expression *expr)
    {
        t_Expression *e = getCktExprFromWeakenedExpression(expr);
        assert(e != NULL);
        return getSingleton()->getLabelOfExpression(e);
    }

private:
    int m_expressionImplementationType;
    t_Expression *m_dummyExpression;
    expressionImplementationManager *m_expressionImplementationManagerInstance;
    // map<string,string> operatorNameAndSymbolMap;
    t_ExpressionEvaluator *m_evaluator;
    ofstream *m_logFile;
    bool m_useEncodingOfExpressions;

    map<string, t_Expression *> m_mapOfConstraintNameAndConstraintExpressions;

    struct updateData
    {
        t_Expression *index, *value;
        int level;
        TypeOfExpressionTuple type_info;

        updateData(t_Expression *idx, t_Expression *val, int lev, TypeOfExpressionTuple type) : index(idx), value(val), level(lev), type_info(type)
        {
        }

        // sorts descending

        static bool sortOrder(const updateData &d1, const updateData &d2)
        {
            return d1.level > d2.level;
        }
    };

    pair<bool, t_Expression *> buildUpdateMap(t_Expression *root, vector<updateData> &vec_updateData, map<string, t_Expression *> expr_map);

    /**
     * Performs the optimization for update at valid indices. This is the same as the optimization as done in
     * the Expression::createExpression method.<br/>
     * Note: Not currently used as this method cannot handle update at invalid indices.
     * @param updExpr
     * @param ub
     * @param lb
     * @return
     */
    t_Expression *replaceLeavesFromMapForUpdate(t_Expression *expr, map<string, t_Expression *> expr_map);
    /**
     * Generates an expression that equates all the values of the given update/array expression.
     * Given an expression, it compares the values in the array/update expression and returns 1
     * if the expressions are not equal and 0 if the expressions evaluate to equal and valid.
     *
     * FOr example<br/>
     * update( A, i, v) =&gt;<br/>
     * ite expression that evaluates to the invalid bit of 'v' if all values in the array are equal to v,
     * 0 otherwise
     * NOTE: Not used, use createExpressionToEquateAllValuesForUpdateOnly instead
     * @param updExpr the expression whose values are to be equated
     * @param ub
     * @param lb
     * @return ite expression that evaluates to 0 if all values are equal and valid, 1 otherwise.
     */
    t_Expression *createExpressionToEquateAllValues(t_Expression *updExpr, int ub, int lb);

    t_Expression *createExpressionToEquateAllValuesForUpdateOnly(t_Expression *updExpr, int ub, int lb);

    /**
     * Generates the invalid bit of the given read expression.
     * <br/>Note: The prerequisite is that the array_expr be the array expression and not another update expression.
     * @param array_expr the array expression
     * @param readIndex index where the read is to be performed
     * @param ub
     * @param lb
     * @return the invalid bit expression for the read
     */
    t_Expression *generateInvalidBitOfReadOfArrayExpression(t_Expression *array_expr, t_Expression *readIndex, int ub, int lb);
    /**
     * Generates the invalid bit expression for the given read expression.
     * <br/>Note: This only works of the read is performed on the array expression and not an update expression.
     * @param read_expr
     * @param ub
     * @param lb
     * @return
     */
    t_Expression *generateInvalidBitOfReadOfArrayExpression(t_Expression *read_expr, int ub, int lb);

public:
    t_Expression *replaceExpressionInCopy(t_Expression *root, t_Expression *toreplace, t_Expression *replaceby);

    pair<vector<t_MemoryBounds>, int> getBoundsOfArray(t_Expression *array_expr, int &baseWidth);
    /**
     * Gets the memory bounds of the array and the index of the current bound (for multidimensional arrays)
     * Thus, if you pass read(a,i) as the expression, this will return bounds of a and '1' to signify that the
     * bounds of read(a,i) will be the bounds[1].
     *
     */
    pair<vector<t_MemoryBounds>, int> getBoundsOfArray(t_Expression *array_expr)
    {
        int a;
        return getBoundsOfArray(array_expr, a);
    }

    // Constructor
    t_ExpressionManager(t_ConfigurationOptions config_options);

    // new constructor added by Soumyajit on 2013/08/28
    t_ExpressionManager(bool recursion_recompute_signatures = true);

    // Destructor of Expression Manager
    ~t_ExpressionManager();
    t_Expression *createIntegerConstant(int value);
    t_Expression *createIntegerConstant(int value, int width);
    t_Expression *extendExpr(t_Expression *expr, int newWidth);
    // Constant is created as expression
    t_Expression *createConstant(
        string &label,
        TypeOfExpressionTuple &typeTuple);
    // Symbol is created as expression
    t_Expression *createSymbol(
        string &label,
        TypeOfExpressionTuple &typeTuple);
    // Expression with sub-expressions
    t_Expression *createExpression(
        string &label,
        vector<t_Expression *> &operands);

    t_Expression *
    createExpression(string &label, vector<t_Expression *> &operands, TypeOfExpressionTuple &typeTuple);

    t_Expression *ReplaceLeavesByExpressionsFromMap(t_Expression *org_expr, t_HashTable<string, t_Expression *> &expr_map, t_HashTable<unsigned long, t_Expression *> &cache);
    t_Expression *ReplaceLeavesByExpressionsFromMap(t_Expression *org_expr, map<string, t_Expression *> &expr_map);

#ifdef QE // Added by Ajith John on 6 Jan 2014
    t_Expression *ReplaceLeafByExpression(t_Expression *org_expr, string &leaf_label_to_replace, t_Expression *expr_to_replace, t_HashTable<unsigned long, t_Expression *> &cache, bool use_second_level_cache_in_compose, unsigned long long int &first_level_cache_hits, unsigned long long int &first_level_cache_misses, unsigned long long int &second_level_cache_hits, unsigned long long int &second_level_cache_misses, unsigned long long int &leaf_cases, unsigned long long int &node_cases, unsigned long long int &no_recreation_cases, unsigned long long int &recreation_cases);
#endif // Added by Ajith John on 6 Jan 2014 ends here

    // Delete a specified expression
    bool removeExpression(t_Expression *expr);

    // Check for Structural equality based on implementation
    bool checkStructuralEquality(
        t_Expression *e1,
        t_Expression *e2);

    // Get size of the nodes in particular expression
    int getSizeOfExpression(t_Expression *expr);

    TypeOfExpressionTuple *inferTypeOfExpression(
        string operatorSymbol,
        vector<t_Expression *> &operands);

    // It will have the operands to be casted and cast those operands and
    // Generate an expression type new operands that are type casted as specified.
    TypeOfExpressionTuple *inferTypeBasedOnCastingOperands(
        string &operands_numbers_to_cast,
        string &operatorSymbol,
        vector<t_Expression *> &operands);
    // print Expression in constraint format (yices)
    bool printExpressionAsYicesExpression(
        t_Expression *expr,
        string &expression_symbol_to_print, ofstream *fout);

    // print Expression into a file
    bool printExpressionToFile(
        t_Expression *expr,
        ofstream *outfile);

    // build Expression back from a file
    bool buildExpressionFromFile(ifstream *infile);

    // print expression in tree format to a file
    bool printExpressionToFileAsTree(
        string &expression_symbol,
        t_Expression *expr, ofstream *outfile);

    inline bool resetVisitedFlagsOfAllExpressions()
    {
        return m_dummyExpression->clearVisitedFlagsOfAllExpressions(m_expressionImplementationType, m_expressionImplementationManagerInstance);
    }

    inline bool isVisited(t_Expression *expr); /* {
         return expr != NULL && expr->m_dagExpression->getDag()->isNodeVisited();
     }*/

    inline void setVisited(t_Expression *expr, bool value = true); /* {
         if (expr != NULL)
             expr->m_dagExpression->getDag()->assignVisitedFlag(value);
     }*/
    // Get the leaf expressions of an expression
    vector<t_Expression *> getVectorOfLeafExpressions(t_Expression *expr);
    void getVectorOfLeafExpressionsRecurse(t_Expression *expr, vector<t_Expression *> &leaves, t_HashTable<unsigned long long, bool> &visited);

public:
    vector<t_Expression *> getVectorOfLeafExpressions2(t_Expression *expr);
    // Get the label of a particular expression
    string getLableOfExpression(t_Expression *expr);

    // Evaluate a particular expression
    bool evaluateExpression(t_Expression *expr);

    // Evaluate a vector of given expressions
    //  With expressions leaves already set and valid flag is set only for those expressions that have
    //  proper evaluate value.
    bool evaluateAVectorOfExpressions(vector<t_Expression *> &expression_to_evaluate);

    /**
     * Part of evaluation assign values to leaves
     */
    bool setValuesOfLeaves(map<string, string> &leaf_values, bool clearValidityOfAncestors);

    /**
     * Set value methods for  expressions with differnt types of values
     */
    bool setValue(t_ExpressionValue *exprValue, t_Expression *expr);
    bool setValue(t_Expression *expr, signed int siVal);
    bool setValue(t_Expression *expr, unsigned int usiVal);
    bool setValue(t_Expression *expr, float fval);
    bool setValue(t_Expression *expr, bvatom bvValue);

    /**
     * Shift the actual value of this expression to given expression with the given label
     */
    bool copyValueFromExpressionToLeafWithLabel(string &label, t_Expression *expr);

    /**
     * clears the valid flag of the expressions under given set of expressions and
     * clears the visited flag of all the dagnodes. It will clear all valid flags of nodes including
     * CONSTANT,SYMBOL,OPERATOR nodes
     */
    bool ResetEvaluator(vector<t_Expression *> &expressions_tobe_evaluated);

    /**
     * Get the expression value as a string
     */
    string getActualExpressionValueAsString(t_Expression *expr);

    /** Has to be made private after testing
     * Get the value of an expression
     */
    t_ExpressionValue *getValue(t_Expression *expr);

    int getWidth(t_Expression *expr);

    inline t_TypeOfExpression getTypeOfExpressionWithOutWidth(t_Expression *expr)
    {
        t_ExpressionValue *val = expr->getValue(m_expressionImplementationType, m_expressionImplementationManagerInstance);
        return val->getOnlyTypeOfExpression();
    }

    /**
     * Print the expression Label and the value in that expression
     *   (i.e., actual value,typeInfo,etc)
     */
    bool printValueInExpressionNode(t_Expression *expr);
    t_Expression *copyExpressionUsingSuffix(
        t_Expression *expression_to_copy,
        string &suffix);

    // Replace a leaf of an expression by another expression
    bool replaceLeafOfAExpressionByExpression(
        t_Expression *expression_to_replace,
        t_Expression *expression_replace_by);

    // Simplify expresssion after creating
    bool SimplifyExpression(t_Expression *&expression_to_simplify);

    // bool initializeSignalsToConcreteValues(map<string, string> signal_values);

    // Get the expression with given label, mainly used in searching for the leaf expressions.
    t_Expression *getExpressionWithLabel(string &label);

    // Get the operands of a given expression
    vector<t_Expression *> getVectorOfOperands(t_Expression *expression);

    inline bool isALeafExpression(t_Expression *expression)
    {

        t_DAGNode *dag = expression->m_dagExpression->getDag();

        return (dag->getOutListBeginIterator() == dag->getOutListEndIterator());
        /**
         * vector<t_Expression*> operands_of_current_expr = getVectorOfOperands(expression);
        if (operands_of_current_expr.size() == 0) {
            return true;
        } else {
            return false;
        }*/
    }

    inline bool isANonConcatExpression(t_Expression *expr)
    {
        if (getLabelOfExpression(expr) == m_operatorLabelConcat)
            return false;
        else
            return true;
    }

    // bool addAsIthChild(t_Expression *from, t_Expression *to, int i);

    // bool removeIthChild(t_Expression *from, t_Expression *to, int i);

    // Check for the cycles when creating an edge between two dags.
    bool checkWhetherCreatingEdgeFormsCycle(t_Expression *nodeFrom, t_Expression *nodeTo);

    bool printExpressionToFileAsDAG(const string &expression_symbol, t_Expression *expr, ostream &out, bool printInvBit = true);

    bool printExpressionToFileAsDAG(string &expression_symbol, t_Expression *expr, ofstream *outfile);
    // Set expression evaluator for memories

    // inline bool setExpressionEvaluatorWithMemoryEntries(vector<t_MemoryArrayBoundsInfo *> &MemoryEntries)
    void setExpressionEvaluatorWithMemoryEntries(vector<t_MemoryArrayBoundsInfo *> &MemoryEntries)
    {
        this->m_evaluator->m_MemoryArraysForEvaluation = MemoryEntries; // should be void. no places used.
    }
    // Call garbage collector of expression implementation class
    bool gc();
    // bool printMapOfTypeInferenceRules(ofstream *inf_rules_file);

    bool replaceExpressionRepresentationInEM(t_Expression *old_expression, t_Expression *new_expression);

    inline t_TypeOfValueStoredInExpressionLabel getTypeOfValueStoredInExpressionLabel(t_Expression *expr)
    {
        if (expr == NULL)
        {
            return INVALID;
        }
        t_ExpressionValue *value_in_expr = expr->getValue(m_expressionImplementationType, m_expressionImplementationManagerInstance);
        return value_in_expr->getTypeOfValueStoredInExpressionLabel();
    }

    inline string getLabelOfExpression(t_Expression *expr)
    {
        return expr->getLabelOfExpression(m_expressionImplementationType, m_expressionImplementationManagerInstance);
    }

    inline t_Expression *getMostRecentRestructuredVersion(t_Expression *expr)
    {
        if (expr == NULL)
        {
            return NULL;
        }
        else
        {
            return expr->getMostRecentRestructuredExpression(m_expressionImplementationType, m_expressionImplementationManagerInstance);
        }
    }

    inline TypeOfExpressionTuple getTypeOfExpressionTuple(t_Expression *expr)
    {
        if (expr == NULL)
        {
            TypeOfExpressionTuple typeInfo = {TYPE_NONE, -1};
            return typeInfo;
        }
        else
        {
            t_ExpressionValue *expr_value = getValue(expr);
            TypeOfExpressionTuple typeInfo = {expr_value->getOnlyTypeOfExpression(), expr_value->getWidth()};
            return typeInfo;
        }
    }

    bool setMemoryExpressionBounds(t_Expression *&expression, t_MemoryArrayBoundsInfo &boundsInfo);

    inline int getExpressionID(t_Expression *&expr)
    {
        if (expr == NULL)
        {
            return -1;
        }
        else
        {
            return expr->getID();
        }
    }
    t_Expression *createTwoOperandOperatorFromMultipleOperands(string &operator_label, vector<t_Expression *> &v_operands, TypeOfExpressionTuple &typeInfo);
    t_Expression *createABitVectorConstantZeroExpression();
    t_Expression *createABitVectorConstantOneExpression();
    t_Expression *createOneBitExpressionWithThreeOperands(string &label, t_Expression *operand1, t_Expression *operand2, t_Expression *operand3);
    t_Expression *createOneBitExpressionWithTwoOperands(string &label, t_Expression *operand1, t_Expression *operand2);
    t_Expression *createOneBitExpressionWithOneOperand(string &label, t_Expression *operand1);
    bool isAConstantOneExpression(t_Expression *expr);
    bool isAConstantZeroExpression(t_Expression *expr);

    pair<t_Expression *, bool> simplifyByEvaluatingConstantOpers(t_Expression *expr, t_HashTable<unsigned long long, t_Expression *> &alreadySimplified);
    bool allOperandsConstant(vector<t_Expression *> ops);

    string m_operatorLabelADD;
    string m_operatorLabelSUB;
    string m_operatorLabelMultiply;
    string m_operatorLabelDivide;
    string m_operatorLabelModulus;
    string m_operatorLabelRedOR;
    string m_operatorLabelRedAND;
    string m_operatorLabelRedXOR;
    string m_operatorLabelRedNOR;
    string m_operatorLabelRedNAND;
    string m_operatorLabelRedXNOR;

    string m_operatorLabelLogNOT;
    string m_operatorLabelLogOR;
    string m_operatorLabelLogAND;
    string m_operatorLabelLogicalEquality;
    string m_operatorLabelLogicalInEquality;
    string m_operatorLabelLogicalWildEquality;
    string m_operatorLabelCaseXequlity;
    string m_operatorLabelCaseZequlity;
    string m_operatorLabelLogicalWildInEquality;
    string m_operatorLabelGreaterThan;
    string m_operatorLabelLessThan;
    string m_operatorLabelGreaterThanOrEqual;
    string m_operatorLabelLessThanOrEqual;
    string m_operatorLabelSelect;

    string m_operatorLabelBitwiseNEG;
    string m_operatorLabelBitwiseAND;
    string m_operatorLabelBitwiseOR;
    string m_operatorLabelBitwiseXOR;
    string m_operatorLabelBitwiseXNOR;
    string m_operatorLabelBitwiseNOR;
    string m_operatorLabelBitwiseNAND;

    string m_operatorLabelIte;
    string m_operatorLabelConcat;
    string m_operatorLabelZeroextension;
    string m_operatorLabelSignextension;

    string m_operatorLabelLeftShift;
    string m_operatorLabelRightShift;
    string m_operatorLabelLefttoRightStream;
    string m_operatorLabelRighttoLeftStream;

    TypeOfExpressionTuple m_typeInfoOfInvalidBitExpr;
    t_TypeOfValueStoredInExpressionLabel m_operatorLabelType;

    // Invalid bit related methods:
    string generateInvalidBitManagerKey(t_Expression *expr, int ub, int lb, string type);
    t_Expression *getInvalidBitExpression(t_Expression *expr);
    t_Expression *getInvalidBitExpression(t_Expression *expr, int ub, int lb, string type);
    t_Expression *generateInvalidBitExpressionForBitwiseOperators(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionForUnaryOperators(t_Expression *expr, int ub, int lb);

    bool setInvalidBitExpression(t_Expression *expr, t_Expression *expr_invalidbit);
    bool setInvalidBitExpression(t_Expression *expr, int ub, int lb, string type, t_Expression *expr_invalid_bit);

    t_Expression *generateInvalidBitExpressionOfSelectExpression(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionOnPartOfADDExpression(t_Expression *expr, int ub, int lb, bool return_carry_invalid_bit);
    t_Expression *generateInvalidBitExpressionOnPartOfSUBExpression(t_Expression *expr, int ub, int lb, bool return_carry_invalid_bit);
    t_Expression *generateInvalidBitExpressionOnPartOfMULTExpression(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionOnPartOfDIVExpression(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionForLEFTSHIFTOperators(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionForRIGHTSHIFTOperators(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionForCOMPOperators(t_Expression *expr, int ub, int lb);
    vector<t_Expression *> getAtomsBetweenUbAndLb(t_Expression *expr, int ub, int lb);
    t_Expression *createASelectExpressionOnExpression(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionForRightToLeftStreamOperator(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionForIteOperator(t_Expression *expr, int ub, int lb);
    t_Expression *generateInvalidBitExpressionOnPartOfMODExpression(t_Expression *expr, int ub, int lb);

    t_Expression *generateWeakenedInvalidBit(t_Expression *expr, t_Expression *weakenExpr, t_Expression *anteDrvExpr,
                                             t_Expression *anteInvExpr);

    t_Expression *generateWeakenedInvalidBitFromITE(t_Expression *expr);
    /**
     * This function is simply a helper to call generateInvalidBitOfRead(op[0],op[1]) or generateInvalidBitOfReadOfITE
     * depending upon the operand[0]
     * @param expr read expression
     * @param ub
     * @param lb
     * @return
     */
    t_Expression *generateInvalidBitExpressionOfReadExpression(t_Expression *expr, int ub, int lb);

    /**
     * Main function that actually generates the invalid expression as specified in MEMORY_SIMPLIFY.TXT
     * @param arrayExpr operand[0] of read
     * @param indexExpr operand[1] of read
     * @param ub
     * @param lb
     * @return
     */
    t_Expression *generateInvalidBitExpressionOfReadExpression(t_Expression *arrayExpr, t_Expression *indexExpr, int ub, int lb, TypeOfExpressionTuple &type);
    /**
     * Helper function for generateInvalidBitExpressionOfReadExpression.
     * This function will build the expression for the else part of the read's invalid bit expression,
     * i.e. assuming that the index of the read expression is valid. This function will be recursively calling
     * itself.
     * @param updExpr
     * @param ub
     * @param lb
     * @return
     */
    t_Expression *generateInvalidBitExpressionOfReadOfUpdateExpression(t_Expression *updExpr, t_Expression *readIndex, int ub, int lb);

    /**
     * Helper function to generate the invalid bit for read.
     * This is useful when the read expression is in flops.
     * i.e. read( ite(clk, __P__array, N@array), index)
     * @param arrayExpr
     * @param readIndex
     * @param ub
     * @param lb
     * @param  typeofReadExpr It is used to infer type of read expr while creating read (then) and read(else) part.
     * This is the type of read( ite(cond,then,else)).
     * @return
     */
    t_Expression *generateInvalidBitExpressionOfReadOfITEExpression(t_Expression *arrayExpr, t_Expression *readIndex, int ub, int lb, TypeOfExpressionTuple &typeofReadExpr);

    int getConstantValuePresentInExpression(t_Expression *expr);
    t_Expression *getConstraintFunction(string &operator_label, vector<t_Expression *> &operands_of_expr);

public:
    t_Expression *generateInvalidBitExpression(t_Expression *&expr);
    bool printAllConstraintExpressions(ofstream *file_to_print);

    static vector<t_Expression *> buildVector(t_Expression *first, t_Expression *second = NULL, t_Expression *third = NULL);
    ostream &printExpression(ostream &output, ostream &declarations, t_Expression *expr, map<string, string> &operatorMap, bool printNodeId = true);

    // added by sukanya
    ostream &mod_printExpression(ostream &output, ostream &declarations, t_Expression *expr, map<string, string> &operatorMap, bool printNodeId = true);

private:
    /**
     * Will print the header if the expr is an array
     * @param out
     * @param expr
     * @param prefix
     * @return true if the expr was an array and the declaration was printed
     */
    bool printArrayHeader(ostream &out, t_Expression *expr, const string &prefix);
    ostream &printDagHeader(ostream &out, t_Expression *expr, const string &prefix);
    map<string, string> &generateMap(istream &input, map<string, string> &operatorMap, char fieldSeparator = '=', char entrySeparator = '\n');

public:
    static void printExpressionToCerr(t_Expression *expr);
    enum type_of_operator
    {
        UNKNOWN,
        LOGICAL,
        RELATIONAL,
        BITWISE
    };

    type_of_operator getTypeOfOperator(t_Expression *expr)
    {
        return getTypeOfOperator(getLabelOfExpression(expr));
    }
    type_of_operator getTypeOfOperator(const string &label);
    void clearPrintingSets();
    template <class T>
    ostream &printExpressions(ostream &output, ostream &declarations, const T &exprs, const string &exprSeparator);
    // ostream& printExpressions(ostream&output, ostream& declarations, const vector<t_Expression*> &exprs, const string& exprSeparator);

    // Add a declaration for your type for this in expressionmanager.cc.
    //  WE have already added for vector, set.
    // example:
    // template void t_ExpressionManager::printExpressionsToFileInSMT(const vector<t_Expression*>&, const string&, const string&, const string&, const string&);
    template <class T>
    void printExpressionsToFileInSMT(const T &exprs, const string &exprSeparator, const string &smtFilename, bool negateAll = false, const string &declFilename = "", const string &constraintsFilename = "");

    template <typename T1, typename T2>
    void printExpressionsToFileInSMT(const T1 &exprs, const T2 &inpConstr, const string &exprSeparator, bool negateAll, bool negateConstraints, bool negateConsVac, const string &final, const string &decl = "", const string &cons = "");

    // added by sukanya
    template <class T>
    void mod_printExpressionsToFileInSMT(const T &exprs, const string &exprSeparator, const string &smtFilename, bool negateAll = false, const string &declFilename = "", const string &constraintsFilename = "");
    template <typename T1, typename T2>
    void mod_printExpressionsToFileInSMT(const T1 &exprs, const T2 &inpConstr, const string &exprSeparator, bool negateAll, bool negateConstraints, bool negateConsVac, const string &final, const string &decl = "", const string &cons = "");

    /**
     * Extracts the actual signal name by removing the N@, __P__ prefix and the ub,lb,and instance number suffizes
     * @param label the label name
     * @param removeBounds removes the bounds of from signal name if true
     * @param removeInstanceNum removes instance number from signal if true
     * @return the actual signal name
     */
    static string getActualSignalName(string label, bool removeBounds = true, bool removeInstanceNum = true, bool removeInternalPrefix = true);

    pair<t_Expression *, t_Expression *> getNewSymbolLimitedToBound(int bound);

    // added by sai krishna
    void printExpressionsToFileInCNF(vector<t_Expression *> &exprs, string filename, int &idx, int &clauses, map<t_Expression *, pair<string, int>> &getCNFIndexMap);
    void printExpressionInCNF(t_Expression *expr, ofstream &fout, int &idx, int &clauses, map<t_Expression *, pair<string, int>> &getCNFIndexMap, ofstream &em_log);
    void printExpressionInCNF_iterative(t_Expression *expr, ofstream &fout, int &idx, int &clauses, map<t_Expression *, pair<string, int>> &getCNFIndexMap, ofstream &em_log);
    int tim;
    map<unsigned long long, int> exprID_cnfID;
    void print_expr_var_cnf_id(map<t_Expression *, pair<string, int>> &getCNFIndexMap);
    void printExpressionInCNFefficient(t_Expression *expr, string filename, int &idx, int &clauses, map<t_Expression *, pair<string, int>> &getCNFIndexMap, ofstream &em_log);
};

#endif /*EXPRESSIONMANAGER_H*/
