#include <iostream>
#include <mutex>
#include <memory>

struct  Node;

struct Node
{
	Node(int value, std::shared_ptr<Node> node_ptr) : _value(value), _next(node_ptr), _node_mutex(std::make_unique<std::mutex>()) { }
	int _value;
	std::shared_ptr<Node> _next;
	std::unique_ptr<std::mutex> _node_mutex;
};

class FineGrainedQueue
{
public:
	FineGrainedQueue(std::shared_ptr<Node> head) : _head(head), _queue_mutex(std::make_unique<std::mutex>()) {}
	auto insertIntoMiddle(int value, int pos) -> void
	{
		if (pos <= 0) return;

		std::shared_ptr<Node> old_prev_node, prev_node, cur_node, new_node;

		_queue_mutex->lock();
		prev_node = _head;
		cur_node = _head->_next;
		prev_node->_node_mutex->lock();
		_queue_mutex->unlock();

		if (cur_node) cur_node->_node_mutex->lock(); // only head exists
		else
		{
			new_node = std::make_shared<Node>(value, nullptr);
			_head->_next = new_node;
			_head->_node_mutex->unlock();
			return;
		}

		auto counter{ 0 };
		while (cur_node)
		{
			old_prev_node = prev_node;
			prev_node = cur_node;
			cur_node = prev_node->_next;

			if (cur_node)
			{
				old_prev_node->_node_mutex->unlock();
				cur_node->_node_mutex->lock();
			}
			if (++counter == pos) break;
		}
		if (cur_node && counter == pos) // stop in middle
		{
			new_node = std::make_shared<Node>(value, prev_node);
			old_prev_node->_next = new_node;
			prev_node->_node_mutex->unlock();
			cur_node->_node_mutex->unlock();
		}
		else if(!cur_node && counter == pos) // stop in last 
		{
			new_node = std::make_shared<Node>(value, prev_node);
			old_prev_node->_next = new_node;
			old_prev_node->_node_mutex->unlock();
			prev_node->_node_mutex->unlock();
		}
		else if (!cur_node && counter != pos) // pos > list_size
		{
			old_prev_node->_node_mutex->unlock();
			new_node = std::make_shared<Node>(value, nullptr);
			prev_node->_next = new_node;
			prev_node->_node_mutex->unlock();
		}
	}
private:
	std::shared_ptr<Node> _head;
	std::unique_ptr<std::mutex> _queue_mutex;
};

auto main() -> int
{
	std::shared_ptr<Node> head = std::make_shared<Node>(0, nullptr);

	FineGrainedQueue fgq(head);

	fgq.insertIntoMiddle(2, 5);

	fgq.insertIntoMiddle(1, 1);

	fgq.insertIntoMiddle(5, 7);

	fgq.insertIntoMiddle(4, 3);

	fgq.insertIntoMiddle(3, 3);

	return 0;
}

