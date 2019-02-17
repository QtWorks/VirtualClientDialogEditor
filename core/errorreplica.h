#pragma once

#include "optional.h"

#include <QString>
#include <QList>
#include <QMetaType>

namespace Core
{

struct ErrorReplica
{
	Optional<QString> errorReplica;
	Optional<double> errorPenalty;
	Optional<QList<QString>> finishingExpectedWords;
	Optional<QString> finishingReplica;

	bool hasAnyField() const
	{
		return errorReplica || errorPenalty || finishingExpectedWords || finishingReplica;
	}

	friend bool operator==(const ErrorReplica& left, const ErrorReplica& right);
};

inline bool operator==(const ErrorReplica& left, const ErrorReplica& right)
{
	const auto rank = [](const ErrorReplica& error)
	{
		return std::tie(error.errorReplica, error.errorPenalty, error.finishingExpectedWords, error.finishingReplica);
	};

	return rank(left) == rank(right);
}

inline bool operator!=(const ErrorReplica& left, const ErrorReplica& right)
{
	return !(left == right);
}

}

Q_DECLARE_METATYPE(Core::ErrorReplica)
