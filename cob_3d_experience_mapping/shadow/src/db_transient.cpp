#include "types.hpp"
#include "db_interface.hpp"


template<class T_STATE=State<>, class T_FEATURE_ID=ID>
class DB_Transient : public DB_Interface<T_STATE, T_FEATURE_ID> {
	typedef T_STATE State;
	
	struct S_FeatureContainer {
		std::vector<typename State::Ptr>	states_;
	};
	
	typedef std::map<typename State::ID, typename State::Ptr> 			MapStates;
	typedef std::map<T_FEATURE_ID, S_FeatureContainer>	MapFeatures;
	MapStates states_;
	MapFeatures ass_features_;
	size_t max_associations_;
	
public:

	DB_Transient():max_associations_(10) {}

	virtual typename State::Ptr query_state(const typename State::ID &id) {
		typename MapStates::iterator it = states_.find(id);
		if(it==states_.end()) return typename State::Ptr();
		return *it;
	}
	
	virtual std::vector<typename State::Ptr> query_feature(const T_FEATURE_ID &id) {
		typename MapFeatures::iterator it = ass_features_.find(id);
		if(it==states_.end() || it->states_.size()>max_associations_) return std::vector<typename State::Ptr>();
		return it->states_;
	}
	
	virtual size_t count_feature(const T_FEATURE_ID &id) {
		typename MapFeatures::iterator it = ass_features_.find(id);
		if(it==states_.end() || it->states_.size()>max_associations_) return 0;
		return it->states_.size();
	}
	
	virtual bool insert(const typename State::Ptr &state) {
		states_[state->id()] = state;
		
		std::vector<T_FEATURE_ID> fts = state->feature_ids();
		for(size_t i=0; i<fts.size(); i++)
			ass_features_[fts[i]].states_.push_back(state->id());		
	}
	
};

