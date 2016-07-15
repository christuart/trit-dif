#ifndef VECTOR_OPS_HH
#define VECTOR_OPS_HH

#include <iostream>
#include <vector>
#include <math.h>

void operator+=(std::vector<float>& u, const std::vector<float>& v);
void operator-=(std::vector<float>& u, const std::vector<float>& v);
void operator*=(std::vector<float>& u, const std::vector<float>& v);
void operator*=(std::vector<float>& u, const float& v);
void operator*=(const float& v, std::vector<float>& u);
std::vector<float> operator+(const std::vector<float>& u, const std::vector<float>& v);
std::vector<float> operator-(const std::vector<float>& u, const std::vector<float>& v);
std::vector<float> operator*(const std::vector<float>& u, const std::vector<float>& v);
std::vector<float> operator*(const std::vector<float>& u, const float& v);
std::vector<float> operator*(const float& v, const std::vector<float>& u);
float dot(const std::vector<float>& u, const std::vector<float>& v);
std::vector<float> cross(const std::vector<float>& u, const std::vector<float>& v);
float magnitude(std::vector<float>& u);
void normalise(std::vector<float>& u);
inline void debug(std::vector<float>* u) { std::cout << "["; std::cout << u->at(0) << ","; std::cout
		                                << u->at(1) << ","; std::cout << u->at(2) << "]" << std::endl; }

#endif
