#include "phase.h"

#include <tuple>

Phase::Phase()
{
}

Phase::Phase(const QString& name, const QList<Replica>& replicas)
	: name(name)
	, replicas(replicas)
{
}

bool Phase::operator==(const Phase& other) const
{
	auto rank = [](const Phase& phase)
	{
		return std::tie(phase.name, phase.replicas);
	};

	return rank(*this) == rank(other);
}
