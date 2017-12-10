#ifndef USER_H
#define USER_H

#include <QString>

class User
{
public:
	typedef QString UsernameType;
	struct Permissions
	{
		Permissions()
			: read(true)
			, write(false)
		{
		}

		Permissions(bool read, bool write)
			: read(read)
			, write(write)
		{
		}

		bool read;
		bool write;
	};

	User();
	User(const UsernameType& name, const Permissions& permissions);

	UsernameType name() const;
	const Permissions& permissions() const;

private:
	UsernameType m_name;
	Permissions m_permissions;
};

#endif // USER_H
