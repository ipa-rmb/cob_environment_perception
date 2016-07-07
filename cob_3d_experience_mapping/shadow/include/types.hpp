#pragma once

struct ID {
	union UID {
		int64_t val_;
		int32_t ids_[2];
	} id_;
	
	typedef int32_t TypeGroupID;
	
	ID(const int32_t _type, const int32_t _id) {
		type() = _type;
		id() = _id;
	}
	
	TypeGroupID &type() {return id_.ids_[0];}
	int32_t &id() {return id_.ids_[1];}
	
	TypeGroupID type() const {return id_.ids_[0];}
	int32_t id() const {return id_.ids_[1];}
};

template<class T_SCALAR=double>
class Transformation {
	typedef T_SCALAR Scalar;
	typedef boost::shared_ptr<Transformation> Ptr;
	
	union UTF {
		Scalar		vals_[3];
		Scalar		x_, y_, yaw_;
	} tf_;
};

template<class T_STATE_ID=ID, class T_FEATURE_ID=ID, class T_TRANSFORMATION=Transformation, class T_SCALAR=double>
class State {
	typedef T_SCALAR Scalar;
	typedef boost::shared_ptr<State> Ptr;
	
	struct S_AssociatedFeature {
		T_FEATURE_ID	ft_id_;
		Scalar		rel_occ_;
	};
	
	T_STATE_ID 							id_;
	State::Ptr 							source_;
	std::vector<S_AssociatedFeature>	features_;
	T_TRANSFORMATION					tf_;
	
public:

	T_STATE_ID id() const {return id_;}
	std::vector<T_FEATURE_ID> feature_ids() const {
		std::vector<T_FEATURE_ID> r;
		for(size_t i=0; i<features_.size(); i++) r.push_back(features_[i].ft_id_);
		return r;
	}
};

template<class T_STATE=State, class T_TRANSFORMATION=Transformation>
class Head {
	T_TRANSFORMATION offset_;
	State::Ptr last_active_state_;
};

template<class T_FEATURE_ID=ID, class T_FEATURE_DATA=void, class T_SCALAR=double>
class InputHistorySlot {
	struct S_Content {
		T_FEATURE_ID id_;
		T_FEATURE_DATA data_;
	};
	
	std::vector<S_Content> slots_;
	
	virtual () = 0;
	
	inline size_t size() const {return slots_.size();}
	
	template<class T_HYPOTHESIS>
	virtual T_SCALAR distinctiveness(const T_HYPOTHESIS &hyp) = 0;
	virtual bool create_new_slot() const {return false;}
};


/*
 * functions:
 * 
 *  - on_new_input(FT_ID, DATA)
 *  - insert_slot()
 */
