#ifndef VECTOR_OPS_HH
#define VECTOR_OPS_HH

#include <iostream>
#include <vector>
#include <math.h>

void operator+=(std::vector<double>& u, const std::vector<double>& v);
void operator-=(std::vector<double>& u, const std::vector<double>& v);
void operator*=(std::vector<double>& u, const std::vector<double>& v);
void operator*=(std::vector<double>& u, const double& v);
void operator*=(const double& v, std::vector<double>& u);
std::vector<double> operator+(const std::vector<double>& u, const std::vector<double>& v);
std::vector<double> operator-(const std::vector<double>& u, const std::vector<double>& v);
std::vector<double> operator*(const std::vector<double>& u, const std::vector<double>& v);
std::vector<double> operator*(const std::vector<double>& u, const double& v);
std::vector<double> operator*(const double& v, const std::vector<double>& u);
double dot(const std::vector<double>& u, const std::vector<double>& v);
std::vector<double> cross(const std::vector<double>& u, const std::vector<double>& v);
double magnitude(std::vector<double>& u);
void normalise(std::vector<double>& u);
inline void debug(std::vector<double>* u) { std::cout << "["; std::cout << u->at(0) << ","; std::cout
		                                << u->at(1) << ","; std::cout << u->at(2) << "]" << std::endl; }

#endif
