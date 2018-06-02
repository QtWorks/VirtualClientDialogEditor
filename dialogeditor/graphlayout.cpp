#include "graphlayout.h"
#include <algorithm>

#include <iostream>

namespace
{

int generateId()
{
	static int s_id = 0;
	return ++s_id;
}

}

GraphLayout::GraphLayout(int numLayer)
	: numLayer(numLayer)
{
}

GraphLayout::NodesByLayer GraphLayout::render(const GraphData& graphData)
{
	GraphData data = graphData;

	initNodeList(data.nodeList);

	addEdges(data.nodeList, data.adjList);

	expandNodeList();
	minimizeCrossings();

	setLayoutPosition();

	return nodes;
}

void GraphLayout::initNodeList(std::vector<GraphNodeData>& nodeList)
{
	for (size_t i = 0; i < nodeList.size(); ++i)
	{
		int layer = nodeList[i].layer;

		size_t col = nodes[layer].size();

		GraphNode node = createNode(col, layer, false, nodeList[i].label);

		// internal node list for fast lookup in grid
		if (nodes[layer].size() == col)
		{
			nodes[layer].push_back(node);
		}
		else
		{
			nodes[layer][col] = node;
		}

		nodeList[i].node = node;
		numPerLayers[layer] = col + 1;
	}

	const int maxLayer = std::prev(numPerLayers.end())->first;
	for (int i = 0; i < maxLayer; ++i)
	{
		if (numPerLayers.find(i) == numPerLayers.end())
		{
			numPerLayers.insert({ i, 0 });
		}
	}
}

GraphLayout::GraphNode GraphLayout::createNode(int x, int y, bool virt, QString label)
{
	GraphNode node;
	node.x = x;
	node.y = y;
	node.lx = x;
	node.ly = y;
	node.wUpper = -1; // -1 means null
	node.wLower = -1;
	node.virt = virt;
	node.label = label;
	return node;
}

void GraphLayout::addEdges(std::vector<GraphNodeData> nodeList, std::vector<std::vector<int>> adjList)
{
	for (size_t i = 0; i < adjList.size(); i++)
	{
		if (adjList[i].size() <= 0)
		{
			continue;
		}

		std::vector<int> trgNodes = adjList[i];

		for (size_t j = 0; j < trgNodes.size(); j++)
		{
			const GraphNode& target = nodeList[trgNodes[j]].node;
			const int tx = target.x;
			const int ty = target.y;

			const GraphNode& source = nodeList[i].node;	// get target's source nodes
			const int sx = source.x;
			const int sy = source.y;

			nodes[sy][sx].trgNodes.push_back({ tx, ty });
			// also add this node to the source list of the edge
			nodes[ty][tx].srcNodes.push_back({ sx, sy });
		}
	}
}

void GraphLayout::expandNodeList()
{
	for (size_t i = 0; i < nodes.size(); i++)
	{
		for (size_t j = 0; j < nodes[i].size(); j++)
		{
			GraphNode& node = nodes[i][j];
			if (!node.virt && node.trgNodes.size() > 0)
			{
				splitLongEdges(node);
			}
		}
	}

	setNodeWeights();
}

std::map<GraphLayout::PointsPair, std::deque<GraphLayout::Point>> GraphLayout::splitLongEdges(GraphNode& srcNode)
{
	std::map<PointsPair, std::deque<Point>> result;

	GraphNode lastVirtNode;
	bool lastVirtNodeAssigned = false;
	GraphNode newNode;

	for (size_t w = 0; w < srcNode.trgNodes.size(); w++)
	{
		lastVirtNodeAssigned = false;

		GraphNode trgNode;
		trgNode.x = srcNode.trgNodes[w][0];
		trgNode.y = srcNode.trgNodes[w][1];

		int span = trgNode.y - srcNode.y;
		// skip short edges of unit length
		if (span == 1)
		{
			continue;
		}

		std::deque<Point> points;

		span--; // We do not need to crate a new node for the original targetNode
		while (span > 0)
		{
			newNode = createVirtualNode(srcNode, trgNode, span); // APPEND virtual nodes and edges
			if (!lastVirtNodeAssigned)
			{
				lastVirtNode = newNode;
				lastVirtNodeAssigned = true;
			}

			if (nodes.find(newNode.y) == nodes.end())
			{
				nodes.insert({ newNode.y, { newNode } });
			}
			else
			{
				nodes[newNode.y].push_back(newNode);
			}

			points.push_front({ newNode.x, newNode.y });

			// update maximum number of nodes per layer to know width of canvas
			numPerLayers[newNode.y]++;
			span--;
		}

		// update original targetNode's srcNode to point to first inserted new node
		const Point& targetNode = srcNode.trgNodes[w];
		GraphNode& origTrgNode = nodes[targetNode[1]][targetNode[0]];

		// find right srcNode to update
		for (size_t f = 0; f < origTrgNode.srcNodes.size(); f++)
		{
			Point& node = origTrgNode.srcNodes[f];

			if (node[0] == srcNode.x && node[1] == srcNode.y)
			{
				origTrgNode.srcNodes[f][0] = lastVirtNode.x;
				origTrgNode.srcNodes[f][1] = lastVirtNode.y;
				break;
			}
		}

		// update srcNode's original targetNode to point to last inserted new node
		// note: do this after updating original target's srcNode, otherwise it is overwritten by newNode
		srcNode.trgNodes[w] = Point({ newNode.x, newNode.y });

		const Point srcNodePoint = { srcNode.x, srcNode.y };
		const Point trgNodePoint = { srcNode.trgNodes[w][0], srcNode.trgNodes[w][1] };
		result.insert({ { srcNodePoint, trgNodePoint }, points });
	}

	return result;
}

GraphLayout::GraphNode GraphLayout::createVirtualNode(const GraphNode& srcNode, const GraphNode& trgNode, int span)
{
	// we have 4 cases: target is a VirtualNode
	// target is originalTarget
	// source is virtual
	// source is originalSource

	int curLayer = srcNode.y + span;

	// second new node has original target node as target
	Point target = (curLayer == trgNode.y - 1) ?
		Point{ trgNode.x, trgNode.y } :
		Point{ numPerLayers[curLayer + 1] - 1, curLayer + 1 };

	// second to last has original source as source
	Point source = span == 1 ?
		Point{ srcNode.x, srcNode.y } :
		Point{ numPerLayers[curLayer - 1], curLayer - 1 };

	const QString label = QString("v%1").arg(generateId());
	return createNode(numPerLayers[curLayer], curLayer, target, source, true, label);
}

GraphLayout::GraphNode GraphLayout::createNode(int x, int y, const Point& target, const Point& source, bool virt, QString label)
{
	GraphNode node;
	node.x = x;
	node.y = y;
	node.lx = x;
	node.ly = y;
	node.trgNodes = { target };
	node.srcNodes = { source };
	node.wUpper = -1; // -1 means null
	node.wLower = -1; // -1 means null
	node.virt = virt;
	node.label = label; // "" means null
	node.selected = false;
	return node;
}

void GraphLayout::setNodeWeights()
{
	for (size_t i = 0; i < nodes.size(); i++)
	{
		for (size_t w = 0; w < nodes[i].size(); w++)
		{
			GraphNode& node = nodes[i][w];

			if (node.srcNodes.size() > 0)
			{
				node.wUpper = getMedian(node, "upper");
			}

			if (node.trgNodes.size() > 0)
			{
				node.wLower = getMedian(node, "lower");
			}
		}
	}
}

double GraphLayout::getMedian(const GraphNode& node, std::string type)
{
	std::vector<double> weights;

	const auto& nodes = (type == "upper" ? node.srcNodes : node.trgNodes);

	for (size_t i = 0; i < nodes.size(); i++)
	{
		const Point& n = nodes[i];
		// do not sort srcNodes or trgNodes directly
		weights.push_back(this->nodes[n[1]][n[0]].lx);
	}

	std::sort(weights.begin(), weights.end());

	int middle = std::floor(weights.size() / 2);

	if ((weights.size() % 2) != 0)
	{
		return weights[middle];
	}

	return (weights[middle - 1] + weights[middle]) / 2;
}

void GraphLayout::minimizeCrossings()
{
	for (int z = 0; z < numRepeat; z++)
	{
		// first layer does not have any upperWeights
		for (size_t i = 1; i < nodes.size(); i++)
		{
			orderLayer(i, "down");
		}

		// last layer does not have any lowerWeights
		for (int i = nodes.size() - 2; i > -1; i--)
		{
			orderLayer(i, "up");
		}
	}
}

bool compareByUpperWeight(const GraphLayout::GraphNode& left, const GraphLayout::GraphNode& right)
{
	if (left.wUpper == -1 || right.wUpper == -1)
	{
		return false;
	}

	return left.wUpper < right.wUpper;
}

bool compareByLowerWeight(const GraphLayout::GraphNode& left, const GraphLayout::GraphNode& right)
{
	if (left.wLower == -1 || right.wLower == -1)
	{
		return false;
	}

	return left.wLower < right.wLower;
}

bool GraphLayout::orderLayer(int layer, std::string direction)
{
	std::vector<GraphNode> nodeOrderRestore = nodes[layer];
	int numCross1 = countCrossings(layer, direction);

	std::vector<GraphNode>& nodeOrder = nodes[layer]; // node ordering has to be done on this.nodes for countCrossings to work
	std::sort(nodeOrder.begin(), nodeOrder.end(), direction == "up" ? compareByLowerWeight : compareByUpperWeight);

	int numCross2 = countCrossings(layer, direction);
	if (numCross2 < numCross1)
	{
		// reassign new position (rank)

		for (size_t i = 0; i < nodeOrder.size(); i++)
		{
			if (i != static_cast<size_t>(nodeOrder[i].x)) // set new pos only if node position has changed
			{
				int oldX = nodeOrder[i].x;
				nodeOrder[i].x = i;
				nodeOrder[i].lx = i;
				// update edges and weights of rearranged nodes
				updateEdges(nodeOrder[i], oldX);
			}
		}
		return true;
	}

	// update order in nodelist
	nodes[layer] = nodeOrderRestore;
	return false;
}

int GraphLayout::countCrossings(int layer, std::string direction)
{
	std::vector<GraphNode>& nodes = this->nodes[layer];
	int numCross = 0;

	for (size_t i = 0; i < nodes.size(); i++)
	{
		GraphNode& node1 = nodes[i];
		std::vector<Point>& edges1 = direction == "up" ? node1.trgNodes : node1.srcNodes;

		// loop node's targets
		for (size_t w = 0; w < edges1.size(); w++)
		{
			const GraphNode& edge1Node = this->nodes[edges1[w][1]][edges1[w][0]];

			// compare
			for (size_t z = i + 1; z < nodes.size(); z++)
			{
				const GraphNode& node2 = nodes[z];
				const std::vector<Point>& edges2 = direction == "up" ? node2.trgNodes : node2.srcNodes;

				for (size_t p = 0; p < edges2.size(); p++)
				{
					const GraphNode& edge2Node = this->nodes[edges2[p][1]][edges2[p][0]];

					if (edge1Node.x > edge2Node.x)
					{
						numCross++;
					}
				}
			}
		}
	}

	return numCross;
}

void GraphLayout::updateEdges(GraphNode& node, int oldIndex)
{
	// update node's source nodes
	for (size_t i = 0; i < node.srcNodes.size(); i++)
	{
		int x = node.srcNodes[i][0];
		int y = node.srcNodes[i][1];

		// find srcNode's target nodes to update
		GraphNode& trgNode = nodes[y][x];
		std::vector<Point>& trgNodes = trgNode.trgNodes;

		for (size_t v = 0; v < trgNodes.size(); v++)
		{
			if (trgNodes[v][0] == oldIndex)
			{
				trgNodes[v][0] = node.x;
				trgNode.wLower = getMedian(trgNode, "lower");
				break;
			}
		}
	}

	// update node's target nodes
	for (size_t i = 0; i < node.trgNodes.size(); i++)
	{
		int x = node.trgNodes[i][0];
		int y = node.trgNodes[i][1];

		// find trgNode's source nodes to update
		GraphNode& srcNode = nodes[y][x];
		std::vector<Point>& srcNodes = srcNode.srcNodes;

		for (size_t v = 0; v < srcNodes.size(); v++)
		{
			if (srcNodes[v][0] == oldIndex)
			{
				srcNodes[v][0] = node.x;
				srcNode.wUpper = getMedian(srcNode, "upper");
				break;
			}
		}
	}
}

void GraphLayout::setLayoutPosition()
{
	const int MAGIC_NUMBER = 4;

	for (int w = 0; w < MAGIC_NUMBER; w++)
	{
		for (int i = 0; i < static_cast<int>(nodes.size()); i++)
		{
			align(i, "down");
		}

		setNodeWeights();

		for (int i = static_cast<int>(nodes.size() - 1); i > -1; i--)
		{
			align(i, "up");
		}

		setNodeWeights();
	}
}

template <typename T>
T round(T num, int decimalPlaces)
{
	return std::round(num * std::pow(10, decimalPlaces)) / std::pow(10, decimalPlaces);
}

void GraphLayout::align(int layer, std::string direction)
{
	// move node according to it's upper/lower median
	const int numDec = 1;

	for (int i = nodes[layer].size() - 1; i > -1; i--)
	{
		GraphNode& node = nodes[layer][i];
		double newX = direction == "up" ? node.wLower : node.wUpper;

		if (node.virt)
		{
			newX = round(newX, numDec);
		}
		else
		{
			newX = std::round(newX / inc) * inc; // align to inc grid
		}

		// nodes can only be moved to the right, because after compacting we are already as far left as possible
		if (node.lx < newX)
		{
			if (i == static_cast<int>(nodes[layer].size() - 1)) // right most node can always move right
			{
				node.lx = newX;
				continue;
			}

			// shift left as long as pos is unoccupied
			// note: when using inc you also have to check for incVirt in between
			double lx = round(node.lx + incVirt, numDec);
			while (lx <= newX)
			{
				if (lx == nodes[layer][i + 1].lx) // pos already occupied
				{
					break;
				}

				double remainder = fmod(lx, inc);
				if (node.virt || remainder == 0)
				{
					node.lx = lx;
				}

				lx = round(lx + incVirt, numDec);
			}
		}
	}
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& container)
{
	os << "[ ";

	for (auto it = container.begin(); it != container.end(); ++it)
	{
		if (it != container.begin())
		{
			os << "; ";
		}

		os << *it;
	}

	os << " ]";

	return os;
}

template <typename Key, typename Value>
std::ostream& operator<<(std::ostream& os, const std::map<Key, Value>& container)
{
	os << "[ ";

	for (auto it = container.begin(); it != container.end(); ++it)
	{
		if (it != container.begin())
		{
			os << "; ";
		}

		os << "{ " << it->first << ", " << it->second << " }";
	}

	os << " ]";

	return os;
}

std::ostream& operator<<(std::ostream& os, const GraphLayout& layout)
{	
	return os
		<< "nodes: " << layout.nodes << "\n"
		<< "numLayer: " << layout.numLayer << "\n"
		<< "numPerLayers: " << layout.numPerLayers << "\n";
}

std::ostream& operator<<(std::ostream& os, const GraphLayout::GraphNode& node)
{
	os << "{ ";

	os
		<< std::boolalpha
		<< "label = " << (node.label.size()  == 0 ? "null" : node.label.toStdString().c_str()) << "; "
		<< "lx = " << node.lx << "; "
		<< "ly = " << node.ly << "; "
		<< "srcNodes = " << node.srcNodes << "; "
		<< "trgNodes = " << node.trgNodes << "; "
		<< "virt = " << node.virt << "; "
		<< "wLower = " << node.wLower << "; "
		<< "wUpper = " << node.wUpper << "; "
		<< "x = " << node.x << "; "
		<< "y = " << node.y;

	os << " }";

	return os;
}
