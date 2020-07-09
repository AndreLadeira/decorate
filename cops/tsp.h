#ifndef ATSP_H
#define ATSP_H

#include <fstream>
#include <vector>

namespace problems{
namespace atsp{

using matrix = std::vector< std::vector<size_t> >;

void loadTSPLIB(std::ifstream& file, matrix & data);

}}
#endif // ATSP_H
