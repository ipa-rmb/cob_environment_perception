#pragma once


template<class T_STATE_ID=ID, class T_FEATURE_ID=ID>
class DB_Interface {
public:

	virtual State::Ptr query_state(const T_STATE_ID &id) = 0;
	virtual std::vector<State::Ptr> query_feature(const T_FEATURE_ID &id) = 0;
	virtual size_t count_feature(const T_FEATURE_ID &id) = 0;
	
	virtual bool insert(const State::Ptr &state) = 0;
};
