#pragma once

#include "event_window.hh"

#include <boost/signals2.hpp>

#include <array>

namespace nct {

class splitter_section;

// Splitter is a window split verticall or horizontally into sections
class splitter : public nct::event_window
{
public:

	enum direction { horizontal, vertical };

	splitter(window_manager& wm, event_window* parent);

	void set_main_section(splitter_section& section, direction dir);

private:

	void on_resized() override;
	void render(nct::ncurses_window& surface) override;

	splitter_section* main_section_ = nullptr;
	direction direction_;
};

// Single splitter item
class splitter_item
{
public:
	enum class geometry { fixed, stretching };

	// creates sretching item
	splitter_item(splitter& sp, nct::event_window& window);

	// creates fixed-size item
	splitter_item(splitter& sp, nct::event_window& window, unsigned preferred_size);

	bool is_visible() const;

protected:

	splitter_item(splitter& sp) : splitter_(sp), geometry_(geometry::stretching) {}
	splitter_item(splitter& sp, unsigned preferred_size)
		: splitter_(sp)
		, geometry_(geometry::stretching)
		, preferred_size_(preferred_size)
		{ }

	bool is_section() const { return window_ == nullptr; }
	virtual void apply_size(const nct::position& pos, const nct::size& sz);

	friend class splitter_section;

	splitter& splitter_;
	geometry geometry_;
	unsigned preferred_size_;
	bool visible_ = true;
	unsigned size_; // current size

private:

	nct::event_window* window_ = nullptr;

};

class splitter_section : public splitter_item
{
public:

	// creates stretching section
	splitter_section(splitter& sp);

	// creates fixed-size sections
	splitter_section(splitter& sp, unsigned preferred_size);

	void add_item(splitter_item& item);

	template<splitter::direction DIR>
	void recalc_size(const nct::size& sz);

private:

	template<splitter::direction DIR>
	void apply_sizes();

	void apply_size(const nct::position& pos, const nct::size& sz) final override;

	std::vector<splitter_item*> items_;
	nct::position position_;
};

}
