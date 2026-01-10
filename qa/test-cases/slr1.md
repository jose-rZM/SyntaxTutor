# Test Cycle: SLR(1) Tutor

## Scope
- Functional, end-to-end flow of the SLR(1) tutor UI.
- Covers all tutor states, error handling, table submission, wizard recovery, and PDF export.
- Backend correctness (LR(0) construction, FOLLOW sets, table generation) is out of scope.

## Global Preconditions
- App launches successfully.
- Language can be ES or EN (tests are language-agnostic; follow the question intent).
- Start from the main window.

## Input Formats (use exactly)
- LR(0) items (one per line): `A -> a.b` or `A -> .b` or `A -> EPSILON.`.
  - Use `.` (dot), not the centered dot shown in the UI.
  - Use `->` (ASCII), not the arrow symbol shown in the UI.
- Rule lines (State A3, one per line): `A -> a b c` (spaces optional).
- Sets of symbols: `a,b,c` (comma-separated, order does not matter).
- State ID lists: `1,3,7` (comma-separated integers).
- Completed items per state (State E2): `id:n, id:n` (comma-separated pairs).
- Table size: `rows,columns`.
- Table cells (State H): `sX`, `rX`, `acc`, or `X` for goto on non-terminals.
- For CB when the symbol shown is EPSILON, leave the response empty.
- For CB with a non-EPSILON symbol, the expected input is the set of LR(0) items of the target state (not the state ID).

## Test Cases

### SLR1-TC-01 - Launch SLR(1) session
Preconditions:
- Main window open.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Click the `SLR(1)` button. | SLR(1) tutor window opens; grammar is shown; first question is shown (State A). |

---

### SLR1-TC-02 - State A correct path (A -> B)
Preconditions:
- SLR(1) tutor open at State A.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter the correct initial LR(0) items (I0) and submit. | Right counter increments; State B question appears; progress panel shows I0. |

Notes:
- Use the grammar panel to compute I0 (closure of the axiom item).

---

### SLR1-TC-03 - State A error path (A -> A1 -> A2 -> A3 -> A4 -> A')
Preconditions:
- Start a new SLR(1) session (State A).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter incorrect I0 items and submit. | Wrong counter increments; State A1 question appears (axiom). |
| 2 | At A1, enter an incorrect axiom and submit. | Wrong counter increments; stays at A1. |
| 3 | At A1, enter the correct axiom and submit. | Right counter increments; State A2 question appears. |
| 4 | At A2, enter an incorrect symbol and submit. | Wrong counter increments; stays at A2. |
| 5 | At A2, enter the correct symbol and submit. | Right counter increments; State A3 question appears. |
| 6 | At A3, enter incorrect rules and submit. | Wrong counter increments; stays at A3. |
| 7 | At A3, enter the correct rules and submit. | Right counter increments; State A4 question appears. |
| 8 | At A4, enter incorrect closure items and submit. | Wrong counter increments; stays at A4. |
| 9 | At A4, enter the correct closure items and submit. | Right counter increments; State A' question appears. |
| 10 | At A', enter incorrect items and submit. | Wrong counter increments; flow still advances to State B; progress panel shows I0. |

Notes:
- A' uses the same item format as A.

---

### SLR1-TC-04 - State B and C incorrect answers still advance (B -> C -> CA)
Preconditions:
- SLR(1) tutor at State B with at least one pending state in the queue (I0 is present).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | At B, enter an incorrect number of states and submit. | Wrong counter increments; State C question appears for some state I#. |
| 2 | At C, enter an incorrect item count and submit. | Wrong counter increments; State CA question appears (symbols after dot). |

Notes:
- This validates the current behavior: B and C advance even on incorrect answers.

---

### SLR1-TC-05 - State CA incorrect then correct (CA loop)
Preconditions:
- SLR(1) tutor at State CA for a state I#.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter an incorrect symbol set (missing one or with duplicates) and submit. | Wrong counter increments; stays at CA. |
| 2 | Enter the correct symbol set and submit. | Right counter increments; State CB question appears for the first symbol. |

Notes:
- Include `$` if an accepting item is present in the state.
- Duplicates are rejected at CA.

---

### SLR1-TC-06 - State CB with EPSILON symbol (empty required)
Preconditions:
- State CB question shows the EPSILON symbol.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter any non-empty response and submit. | Wrong counter increments; flow advances to the next CB symbol (or back to B if it was the last). |
| 2 | Restart and reach a CB step where the symbol is EPSILON again. Leave the response empty and submit. | Right counter increments; flow advances to the next CB symbol (or back to B if it was the last). |

Notes:
- If your grammar has no EPSILON in CA, this case is not applicable.

---

### SLR1-TC-07 - State CB with non-EPSILON symbol (items required)
Preconditions:
- State CB question shows a non-EPSILON symbol.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter an incorrect response (e.g., only the state number) and submit. | Wrong counter increments; flow advances to the next CB symbol. |
| 2 | At the next CB symbol, enter the correct LR(0) item set for the target state and submit. | Right counter increments; the target state is added to the progress panel; flow advances to the next symbol or back to B. |

Notes:
- The correct answer is the item set of the destination state, not the numeric ID.

---

### SLR1-TC-08 - Complete the B -> C -> CA -> CB loop until D
Preconditions:
- You are at State B and still have pending states in the queue.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Continue answering B, C, CA, and CB for each queued state, following the prompts. | New states and transitions appear in the progress panel as they are discovered. |
| 2 | When no new states are pending, answer B again. | The flow advances to State D (table size). |

---

### SLR1-TC-09 - State D correct path (D -> E)
Preconditions:
- SLR(1) tutor at State D.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter the correct `rows,columns` and submit. | Right counter increments; State E question appears. |

---

### SLR1-TC-10 - State D error path (D -> D1 -> D2 -> D')
Preconditions:
- Start a new SLR(1) session and reach State D.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter an incorrect `rows,columns` and submit. | Wrong counter increments; State D1 question appears. |
| 2 | At D1, enter an incorrect value and submit. | Wrong counter increments; stays at D1. |
| 3 | At D1, enter the correct value and submit. | Right counter increments; State D2 question appears. |
| 4 | At D2, enter an incorrect value and submit. | Wrong counter increments; stays at D2. |
| 5 | At D2, enter the correct value and submit. | Right counter increments; State D' question appears. |
| 6 | At D', enter an incorrect `rows,columns` and submit. | Wrong counter increments; flow advances to State E. |

Notes:
- D' advances even when the answer is incorrect.

---

### SLR1-TC-11 - State E correct path (E -> F)
Preconditions:
- SLR(1) tutor at State E.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter the correct number of states with at least one complete item. | Right counter increments; State F question appears. |

---

### SLR1-TC-12 - State E error path (E -> E1 -> E2)
Preconditions:
- Start a new SLR(1) session and reach State E.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter an incorrect number and submit. | Wrong counter increments; State E1 question appears. |
| 2 | At E1, enter an incorrect ID list and submit. | Wrong counter increments; stays at E1. |
| 3 | At E1, enter the correct ID list and submit. | Right counter increments; State E2 question appears. |
| 4 | At E2, enter an incorrect `id:n` list and submit. | Wrong counter increments; flow advances to State F. |

Notes:
- E2 advances even when the answer is incorrect.

---

### SLR1-TC-13 - State F retry on wrong
Preconditions:
- SLR(1) tutor at State F.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter an incorrect conflict state list and submit. | Wrong counter increments; stays at F. |

---

### SLR1-TC-14 - State F with conflicts present (F -> FA)
Preconditions:
- Use a grammar that produces at least one LR(0) conflict state.
- SLR(1) tutor at State F.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter the correct conflict state IDs and submit. | Right counter increments; State FA question appears for the first conflict state. |

Notes:
- If no conflicts exist, F will go directly to G (see SLR1-TC-15).

---

### SLR1-TC-15 - State FA conflict resolution (per conflict state)
Preconditions:
- SLR(1) tutor at State FA.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter an incorrect terminal set and submit. | Wrong counter increments; stays at FA for the same conflict state. |
| 2 | Enter the correct terminal set and submit. | Right counter increments; moves to the next conflict state or to State G after the last. |

---

### SLR1-TC-16 - State F with no conflicts (F -> G)
Preconditions:
- Use a grammar that produces no LR(0) conflicts.
- SLR(1) tutor at State F.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Leave the response empty and submit. | Right counter increments; State G question appears (FA is skipped). |

---

### SLR1-TC-17 - State G reduction application (loop then H)
Preconditions:
- SLR(1) tutor at State G.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Enter an incorrect terminal set and submit. | Wrong counter increments; stays at G for the same reduce state. |
| 2 | Enter the correct terminal set and submit. | Right counter increments; moves to the next reduce state or to State H when done. |

---

### SLR1-TC-18 - State H table incorrect -> H' wizard, cancel flows
Preconditions:
- SLR(1) tutor at State H (table dialog opens).

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Fill the table incorrectly and click `Finalizar`. | Wrong counter increments; State H' wizard opens. |
| 2 | In the wizard, click `Cancel`. | Confirmation dialog appears. |
| 3 | Select `No`. | Wizard closes; tutor returns to State H and the table dialog opens again. |
| 4 | In the wizard again, click `Cancel` and select `Yes`. | Tutor window closes. |

---

### SLR1-TC-19 - State H correct path + PDF export (Yes/No)
Preconditions:
- Start a new SLR(1) session and reach State H.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Fill the table incorrectly and click `Finalizar`. | Wrong counter increments; State H' wizard opens. |
| 2 | Complete the wizard and click `Finish`. | Tutor returns to State H and the table dialog opens again. |
| 3 | Fill the table correctly and click `Finalizar`. | Tutor reaches final state; export prompt appears. |
| 4 | Select `Yes`, choose a path, and confirm. | PDF is created at the selected path and is non-empty. |
| 5 | Start a new SLR(1) session, reach final state again, and select `No`. | Tutor closes without creating a new PDF. |

---

### SLR1-TC-20 - Cancel SLR table dialog (X button)
Preconditions:
- State H table dialog is open.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Close the table dialog (window X). | Confirmation dialog appears. |
| 2 | Select `No`. | Table dialog reopens and flow continues. |
| 3 | Close the table dialog again; select `Yes`. | Tutor window closes. |
