#ifndef APPVERSION_H
#define APPVERSION_H

#include <QString>

namespace SyntaxTutor::Version {
inline QString current() {
    return QStringLiteral("1.0.3");
}

inline const char* raw() {
    return "1.0.3";
}
} // namespace SyntaxTutor::Version

#endif // APPVERSION_H
