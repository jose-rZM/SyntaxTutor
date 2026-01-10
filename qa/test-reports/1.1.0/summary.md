# Test Execution Report - Version 1.1.0

## Build Information
- Version: 1.1.0
- Commit:
- Date: YYYY-MM-DD

## Environment
- OS: Ubuntu 22.04
- Qt version: X.Y
- Language: ES / EN

## Test Plan
- qa/test-cases/main.md
- qa/test-cases/ll1.md
- qa/test-cases/slr1.md

## Result Legend
- PASS: expected result met
- FAIL: expected result not met
- BLOCKED: test could not be executed
- NOT RUN: not executed in this cycle

## Execution Summary
| Suite | Planned | Executed | Passed | Failed | Blocked | Not Run |
|------|---------|----------|--------|--------|---------|---------|
| Main App | | | | | | |
| LL(1) Tutor | | | | | | |
| SLR(1) Tutor | | | | | | |

## Test Case Results

### Main App
| ID | Title | Result | Evidence | Notes |
|----|-------|--------|----------|-------|
| MAIN-TC-01 | App launch and initial UI | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-02 | Switch language to English | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-03 | Switch language to Spanish | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-04 | About menu | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-05 | Quick references | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-06 | Full tutorial flow | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-07 | LL(1) response input behavior | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-08 | LL(1) table dialog cancel | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-09 | SLR(1) flow to table | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-10 | Export to PDF (LL/SLR) | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-11 | Gamification and level up | PASS/FAIL/BLOCKED/NOT RUN | | |
| MAIN-TC-12 | State persistence across restart | PASS/FAIL/BLOCKED/NOT RUN | | |

### LL(1) Tutor
| ID | Title | Result | Evidence | Notes |
|----|-------|--------|----------|-------|
| LL1-TC-01 | Launch LL(1) session | PASS/FAIL/BLOCKED/NOT RUN | | |
| LL1-TC-02 | State A error path (A -> A1 -> A2 -> A') | PASS/FAIL/BLOCKED/NOT RUN | | |
| LL1-TC-03 | State A correct path (skip A1/A2) | PASS/FAIL/BLOCKED/NOT RUN | | |
| LL1-TC-04 | State B full path (B -> B1 -> B2 -> B') | PASS/FAIL/BLOCKED/NOT RUN | | |
| LL1-TC-05 | State B axiom branch (B1 -> B') | PASS/FAIL/BLOCKED/NOT RUN | | |
| LL1-TC-06 | State B direct path (correct SD) | PASS/FAIL/BLOCKED/NOT RUN | | |
| LL1-TC-07 | State C table correct path + PDF export (Yes/No) | PASS/FAIL/BLOCKED/NOT RUN | | |
| LL1-TC-08 | State C wrong attempts and retry limit (C -> C') | PASS/FAIL/BLOCKED/NOT RUN | | |
| LL1-TC-09 | Cancel LL(1) table dialog | PASS/FAIL/BLOCKED/NOT RUN | | |

### SLR(1) Tutor
| ID | Title | Result | Evidence | Notes |
|----|-------|--------|----------|-------|
| SLR1-TC-01 | Launch SLR(1) session | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-02 | State A correct path (A -> B) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-03 | State A error path (A -> A1 -> A2 -> A3 -> A4 -> A') | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-04 | State B and C incorrect answers still advance (B -> C -> CA) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-05 | State CA incorrect then correct (CA loop) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-06 | State CB with EPSILON symbol (empty required) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-07 | State CB with non-EPSILON symbol (items required) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-08 | Complete the B -> C -> CA -> CB loop until D | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-09 | State D correct path (D -> E) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-10 | State D error path (D -> D1 -> D2 -> D') | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-11 | State E correct path (E -> F) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-12 | State E error path (E -> E1 -> E2) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-13 | State F retry on wrong | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-14 | State F with conflicts present (F -> FA) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-15 | State FA conflict resolution (per conflict state) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-16 | State F with no conflicts (F -> G) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-17 | State G reduction application (loop then H) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-18 | State H table incorrect -> H' wizard, cancel flows | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-19 | State H correct path + PDF export (Yes/No) | PASS/FAIL/BLOCKED/NOT RUN | | |
| SLR1-TC-20 | Cancel SLR table dialog (X button) | PASS/FAIL/BLOCKED/NOT RUN | | |

## Known Issues
- ISSUE-ID - Short description and impact.

## Overall Result
- PASS / FAIL
