#ifndef APPVERSION_H
#define APPVERSION_H

#include <QString>

namespace SyntaxTutor::Version {
inline QString current() {
    return QStringLiteral("2.0.0-ALPHA");
}

inline const char* raw() {
    return "2.0.0-ALPHA";
}
} // namespace SyntaxTutor::Version
#endif // APPVERSION_H
