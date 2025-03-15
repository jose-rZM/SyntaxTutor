#pragma once

#include <string>
#include <vector>

struct Lr0Item {
    std::string              antecedent_;
    std::vector<std::string> consequent_;
    std::string              epsilon_;
    std::string              eol_;
    unsigned int             dot_ = 0;

    Lr0Item(std::string antecedent, std::vector<std::string> consequent,
            std::string epsilon, std::string eol);
    Lr0Item(std::string antecedent, std::vector<std::string> consequent,
            unsigned int dot, std::string epsilon, std::string eol);
    std::string NextToDot() const;
    void        PrintItem() const;
    std::string ToString() const;
    void        AdvanceDot();
    bool        IsComplete() const;
    bool        operator==(const Lr0Item& other) const;
};

namespace std {
template <> struct hash<Lr0Item> {
    size_t operator()(const Lr0Item& item) const;
};
} // namespace std
