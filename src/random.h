#pragma once

#include <chrono>
#include <random>

namespace Random {
	inline std::random_device rd{};

	inline std::mt19937 generate(){
		std::seed_seq ss{
			static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
				rd(), rd(), rd(), rd(), rd(), rd(), rd() };
		return std::mt19937{ ss };
	}
	
	inline std::mt19937 mt{ generate() }; 

	inline void seed() {
		std::seed_seq ss{
			static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
				rd(), rd(), rd(), rd(), rd(), rd(), rd() };
		mt.seed(ss);
	}

	template <class T>
	void seed(T seed) {
		mt.seed(seed);
	}

	inline int geti(int min, int max){
		return std::uniform_int_distribution{min, max}(mt);
	}

	template <class T>
	T geti(T min, T max) {
		return std::uniform_int_distribution<T>{min, max}(mt);
	}

	template <class R, class S, class T>
	R geti(S min, T max) {
		return geti<R>(static_cast<R>(min), static_cast<R>(max));
	}

	inline float getr(float min, float max) {
		return std::uniform_real_distribution{ min, max }(mt);
	}

	template <class R, class S, class T>
	T getr(S min, T max) {
		return getr<R>(static_cast<S>(min), static_cast<R>(max));
	}

	template <class T>
	T getr(T min, T max) {
		return std::uniform_real_distribution<T>{min, max};
	}
}
