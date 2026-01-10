# Test Cycle: LL(1) Tutor

## Scope
- Functional, end-to-end flow of the LL(1) tutor UI.
- Covers all tutor states, error handling, table submission, and PDF export.
- Backend correctness (FIRST/FOLLOW/LL1 table generation) is out of scope.

## Global Preconditions
- App launches successfully.
- Language can be ES or EN (tests are language-agnostic; use the question intent).
- Start from the main window.

## Input Formats (use exactly)
- Table size: `rows,columns` (comma-separated integers).
- Set answers: `a,b,c` (comma-separated, order does not matter, spaces optional).
- LL(1) table cells: use the exact symbols shown in the grammar.

## Test Cases

### LL1-TC-01 - Launch LL(1) session
Preconditions:
- Main window open.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Click the `LL(1)` button. | LL(1) tutor window opens; grammar is shown; first question is shown (State A). |

---

### LL1-TC-02 - State A error path (A -> A1 -> A2 -> A')
Preconditions:
- LL(1) tutor open at State A (question about table size).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter an incorrect `rows,columns` (e.g., `1,1`) and submit. | Wrong counter increments; feedback shown; next question is A1 (number of non-terminals). |
| 2 | At A1, enter an incorrect value and submit. | Wrong counter increments; question stays at A1. |
| 3 | At A1, enter the correct non-terminal count and submit. | Right counter increments; next question is A2 (number of terminals). |
| 4 | At A2, enter an incorrect value and submit. | Wrong counter increments; question stays at A2. |
| 5 | At A2, enter the correct terminal count and submit. | Right counter increments; next question is A' (table size again). |
| 6 | At A', enter an incorrect `rows,columns` and submit. | Wrong counter increments; feedback shown; flow advances to State B (prediction symbols). |

Notes:
- The correct counts must be computed from the grammar shown on the right panel.

---

### LL1-TC-03 - State A correct path (skip A1/A2)
Preconditions:
- Start a new LL(1) session (State A).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter the correct `rows,columns` and submit. | Right counter increments; flow advances directly to State B (skips A1/A2). |

---

### LL1-TC-04 - State B full path (B -> B1 -> B2 -> B')
Preconditions:
- LL(1) tutor in State B for a rule whose antecedent is NOT the axiom (S).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | At B, enter an incorrect SD set and submit. | Wrong counter increments; next question is B1 (FIRST/CAB). |
| 2 | At B1, enter an incorrect set and submit. | Wrong counter increments; question stays at B1. |
| 3 | At B1, enter the correct CAB set and submit. | Right counter increments; next question is B2 (FOLLOW/SIG). |
| 4 | At B2, enter an incorrect set and submit. | Wrong counter increments; question stays at B2. |
| 5 | At B2, enter the correct SIG set and submit. | Right counter increments; next question is B' (SD again). |
| 6 | At B', enter an incorrect SD set and submit. | Wrong counter increments; feedback shown; flow advances to the next rule (State B) or to State C if this was the last rule. |

Notes:
- SD/CAB/SIG are computed from the grammar shown and the LL(1) rules.

---

### LL1-TC-05 - State B axiom branch (B1 -> B')
Preconditions:
- LL(1) tutor in State B for a rule whose antecedent IS the axiom (S).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | At B, enter an incorrect SD set and submit. | Wrong counter increments; next question is B1 (CAB). |
| 2 | At B1, enter the correct CAB set and submit. | Right counter increments; State B2 is skipped; next question is B' (SD again). |
| 3 | At B', enter a correct SD set and submit. | Right counter increments; flow advances to next rule or to State C. |

---

### LL1-TC-06 - State B direct path (correct SD)
Preconditions:
- LL(1) tutor in State B for any rule.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter the correct SD set and submit. | Right counter increments; flow advances to next rule or to State C (no B1/B2). |

Notes:
- If the grammar has only one rule and you already used the B1/B2 path, restart the tutor to cover this direct path.

---

### LL1-TC-07 - State C table correct path + PDF export (Yes/No)
Preconditions:
- Reached State C (table input dialog opens).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Fill the LL(1) table correctly and click `Finalizar`. | Table is accepted; tutor reaches final state; export prompt appears. |
| 2 | Select `Yes` to export, choose a path, and confirm. | PDF is created at the selected path and is non-empty. |
| 3 | Start a new LL(1) session, reach final state again. | Export prompt appears again. |
| 4 | Select `No` to export. | Tutor closes without creating a new PDF. |

---

### LL1-TC-08 - State C wrong attempts and retry limit (C -> C')
Preconditions:
- Reached State C (table dialog open).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Submit an incorrect table (Attempt 1). | Cells are highlighted in red; info message shown. |
| 2 | Submit an incorrect table (Attempt 2). | Cells are highlighted; info message shown. |
| 3 | Submit an incorrect table (Attempt 3). | Cells are highlighted; info message shown. |
| 4 | Submit an incorrect table (Attempt 4). | Generic retry message shown; no cell highlights required. |
| 5 | Submit an incorrect table (Attempt 5). | Dialog closes; flow moves to State C' (table shown again). |
| 6 | In C', submit an incorrect table. | Wrong counter increments; table is shown again (C' repeats). |
| 7 | In C', submit a correct table. | Tutor reaches final state and shows export prompt. |

Notes:
- Attempts are counted globally for the session. The UI should move to C' when the max attempts threshold is reached.

---

### LL1-TC-09 - Cancel LL(1) table dialog
Preconditions:
- Reached State C or C' (table dialog open).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Close the table dialog (window X). | Confirmation dialog appears. |
| 2 | Select `No`. | Table dialog reopens and flow continues. |
| 3 | Close the table dialog again; select `Yes`. | Tutor window closes. |
