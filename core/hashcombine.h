#ifndef HASHCOMBINE_H
#define HASHCOMBINE_H

#include <QHash>

template <typename T>
inline void hashCombine(size_t& seed, const T& value)
{
	seed ^= qHash(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

#endif // HASHCOMBINE_H
