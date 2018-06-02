#pragma once

#include <QString>
#include <string>
#include <vector>
#include <map>
#include <ostream>

#include <deque>
#include <sstream>

struct Point
{
	int x;
	int y;
};

class GraphLayout
{
public:
	GraphLayout(int numLayer);

	typedef std::vector<int> Point;

	struct GraphNode
	{
		int x;
		int y;
		double lx;
		double ly;
		std::vector<Point> trgNodes;
		std::vector<Point> srcNodes;
		double wUpper;
		double wLower;
		bool virt;
		QString label;
		bool selected;
	};

	struct GraphNodeData
	{
		QString label;
		int layer;

		GraphNode node; // needed to be able to update trgNodes in addEdges
	};

	struct GraphData
	{
		std::vector<GraphNodeData> nodeList;
		std::vector<std::vector<int>> adjList;
		int totalLayers;
	};

	typedef std::map<int, std::vector<GraphNode>> NodesByLayer;

	NodesByLayer render(const GraphData& graphData);

	friend std::ostream& operator<<(std::ostream& os, const GraphLayout& layout);
	operator QString() const { std::stringstream s; s << *this; return QString::fromStdString(s.str()); }

private:
	void initNodeList(std::vector<GraphNodeData>& nodeList);
	GraphNode createNode(int x, int y, bool virt, QString label);

	void addEdges(std::vector<GraphNodeData> nodeList, std::vector<std::vector<int>> adjList);

	void expandNodeList();

	typedef std::pair<Point, Point> PointsPair;
	std::map<PointsPair, std::deque<Point>> splitLongEdges(GraphNode& srcNode);
	GraphNode createVirtualNode(const GraphNode& srcNode, const GraphNode& trgNode, int span);
	GraphNode createNode(int x, int y, const Point& target, const Point& source, bool virt, QString label);
	void setNodeWeights();
	double getMedian(const GraphNode& node, std::string type);

	void minimizeCrossings();
	bool orderLayer(int layer, std::string direction);
	int countCrossings(int layer, std::string direction);
	void updateEdges(GraphNode& node, int oldIndex);

	void setLayoutPosition();
	void align(int layer, std::string direction);

private:
	int numLayer = 0;
	std::map<int, int> numPerLayers;
	std::map<int, std::vector<GraphNode>> nodes;
	double incVirt = 1;
	int inc = 1.0;
	int numRepeat = 20;
};

std::ostream& operator<<(std::ostream& os, const GraphLayout& layout);
std::ostream& operator<<(std::ostream& os, const GraphLayout::GraphNode& node);
