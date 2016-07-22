#pragma once


template<class T_STATE=State<>, class T_FEATURE_ID=ID>
class DB_Interface {
public:
	typedef T_STATE State;

	virtual typename State::Ptr query_state(const typename State::ID &id) = 0;
	virtual std::vector<typename State::Ptr> query_feature(const T_FEATURE_ID &id) = 0;
	virtual size_t count_feature(const T_FEATURE_ID &id) = 0;
	
	virtual bool insert(const typename State::Ptr &state) = 0;
};
