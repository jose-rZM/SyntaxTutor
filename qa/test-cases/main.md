# Test Cycle: Main App

## Scope
- Functional checks for the main window and global flows (language, tutorial, references).
- Covers LL(1)/SLR(1) entry points and PDF export at a high level.
- Tutor logic is covered in the LL(1) and SLR(1) test cycles.

## Global Preconditions
- App launches successfully.
- Start from the main window unless stated otherwise.

## Input Formats (use exactly)
- N/A (no structured input beyond UI selections).

## Test Cases

### MAIN-TC-01 - App launch and initial UI
Preconditions:
- App installed.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Launch the app. | Main window is visible with no errors. |
| 2 | Verify buttons `LL(1)`, `SLR(1)`, `Tutorial`, and `Language`. | All main controls are enabled. |
| 3 | Verify progress bar, level, and score. | Level and score are visible and consistent. |

---

### MAIN-TC-02 - Switch language to English
Preconditions:
- App open.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Click `Language`. | Language selection dialog opens. |
| 2 | Choose `English`. | Restart confirmation appears. |
| 3 | Confirm restart and reopen. | Main UI text is in English and previous state is preserved (level/score). |

---

### MAIN-TC-03 - Switch language to Spanish
Preconditions:
- App open in English.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Click `Language`. | Language selection dialog opens. |
| 2 | Choose `Spanish`. | Restart confirmation appears. |
| 3 | Confirm restart and reopen. | Main UI text is in Spanish and previous state is preserved. |

---

### MAIN-TC-04 - About menu
Preconditions:
- App open.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Open the `About` menu. | About menu opens. |
| 2 | Click `About the app`. | Dialog shows version, license, author, and link. |

---

### MAIN-TC-05 - Quick references
Preconditions:
- App open.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Open `About` > `LL(1) Reference`. | LL(1) reference dialog appears. |
| 2 | Close the dialog. | Returns to main window. |
| 3 | Open `About` > `SLR(1) Reference`. | SLR(1) reference dialog appears. |

---

### MAIN-TC-06 - Full tutorial flow
Preconditions:
- App open with default level.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Click `Tutorial`. | Tutorial starts. |
| 2 | Follow steps with `Next`. | Tutorial advances without errors. |
| 3 | Verify LL(1) and then SLR(1) windows open during the tutorial. | Tutor windows open as guided. |
| 4 | Complete the tutorial. | Main controls are re-enabled; tutorial can be restarted. |

---

### MAIN-TC-07 - LL(1) response input behavior
Preconditions:
- LL(1) tutor window open.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Type multiple lines using Shift+Enter. | New line is inserted without sending. |
| 2 | Press Enter to submit. | Response is sent to the tutor chat. |
| 3 | Submit an incorrect response. | Wrong counter increments and feedback appears. |

---

### MAIN-TC-08 - LL(1) table dialog cancel
Preconditions:
- LL(1) tutor at a state that requires the table.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Open the table dialog. | Table dialog appears. |
| 2 | Close the dialog with the window X. | Confirmation dialog appears. |
| 3 | Select `No`. | Table dialog reopens and flow continues. |
| 4 | Close the dialog again and select `Yes`. | Tutor window closes. |

---

### MAIN-TC-09 - SLR(1) flow to table
Preconditions:
- SLR(1) tutor window open.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Start the exercise and advance to the table step. | SLR(1) table dialog appears. |
| 2 | Submit a response with `Finalizar`. | Submission is processed with no hang; feedback is shown. |

---

### MAIN-TC-10 - Export to PDF (LL/SLR)
Preconditions:
- Finish an LL(1) or SLR(1) exercise.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Select `Yes` to export. | File picker opens. |
| 2 | Choose a path and confirm. | PDF is created at the selected path. |
| 3 | Open the PDF. | PDF is non-empty and contains the expected conversation/summary. |

---

### MAIN-TC-11 - Gamification and level up
Preconditions:
- Complete exercises.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Finish a session with more correct than wrong answers. | Points increase. |
| 2 | Repeat until the level threshold is reached. | Level increases; progress bar updates and persists. |

---

### MAIN-TC-12 - State persistence across restart
Preconditions:
- Language, level, or score changed.

Steps:
| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Close the app. | App closes normally. |
| 2 | Reopen the app. | Language, level, and score are preserved. |
