#pragma once

template<class T_SCALAR=double>
struct InputHistorySlot_Odometry {
	typedef T_SCALAR								Scalar;
	typedef ros::Time								T_FEATURE_ID;
	typedef Eigen::Matrix<Scalar, Dynamic, Dynamic>	T_FEATURE_DATA;
	
	template<class T_HYPOTHESIS, class S_CONTENT>
	static T_SCALAR distinctiveness(const T_HYPOTHESIS &hyp, const std::vector<S_CONTENT> &slots) {
		const S_CONTENT &head = slots[hyp.begin_index()];
		const S_CONTENT &tail = slots[hyp.end_index()-1 ];
		
		return 1-std::exp( (head.data_.inverse() * tail.data_).norm() );
	}
	
};

template<class T_FEATURE_CONTENT_ID, class T_SCALAR=double>
struct Histogram : public std::map<T_FEATURE_CONTENT_ID, size_t> {
	
	void operator+=(const std::vector<T_FEATURE_CONTENT_ID> &ft_ids) {
		for(size_t i=0; i<ft_ids.size(); i++) {
			iterator it = find(ft_ids[i]);
			if(it==end())
				(*this)[ft_ids[i]] = 1;
			else
				it->second++;
		}		
	}
	
	size_t count(const T_FEATURE_CONTENT_ID &ft_id) const {
		iterator it = find(ft_ids[i]);
		if(it==end())
			return 0;
		return it->second;
	}
	
	T_SCALAR operator&(const Histogram &o) const {
		return chi_square(o)+o.chi_square(*this);
	}
	
	T_SCALAR chi_square(const Histogram &o) const {
		T_SCALAR r = 0;
		for(const_iterator it=begin(); it!=end(); it++) {
			r += std::pow( it->second-o.count(it->first) , 2) / it->second;
		}
		return r;
	}
	
};


template<class T_FEATURE_CONTENT_ID, class T_SCALAR=double>
struct InputHistorySlot_Feature {
	typedef T_SCALAR								Scalar;
	typedef ros::Time								T_FEATURE_ID;
	typedef std::vector<T_FEATURE_CONTENT_ID>		T_FEATURE_DATA;
	
	template<class T_HYPOTHESIS, class S_CONTENT>
	static T_SCALAR distinctiveness(const T_HYPOTHESIS &hyp, const std::vector<S_CONTENT> &slots) {
		const size_t mid = (hyp.begin_index()+hyp.end_index())/2;
		Histogram hists[3];
		T_SCALAR c = 1;
		
		for(size_t i=hyp.begin_index(); i<hyp.end_index(); i++) {
			hists[i>=mid?1:0] += slots[i].data_;
			hists[2] += slots[i].data_;
		}
		
		T_SCALAR c = 1;
		for(Histogram::const_iterator it=hists[2].begin(); it!=hists[2].end(); it++) {
			c *= 1-it->second/db.count_feature(it->first);
		}
			
		const T_SCALAR s = hists[0]&hists[1];
		
		return (1-s)+s*c;
	}
	
};
