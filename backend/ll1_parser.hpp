#pragma once
#include "grammar.hpp"
#include <span>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class LL1Parser {

    /**
     * @brief Represents the parsing table for an LL(1) parser.
     *
     * The LL(1) parsing table is a nested unordered map that associates each
     * non-terminal symbol and input symbol with a list of productions to apply
     * during parsing. It is used to guide the LL(1) parser's decisions based on
     * the current non-terminal and the next input symbol.
     *
     * The table is structured as:
     * - Outer map: Keys are non-terminal symbols (std::string).
     * - Inner map: Keys are input symbols (std::string), and values are vectors
     *   of productions (std::vector<production>) that can be applied.
     *
     * @see production
     */
    using ll1_table = std::unordered_map<
        std::string, std::unordered_map<std::string, std::vector<production>>>;

  public:
    LL1Parser() = default;
    /**
     * @brief Constructs an LL1Parser with a grammar object and an input file.
     *
     * @param gr Grammar object to parse with
     */
    LL1Parser(Grammar gr);

    /**
     * @brief Creates the LL(1) parsing table for the grammar.
     *
     * This function constructs the LL(1) parsing table by iterating over each
     * production in the grammar and determining the appropriate cells for each
     * non-terminal and director symbol (prediction symbol) combination. If the
     * grammar is LL(1) compatible, each cell will contain at most one
     * production, indicating no conflicts. If conflicts are found, the function
     * will return `false`, signaling that the grammar is not LL(1).
     *
     * - For each production rule `A -> α`, the function calculates the director
     * symbols using the `director_symbols` function.
     * - It then fills the parsing table at the cell corresponding to the
     * non-terminal `A` and each director symbol in the set.
     * - If a cell already contains a production, this indicates a conflict,
     * meaning the grammar is not LL(1).
     *
     * @return `true` if the table is created successfully, indicating the
     * grammar is LL(1) compatible; `false` if any conflicts are detected,
     * showing that the grammar does not meet LL(1) requirements.
     */
    bool CreateLL1Table();

    void PrintTable();

    /**
     * @brief Calculates the FIRST set for a given production rule in a grammar.
     *
     * The FIRST set of a production rule contains all terminal symbols that can
     * appear at the beginning of any string derived from that rule. If the rule
     * can derive the empty string (epsilon), epsilon is included in the FIRST
     * set.
     *
     * This function computes the FIRST set by examining each symbol in the
     * production rule:
     * - If a terminal symbol is encountered, it is added directly to the FIRST
     * set, as it is the starting symbol of some derivation.
     * - If a non-terminal symbol is encountered, its FIRST set is recursively
     * computed and added to the result, excluding epsilon unless it is followed
     * by another symbol that could also lead to epsilon.
     * - If the entire rule could derive epsilon (i.e., each symbol in the rule
     * can derive epsilon), then epsilon is added to the FIRST set.
     *
     * @param rule A span of strings representing the production rule for which
     * to compute the FIRST set. Each string in the span is a symbol (either
     * terminal or non-terminal).
     * @param result A reference to an unordered set of strings where the
     * computed FIRST set will be stored. The set will contain all terminal
     * symbols that can start derivations of the rule, and possibly epsilon if
     * the rule can derive an empty string.
     */
    void First(std::span<const std::string>     rule,
               std::unordered_set<std::string>& result);

    /**
     * @brief Computes the FIRST sets for all non-terminal symbols in the
     * grammar.
     *
     * This function calculates the FIRST set for each non-terminal symbol in
     * the grammar by iteratively applying a least fixed-point algorithm. This
     * approach ensures that the FIRST sets are fully populated by repeatedly
     * expanding and updating the sets until no further changes occur (i.e., a
     * fixed-point is reached).
     */
    void ComputeFirstSets();

    /**
     * @brief Computes the FOLLOW sets for all non-terminal symbols in the
     * grammar.
     *
     * The FOLLOW set of a non-terminal symbol A contains all terminal symbols
     * that can appear immediately after A in any sentential form derived from
     * the grammar's start symbol. Additionally, if A can be the last symbol in
     * a derivation, the end-of-input marker (`$`) is included in its FOLLOW
     * set.
     *
     * This function computes the FOLLOW sets using the following rules:
     * 1. Initialize FOLLOW(S) = { $ }, where S is the start symbol.
     * 2. For each production rule of the form A → αBβ:
     *    - Add FIRST(β) (excluding ε) to FOLLOW(B).
     *    - If ε ∈ FIRST(β), add FOLLOW(A) to FOLLOW(B).
     * 3. Repeat step 2 until no changes occur in any FOLLOW set.
     *
     * The computed FOLLOW sets are cached in the `follow_sets_` member variable
     * for later use by the parser.
     *
     * @note This function assumes that the FIRST sets for all symbols have
     * already been computed and are available in the `first_sets_` member
     * variable.
     *
     * @see First
     * @see follow_sets_
     */
    void ComputeFollowSets();

    /**
     * @brief Computes the FOLLOW set for a given non-terminal symbol in the
     * grammar.
     *
     * The FOLLOW set for a non-terminal symbol includes all symbols that can
     * appear immediately to the right of that symbol in any derivation, as well
     * as any end-of-input markers if the symbol can appear at the end of
     * derivations. FOLLOW sets are used in LL(1) parsing table construction to
     * determine possible continuations after a non-terminal.
     *
     * @param arg Non-terminal symbol for which to compute the FOLLOW set.
     * @return An unordered set of strings containing symbols that form the
     * FOLLOW set for `arg`.
     */
    std::unordered_set<std::string> Follow(const std::string& arg);

    /**
     * @brief Computes the prediction symbols for a given
     * production rule.
     *
     * The prediction symbols for a rule,
     * determine the set of input symbols that can trigger this rule in the
     * parsing table. This function calculates the prediction symbols based on
     * the FIRST set of the consequent and, if epsilon (the empty symbol) is in
     * the FIRST set, also includes the FOLLOW set of the antecedent.
     *
     * - If the FIRST set of the consequent does not contain epsilon, the
     * prediction symbols are simply the FIRST symbols of the consequent.
     * - If the FIRST set of the consequent contains epsilon, the prediction
     * symbols are computed as (FIRST(consequent) - {epsilon}) ∪
     * FOLLOW(antecedent).
     *
     * @param antecedent The left-hand side non-terminal symbol of the rule.
     * @param consequent A vector of symbols on the right-hand side of the rule
     * (production body).
     * @return An unordered set of strings containing the prediction symbols for
     * the specified rule.
     */
    std::unordered_set<std::string>
    PredictionSymbols(const std::string&              antecedent,
                      const std::vector<std::string>& consequent);

    std::string TeachFirst(const std::vector<std::string>& symbols);
    void TeachFirstUtil(const std::vector<std::string>&  symbols,
                        std::unordered_set<std::string>& first_set, int depth,
                        std::unordered_set<std::string>& processing, std::ostringstream& output);
    std::string TeachFollow(const std::string& non_terminal);
    std::string TeachPredictionSymbols(const std::string& antecedent,
                                const production&  consequent);

    std::string TeachLL1Table();

    /// @brief The LL(1) parsing table, mapping non-terminals and terminals to
    /// productions.
    ll1_table ll1_t_;

    /// @brief Grammar object associated with this parser.
    Grammar gr_;

    /// @brief FIRST sets for each non-terminal in the grammar.
    std::unordered_map<std::string, std::unordered_set<std::string>>
        first_sets_;

    /// @brief FOLLOW sets for each non-terminal in the grammar.
    std::unordered_map<std::string, std::unordered_set<std::string>>
        follow_sets_;
};
