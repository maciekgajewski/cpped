#pragma once

#include "clang.hh"

#include <boost/filesystem.hpp>
#include <boost/container/flat_set.hpp>

namespace cpped { namespace clang {

boost::container::flat_set<boost::filesystem::path> get_includes(clang::translation_unit& tu);

}}
