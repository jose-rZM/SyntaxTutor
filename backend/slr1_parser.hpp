#pragma once

#include <map>
#include <span>
#include <string>
#include <unordered_set>

#include "grammar.hpp"
#include "lr0_item.hpp"
#include "state.hpp"

class SLR1Parser {
  public:
    /**
     * @brief Represents the possible actions in the SLR(1) parsing table.
     *
     * This enumeration defines the types of actions that can be taken by the
     * parser during the parsing process:
     * - `Shift`: Shift the input symbol onto the stack and transition to a new
     * state.
     * - `Reduce`: Reduce a production rule and pop symbols from the stack.
     * - `Accept`: Accept the input as a valid string in the grammar.
     * - `Empty`: No action is defined for the current state and input symbol.
     */
    enum class Action { Shift, Reduce, Accept, Empty };

    /**
     * @brief Represents an action in the SLR(1) parsing table.
     *
     * This struct associates an LR(0) item with an action to be taken by the
     * parser. It is used to store entries in the action table.
     *
     * @var item The LR(0) item associated with the action if it is a reduce
     * action.
     * @var action The type of action to be taken (Shift, Reduce, Accept, or
     * Empty).
     */
    struct s_action {
        const Lr0Item* item;
        Action         action;
    };

    /**
     * @brief Represents the action table for the SLR(1) parser.
     *
     * The action table is a map that associates each state and input symbol
     * with a specific action (`Shift`, `Reduce`, `Accept`, or `Empty`). It is
     * used to determine the parser's behavior during the parsing process.
     *
     * The table is structured as:
     * - Outer map: Keys are state IDs (unsigned int).
     * - Inner map: Keys are input symbols (std::string), and values are
     * `s_action` structs representing the action to take.
     */
    using action_table =
        std::map<unsigned int, std::map<std::string, SLR1Parser::s_action>>;

    /**
     * @brief Represents the transition table for the SLR(1) parser.
     *
     * The transition table is a map that associates each state and symbol with
     * the next state to transition to. It is used to guide the parser's state
     * transitions during the parsing process.
     *
     * The table is structured as:
     * - Outer map: Keys are state IDs (unsigned int).
     * - Inner map: Keys are symbols (std::string), and values are the next
     * state IDs (unsigned int).
     */
    using transition_table =
        std::map<unsigned int, std::map<std::string, unsigned int>>;

    SLR1Parser() = default;
    SLR1Parser(Grammar gr);

    /**
     * @brief Retrieves all LR(0) items in the grammar.
     *
     * This function returns a set of all LR(0) items derived from the grammar's
     * productions. Each LR(0) item represents a production with a marker
     * indicating the current position in the production (e.g., A → α•β).
     *
     * @return A set of all LR(0) items in the grammar.
     */
    std::unordered_set<Lr0Item> AllItems() const;

    /**
     * @brief Prints debug information about the parser's states.
     *
     * This function outputs detailed information about the states in the
     * parser's state machine, including the LR(0) items in each state. It is
     * used for debugging and verifying the correctness of the state machine.
     */
    void DebugStates() const;

    /**
     * @brief Prints debug information about the parser's action table.
     *
     * This function outputs detailed information about the action table,
     * including shift, reduce, and accept actions for each state and input
     * symbol. It is used for debugging and verifying the correctness of the
     * parsing table.
     */
    void DebugActions();

    /**
     * @brief Computes the closure of a set of LR(0) items.
     *
     * This function computes the closure of a given set of LR(0) items by
     * adding all items that can be derived from the current items using the
     * grammar's productions. The closure operation ensures that all possible
     * derivations are considered when constructing the parser's states.
     *
     * @param items The set of LR(0) items for which to compute the closure.
     */
    void Closure(std::unordered_set<Lr0Item>& items);

    /**
     * @brief Helper function for computing the closure of LR(0) items.
     *
     * This function recursively computes the closure of a set of LR(0) items by
     * adding items derived from non-terminal symbols. It avoids redundant work
     * by tracking visited non-terminals and stopping when no new items are
     * added.
     *
     * @param items The set of LR(0) items being processed.
     * @param size The size of the items set at the start of the current
     * iteration.
     * @param visited A set of non-terminals that have already been processed.
     */
    void ClosureUtil(std::unordered_set<Lr0Item>& items, unsigned int size,
                     std::unordered_set<std::string>& visited);

    std::unordered_set<Lr0Item> Delta(const std::unordered_set<Lr0Item>& items,
                                      const std::string&                 str);
    /**
     * @brief Resolves LR conflicts in a given state.
     *
     * This function attempts to resolve shift/reduce or reduce/reduce conflicts
     * in a given state using SLR(1) parsing rules. It checks the FOLLOW sets of
     * non-terminals to determine the correct action and updates the action
     * table accordingly.
     *
     * @param st The state in which to resolve conflicts.
     * @return `true` if all conflicts are resolved, `false` if an unresolvable
     *         conflict is detected.
     */
    bool SolveLRConflicts(const state& st);

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
     * @brief Creates the initial state of the parser's state machine.
     *
     * This function initializes the starting state of the parser by computing
     * the closure of the initial set of LR(0) items derived from the grammar's
     * start symbol. The initial state is added to the `states_` set, and its
     * transitions are prepared for further processing in the parser
     * construction.
     *
     * @see states_
     * @see transitions_
     */
    void MakeInitialState();

    /**
     * @brief Constructs the SLR(1) parsing tables (action and transition
     * tables).
     *
     * This function builds the SLR(1) parsing tables by computing the canonical
     * collection of LR(0) items, generating the action and transition tables,
     * and resolving conflicts (if any). It returns `true` if the grammar is
     * SLR(1) and the tables are successfully constructed, or `false` if a
     * conflict is detected that cannot be resolved.
     *
     * @return `true` if the parsing tables are successfully constructed,
     * `false` if the grammar is not SLR(1) or a conflict is encountered.
     *
     * @see actions_
     * @see transitions_
     * @see states_
     */
    bool MakeParser();

    void TeachAllItems();
    std::string TeachClosure(std::unordered_set<Lr0Item>& items);
    void TeachClosureUtil(std::unordered_set<Lr0Item>& items, unsigned int size,
                          std::unordered_set<std::string>& visited, int depth, std::ostringstream &output);
    std::string TeachDeltaFunction(const std::unordered_set<Lr0Item>& items,
                            const std::string&                 symbol);
    void TeachCanonicalCollection();
    std::string PrintItems(const std::unordered_set<Lr0Item>& items);

    /// @brief The grammar being processed by the parser.
    Grammar gr_;

    /// @brief Cached FIRST sets for all symbols in the grammar.
    std::unordered_map<std::string, std::unordered_set<std::string>>
        first_sets_;

    /// @brief Cached FOLLOW sets for all non-terminal symbols in the grammar.
    std::unordered_map<std::string, std::unordered_set<std::string>>
        follow_sets_;

    /// @brief The action table used by the parser to determine shift/reduce
    /// actions.
    action_table actions_;

    /// @brief The transition table used by the parser to determine state
    /// transitions.
    transition_table transitions_;

    /// @brief The set of states in the parser's state machine.
    std::unordered_set<state> states_;
};
