#include "user.h"

namespace Core
{

User::User()
{
}

User::User(const UsernameType& name, const Permissions& permissions)
	: m_name(name)
	, m_permissions(permissions)
{
}

User::UsernameType User::name() const
{
	return m_name;
}

const User::Permissions& User::permissions() const
{
	return m_permissions;
}

}
