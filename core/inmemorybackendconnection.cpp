#include "inmemorybackendconnection.h"

#include <QTimer>

static const int c_delayMsecs = 20;
static const QList<Dialog> c_dialogs = {
	Dialog("Диалог №1", Dialog::Difficulty::Easy, {
		Phase("Фаза 1 (открытие)", {
			Replica(
				"Здравствуйте! С чем сегодня пожаловали?",
				{
					"Здравствуйте, добрый день",
					"Меня зовут",
					"Я медицинский представитель",
					"Я пришел обсудить с Вами варианты по эффективному купированию у пациентов зубной боли различной этиологии и интенсивности"
				},
				"...")
		}),
		Phase("Фаза 2", {
			Replica(
				"Понятно",
				{
					"пациентов с зубной болью"
				},
				"Я бы хотел начать с пациентов с зубной болью. По статистике это каждый 2-й пациент на приеме у врача-стоматолога. "
				"Скажите у Вас на приеме такая же ситуация или отличается?"),
			Replica(
				"Так же",
				{
					"сколько сейчас в среднем на приеме в день человек"
				},
				"А сколько сейчас в среднем на приеме в день человек?"),
			Replica(
				"20 (если хирург), 40 (если терапевт)",
				{
					"зубной болью"
				},
				"Таким образом, у Вас примерно 8-10 пациентов с зубной болью"),
			Replica(
				"Да",
				{
					"боль является серьезной проблемой",
					"ключевыми критериями выбора препарата эффективность и безопасность"
				},
				"Поскольку боль является серьезной проблемой, нарушающей жизнь пациента, то ключевыми критериями выбора"
				" препарата являются его эффективность и безопасность. Согласитесь?"),
			Replica(
				"Да",
				{
					"ДЕКСКЕТОПРОФЕН"
				},
				"Я бы хотел Вам рассказать про препарат ДЕКСКЕТОПРОФЕН. "
				"Если сравнивать с другими обезболивающими, то, максимальная концентрация при приеме ДЕКСКЕТОПРОФЕНа достигается гораздо быстрее, "
				"чем у кеторолака и кетопрофена. Что безусловно важно для пациента, не так ли?"),
		}),
		Phase("Фаза 3 (завершение)", {
			Replica(
				"Согласен",
				{
					"10 пациентам с интенсивной зубной болью"
				},
				"Давайте договоримся, что всем 10 пациентам с интенсивной зубной болью, по показаниям, Вы будете назначать ДЕКСКЕТОПРОФЕН."),
			Replica(
				"Хорошо",
				{
					"необходимость эффективного купирования боли"
				},
				"Наверняка Вы также сталкиваетесь с необходимостью эффективного купирования боли после удаления 3-го маляра. У Вас есть немало таких пациентов?"),
			Replica(
				"Да, 1-2 в день",
				{
					"ДЕКСКЕТОПРОФЕН эффективнее купировал боль, чем кетопрофен"
				},
				"Было проведено исследование эффективности купирования боли у пациентов после удаления 3-го моляра. "
				"Результаты показали, что ДЕКСКЕТОПРОФЕН эффективнее купировал боль, чем кетопрофен. Какой части таких пациентов Вы готовы начать его рекомендацию?"),
			Replica(
				"Думаю, половине попробую",
				{
					"Спасибо за Ваш интерес, применяют ДЕКСКЕТОПРОФЕН, повторно прийти к Вам"
				},
				"Спасибо за Ваш интерес, многие опытные стоматологи применяют именно ДЕКСКЕТОПРОФЕН в период острого болевого синдрома. "
				"Если Вы не возражаете, я хотел бы повторно прийти к Вам через 2 недели. Вы будете на месте? "
				"Мне было бы интересно узнать, какому количеству пациентов с болевым синдромом понадобилось назначение препаратов ДЕКСКЕТОПРОФЕН "
				"и Ваше мнение об эффективности их применения у этих пациентов"),
		})
	}),
	Dialog("Диалог №2", Dialog::Difficulty::Easy, {
		Phase("...", { Replica("- необходимость эффективного купирования боли.", { }, "") })
	})
};

static const QList<User> c_users = {
	User("test", User::Permissions(true, false)),
	User("admin", User::Permissions(true, true)),
};

#define DELAYED_EMIT(signal) QTimer::singleShot(c_delayMsecs, [this]() { emit (signal)(); });
#define DELAYED_EMIT_ARGS(signal, args) QTimer::singleShot(c_delayMsecs, [&]() { emit (signal)(args); });

InMemoryBackendConnection::InMemoryBackendConnection()
	: m_dialogs(c_dialogs)
	, m_users(c_users)
{
}

void InMemoryBackendConnection::open(const QString& /*url*/, const QString& login, const QString& password)
{
	if (login == "admin" && password == "admin")
	{
		DELAYED_EMIT(onAuth);
	}
	else
	{
		DELAYED_EMIT_ARGS(onError, "Неправильное имя пользователя или пароль");
	}
}

void InMemoryBackendConnection::close()
{
}

void InMemoryBackendConnection::readDialogs()
{
	DELAYED_EMIT_ARGS(onDialogsReaded, m_dialogs);
}

void InMemoryBackendConnection::addDialog(const Dialog& /*dialog*/)
{
}

void InMemoryBackendConnection::updateDialog(const QString& /*name*/, Dialog::Difficulty /*difficulty*/, const Dialog& /*dialog*/)
{
}

void InMemoryBackendConnection::deleteDialog(const QString& /*name*/, Dialog::Difficulty /*difficulty*/)
{
}

void InMemoryBackendConnection::readUsers()
{
	DELAYED_EMIT_ARGS(onUsersReaded, m_users);
}

void InMemoryBackendConnection::addUser(const User& /*user*/)
{
}

void InMemoryBackendConnection::updateUser(const User::UsernameType& /*username*/, const User& /*user*/)
{
}

void InMemoryBackendConnection::deleteUser(const User::UsernameType& /*username*/)
{
}

void InMemoryBackendConnection::triggerError(const QString& message)
{
	DELAYED_EMIT_ARGS(onError, message);
}
