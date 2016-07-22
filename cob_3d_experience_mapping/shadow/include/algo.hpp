#pragma once

#include "dfs.hpp"


/*
 * hyptotheses tree:
 *  in ideal case try each combination
 *  e. g.
 *    H1: [0,1]
 *      H2: [1, 2]
 *      H2: [1, 3]
 *      H3: [1, 1]
 *      H3: [1, 2]
 *      ....
 *    H1: [0,2]
 *      H2: [2, 3]
 *      H2: [2, 4]
 *      H2: [2, 5]
 *      H3: [2, 3]
 *      H3: [2, 4]
 *      ....
 *    H1: [0,3]
 *      ....
 *    H1: [0,4]
 *      ....
 *    ...
 * 
 * probably not possible for a queue size of >100 (->perforamce issue)
 * if probability of path is estimated by p(H1)*p(H2)*p(Hn)
 */
template<class T_FEATURE_ID=ID>
class MapContext
{
public:
	typedef T_FEATURE_ID;
	typedef T_FEATURE_DATA;
	typedef T_HYPOTHESIS;
	typedef T_FEATURE_ID::TypeGroupID TypeGroupID;
	typedef std::map<TypeGroupID, InputHistorySlot> History;
	
private:
	Head<> head_;
	History hist_;
	
	typedef T_SCALAR double;
	typedef DFS::Graph<typename T_STATE::Ptr, Head<>, T_SCALAR, InputHistorySlot> T_INTERNAL_HYPOTHESIS;
	std::vector<T_INTERNAL_HYPOTHESIS> hypos_;
	
	size_t history_size() const {return hist_.front().size();}
	
	void insert_slot() {
		
		//update hyptohesis
		for(hyp) {
			Scalar p=;
			for(History::iterator it=hist_.begin(); it!=hist_.end(); it++) {
				const Scalar pt = it->second.distinctiveness(hyp);
			}
			
			if(p>hyp.max_probability()) {
				if(hyp.end()+1 == history_size())
					hyp.end() = history_size();
				else
					insert_hypothesis(hyp);xyz
			}
			
		}
	}
public:

	void on_new_input(const T_FEATURE_ID &ft_id, const T_FEATURE_DATA &ft_data) {			
		hist[ft_id.type()].add_data(ft_data);
		
		if(hist[ft_id.type()].create_new_slot())
			insert_slot();
	}
};
