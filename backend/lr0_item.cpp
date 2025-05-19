#include <cstddef>
#include <functional>
#include <iostream> // DEBUG ONLY
#include <string>
#include <utility>
#include <vector>

#include "lr0_item.hpp"
#include "symbol_table.hpp"

Lr0Item::Lr0Item(std::string antecedent, std::vector<std::string> consequent,
                 std::string epsilon, std::string eol)
    : antecedent_(std::move(antecedent)), consequent_(std::move(consequent)),
      epsilon_(std::move(epsilon)), eol_(std::move(eol)), dot_(0) {
    if (this->consequent_.size() == 1 && this->consequent_[0] == epsilon_) {
        dot_ = 1;
    }
}

Lr0Item::Lr0Item(std::string antecedent, std::vector<std::string> consequent,
                 unsigned int dot, std::string epsilon, std::string eol)
    : antecedent_(std::move(antecedent)), consequent_(std::move(consequent)),
      epsilon_(std::move(epsilon)), eol_(std::move(eol)), dot_(dot) {
    if (this->consequent_.size() == 1 && this->consequent_[0] == epsilon_) {
        dot = 1;
    }
}

std::string Lr0Item::NextToDot() const {
    if (dot_ >= consequent_.size()) {
        return epsilon_;
    }
    return consequent_[dot_];
}

void Lr0Item::AdvanceDot() {
    if (dot_ < consequent_.size()) {
        dot_++;
    }
}

bool Lr0Item::IsComplete() const {
    return dot_ >= consequent_.size() || (consequent_.size() == 1 && consequent_[0] == epsilon_);
}

void Lr0Item::PrintItem() const {
    std::cout << "[ " << antecedent_ << " -> ";
    for (unsigned int i = 0; i < consequent_.size(); ++i) {
        if (i == dot_) {
            std::cout << "· ";
        }
        std::cout << consequent_[i] << " ";
    }
    if (dot_ == consequent_.size()) {
        std::cout << "· ";
    }
    std::cout << "]";
}

std::string Lr0Item::ToString() const {
    std::string str = "[ " + antecedent_ + " -> ";
    for (size_t i = 0; i < consequent_.size(); ++i) {
        if (i == dot_) {
            str += "· ";
        }
        str += consequent_[i] + " ";
    }
    if (dot_ == consequent_.size()) {
        str += "· ";
    }
    str += "]";
    return str;
}

bool Lr0Item::operator==(const Lr0Item& other) const {
    return antecedent_ == other.antecedent_ &&
           consequent_ == other.consequent_ && dot_ == other.dot_;
}

namespace std {
std::size_t hash<Lr0Item>::operator()(const Lr0Item& item) const {
    std::size_t seed = 0;

    seed ^= std::hash<std::string>()(item.antecedent_) + 0x9e3779b9 +
            (seed << 6) + (seed >> 2);
    for (const auto& str : item.consequent_) {
        seed ^= std::hash<std::string>()(str) + 0x9e3779b9 + (seed << 6) +
                (seed >> 2);
    }
    seed ^=
        std::hash<int>()(item.dot_) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    return seed;
}
} // namespace std
