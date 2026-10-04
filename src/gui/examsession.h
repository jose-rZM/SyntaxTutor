/*
 * SyntaxTutor - Interactive Tutorial About Syntax Analyzers
 * Copyright (C) 2025 Jose R. (jose-rzm)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef EXAMSESSION_H
#define EXAMSESSION_H

#include <QString>
#include <QVector>

/**
 * @struct ExamRecord
 * @brief One graded item of an exam: a question (or table cell), the user's
 * answer, the expected answer, and whether it was correct.
 */
struct ExamRecord {
    /// @brief Question text as it was shown to the student.
    QString question;
    /// @brief Answer the student gave.
    QString userAnswer;
    /// @brief Expected answer.
    QString correctAnswer;
    /// @brief Whether the student's answer was correct.
    bool correct;
};

/**
 * @class ExamSession
 * @brief Accumulates graded answers during an exam-mode exercise.
 *
 * In exam mode the tutors give no feedback: every answer is recorded here
 * with its real correctness while the exercise advances along the correct
 * path. At the end the session produces the stats and the 0-10 grade shown
 * in the exam report.
 */
class ExamSession {
  public:
    /**
     * @brief Records one graded item.
     *
     * @param question Question text shown to the student.
     * @param userAnswer Answer the student gave.
     * @param correctAnswer Expected answer.
     * @param correct Whether the answer was correct.
     */
    void record(const QString& question, const QString& userAnswer,
                const QString& correctAnswer, bool correct) {
        records_.push_back({question, userAnswer, correctAnswer, correct});
    }

    /// @brief Total number of graded items.
    int total() const { return static_cast<int>(records_.size()); }

    /// @brief Number of correct items.
    int right() const {
        int count = 0;
        for (const ExamRecord& record : records_) {
            if (record.correct) {
                ++count;
            }
        }
        return count;
    }

    /**
     * @brief Grade on the Spanish 0-10 scale.
     *
     * An empty exam grades 0.
     */
    double grade() const {
        if (records_.isEmpty()) {
            return 0.0;
        }
        return 10.0 * right() / total();
    }

    /// @brief All graded items in exam order.
    const QVector<ExamRecord>& records() const { return records_; }

  private:
    QVector<ExamRecord> records_;
};

#endif // EXAMSESSION_H
