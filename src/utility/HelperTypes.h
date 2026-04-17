#ifndef HELPER_TYPES_H
#define HELPER_TYPES_H

#include <stdexcept>
#include <sstream>
#include <tuple>

/// A marker class
/// Extend this class to make the base class as non-copyable
struct NonCopyable {
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable() = default;
};

/// A useful shorthand that not all compilers provide by default
using uint = unsigned int;

/// An easy way to format to a string
/// Usage:
/// int a = 12;
///
/// std::string message = Formatter() << "a = " << a;
/// /* now message = "a = 12" */
///
/// From: https://stackoverflow.com/a/12262626
class Formatter : NonCopyable {
public:
    Formatter() = default;
    ~Formatter() = default;

    template<typename Type>
    Formatter& operator<<(const Type& value) {
        stream << value;
        return *this;
    }

    std::string str() const { return stream.str(); }

    operator std::string() const { return stream.str(); } // NOLINT(google-explicit-constructor)
private:
    std::stringstream stream;
};

/// A hasher to allow using a pair as a key for an unordered_map or unordered_set,
/// see ModelLoader.cache for an example
struct PairHash {
    template<class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

/// A hasher to allow using a 3-tuple as a key for an unordered_map or unordered_set,
/// see TextureLoader.cache for an example
struct TripleHash {
    template<class T1, class T2, class T3>
    std::size_t operator()(const std::tuple<T1, T2, T3>& triple) const {
        return std::hash<T1>()(std::get<0>(triple))
               ^ (std::hash<T2>()(std::get<1>(triple)) << 3)
               ^ (std::hash<T3>()(std::get<2>(triple)) << 7);
    }
};

#endif //HELPER_TYPES_H
