#pragma once

#include <string>
#include <cassert>

namespace cpped
{

class clipboard
{
public:

	clipboard();
	~clipboard();

	static bool empty() { assert(instance_); return instance_->content_.empty(); }
	static void set(const std::string& text) { assert(instance_); instance_->content_ = text; }
	static const std::string& get(){ assert(instance_); return instance_->content_; }

private:

	static clipboard* instance_;
	std::string content_;
};

}
