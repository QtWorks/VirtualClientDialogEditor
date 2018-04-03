#ifndef DIALOG_H
#define DIALOG_H

#include <QString>
#include <QList>

namespace Core
{

class AbstractDialogNodeVisitor;

class AbstractDialogNode
{
public:
	virtual ~AbstractDialogNode() {}

	AbstractDialogNode& appendChild(AbstractDialogNode* child)
	{
		Q_ASSERT(m_childNodes.indexOf(child) == -1);
		m_childNodes.append(child);
		return *this;
	}

	AbstractDialogNode& removeChild(AbstractDialogNode* child)
	{
		Q_ASSERT(m_childNodes.indexOf(child) != -1);
		m_childNodes.removeOne(child);
		return *this;
	}

	QList<AbstractDialogNode*> childNodes() const
	{
		return m_childNodes;
	}

	virtual void accept(AbstractDialogNodeVisitor* visitor) = 0;
	virtual void accept(const AbstractDialogNodeVisitor* visitor) const = 0;

protected:
	QList<AbstractDialogNode*> m_childNodes;
};

class PhaseNode;
class ClientReplicaNode;
class ExpectedWordsNode;

class AbstractDialogNodeVisitor
{
public:
	virtual ~AbstractDialogNodeVisitor() {}

	virtual void visit(PhaseNode* /*node*/) {}
	virtual void visit(const PhaseNode* /*node*/) const {}
	virtual void visit(ClientReplicaNode* /*node*/) {}
	virtual void visit(const ClientReplicaNode* /*node*/) const {}
	virtual void visit(ExpectedWordsNode* /*node*/) {}
	virtual void visit(const ExpectedWordsNode* /*node*/) const {}
};

class PhaseNode
	: public AbstractDialogNode
{
public:
	PhaseNode(const QString& name, double score, AbstractDialogNode* root)
		: name(name)
		, score(score)
		, root(root)
	{
	}

	virtual void accept(AbstractDialogNodeVisitor* visitor) override { visitor->visit(this); }
	virtual void accept(const AbstractDialogNodeVisitor* visitor) const override { visitor->visit(this); }

	QString name;
	double score;
	AbstractDialogNode* root;
};

class ClientReplicaNode
	: public AbstractDialogNode
{
public:
	ClientReplicaNode(const QString& replica)
		: replica(replica)
	{
	}

	virtual void accept(AbstractDialogNodeVisitor* visitor) override { visitor->visit(this); }
	virtual void accept(const AbstractDialogNodeVisitor* visitor) const override { visitor->visit(this); }

	QString replica;
};

struct ExpectedWords
{
	ExpectedWords(const QString& words, double score)
		: words(words)
		, score(score)
	{
	}

	QString words;
	double score;
};

class ExpectedWordsNode
	: public AbstractDialogNode
{
public:
	ExpectedWordsNode(QList<ExpectedWords> expectedWords)
		: expectedWords(expectedWords)
	{
	}

	ExpectedWordsNode(QList<ExpectedWords> expectedWords, QString hint)
		: expectedWords(expectedWords)
		, hint(hint)
	{
	}

	virtual void accept(AbstractDialogNodeVisitor* visitor) override { visitor->visit(this); }
	virtual void accept(const AbstractDialogNodeVisitor* visitor) const override { visitor->visit(this); }

	QList<ExpectedWords> expectedWords;
	QString hint;
};

class Dialog
{
public:
	enum class Difficulty
	{
		Easy,
		Hard
	};

	Dialog()
	{
	}

	Dialog(const QString& name, Difficulty difficulty, const QList<PhaseNode>& phases)
		: name(name)
		, difficulty(difficulty)
		, phases(phases)
	{
	}

	QString printableName() const
	{
		return name + " (" + difficultyToString(difficulty) + ")";
	}

	// TODO: error replica?
	// TODO: intro replica?
	// TODO: outro replica?

	static QString difficultyToString(Difficulty difficulty)
	{
		if (difficulty == Difficulty::Easy)
		{
			return "простой";
		}

		if (difficulty == Difficulty::Hard)
		{
			return "сложный";
		}

		Q_ASSERT(!"Unexpected difficulty");
		return QString();
	}

	static Difficulty difficultyFromString(const QString& string)
	{
		if (string.compare(difficultyToString(Difficulty::Easy), Qt::CaseInsensitive) == 0)
		{
			return Difficulty::Easy;
		}

		if (string.compare(difficultyToString(Difficulty::Hard), Qt::CaseInsensitive) == 0)
		{
			return Difficulty::Hard;
		}

		Q_ASSERT(!"Unexpected difficulty");
	}

	static QStringList availableDifficulties()
	{
		return { difficultyToString(Difficulty::Easy), difficultyToString(Difficulty::Hard) };
	}

	QString name;
	Difficulty difficulty;
	QList<PhaseNode> phases;
};

}

#endif // DIALOG_H
