#include "replica.h"

#include <tuple>

Replica::Replica()
{
}

Replica::Replica(const QString& clientReplica, const QStringList& expectedWords, const QString& hint)
	: clientReplica(clientReplica)
	, expectedWords(expectedWords)
	, hint(hint)
{
}

bool Replica::operator==(const Replica& other) const
{
	auto rank = [](const Replica& replica)
	{
		return std::tie(replica.clientReplica, replica.expectedWords, replica.hint);
	};

	return rank(*this) == rank(other);
}
