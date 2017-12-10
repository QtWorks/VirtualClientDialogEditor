#pragma once

#include "dialog.h"
#include <QString>
#include <QList>

class IDialogProvider
{
public:
	virtual ~IDialogProvider() {}

	class OnReadListener
	{
	public:
		virtual ~OnReadListener() {}

		virtual void onRead() = 0;
		virtual void onError(const QString& message) = 0;
	};

	class OnAddListener
	{
	public:
		virtual ~OnAddListener() {}

		virtual void onAdded() = 0;
		virtual void onError(const QString& message) = 0;
	};

	class OnUpdateListener
	{
	public:
		virtual ~OnUpdateListener() {}

		virtual void onUpdated() = 0;
		virtual void onError(const QString& message) = 0;
	};

	class OnRemoveListener
	{
	public:
		virtual ~OnRemoveListener() {}

		virtual void onRemoved() = 0;
		virtual void onError(const QString& message) = 0;
	};

	virtual void readDialogs(OnReadListener* listener) = 0;
	virtual void addDialog(const QString& name, Dialog::Difficulty difficulty, const Dialog& dialog, OnAddListener* listener) = 0;
	virtual void updateDialog(const QString& name, Dialog::Difficulty difficulty, const Dialog& dialog, OnUpdateListener* listener) = 0;
	virtual void deleteDialog(const QString& name, Dialog::Difficulty difficulty, OnRemoveListener* listener) = 0;

	virtual const QList<Dialog>& getDialogList() = 0;
	virtual int findDialog(const QString& name, Dialog::Difficulty difficulty = Dialog::Difficulty::Easy) = 0;
	virtual const Dialog& getDialogByPosition(int position) = 0;
	virtual const Dialog& getDialogByName(const QString& name, Dialog::Difficulty difficulty) = 0;
};
