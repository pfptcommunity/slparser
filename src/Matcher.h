/**
 * This code was tested against C++20
 *
 * @author Ludvik Jerabek
 * @package slanalyzer
 * @version 1.0.0
 * @license MIT
 */
#ifndef SLANALYZER_MATCHER_H
#define SLANALYZER_MATCHER_H

#include "IMatcher.h"
#include "re2/re2.h"
#include "re2/set.h"
#include <memory>
#include <unordered_map>

namespace Proofpoint {

template <typename T>
class Matcher : public IMatcher<T> {
public:
	explicit Matcher(bool literal = false, bool case_sensitive = false, RE2::Anchor anchor = RE2::ANCHOR_BOTH);
	~Matcher() { delete match; }
public:
	void Add(const std::string& pattern, const T& index, PatternErrors<T>& pattern_errors) override;
	bool Match(const std::string& pattern, std::vector<T>& match_indexes) override;
	std::size_t GetPatternCount() override;
private:
	bool compiled;
	RE2::Options opt;
	RE2::Set* match;
	std::unordered_map<int, T> map_to_global_list;
};


template<typename T>
Proofpoint::Matcher<T>::Matcher(bool literal, bool case_sensitive, RE2::Anchor anchor) : compiled(false)
{
	opt.set_literal(literal);
	opt.set_case_sensitive(case_sensitive);
	opt.set_log_errors(false);
	match = new RE2::Set(opt, anchor);
}

template<typename T>
void Proofpoint::Matcher<T>::Add(const std::string& pattern, const T& index, PatternErrors<T>& pattern_errors)
{
	std::string error;
	int i = match->Add(pattern, &error);
	if (i==-1) {
		pattern_errors.push_back({index, pattern, error});
		return;
	}
	map_to_global_list.insert({i, index});
}

template<typename T>
bool Proofpoint::Matcher<T>::Match(const std::string& pattern, std::vector<T>& match_indexes)
{
	match_indexes.clear();
	if (!compiled) {
		match->Compile();
		compiled = true;
	}
	std::vector<int> m;
	bool matched = match->Match(pattern, &m);
	for (auto index : m) {
		match_indexes.emplace_back(map_to_global_list[index]);
	}
	return matched;
}

template<typename T>
std::size_t Proofpoint::Matcher<T>::GetPatternCount()
{
	return map_to_global_list.size();
}

}
#endif //SLANALYZER_MATCHER_H
