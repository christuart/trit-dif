#include "vector_ops.hh"

void operator+=(std::vector<double>& u, const std::vector<double>& v) { u = u+v; }
void operator-=(std::vector<double>& u, const std::vector<double>& v) { u = u-v; }
void operator*=(std::vector<double>& u, const std::vector<double>& v) { u = u*v; }
void operator*=(std::vector<double>& u, const double& v) { u = u*v; }
void operator*=(const double& v, std::vector<double>& u) { u = u*v; }

std::vector<double> operator+(const std::vector<double>& u, const std::vector<double>& v) {
	std::vector<double> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] += v[i];
	}
	return w;
}
std::vector<double> operator-(const std::vector<double>& u, const std::vector<double>& v) {
	std::vector<double> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] -= v[i];
	}
	return w;
}
std::vector<double> operator*(const std::vector<double>& u, const std::vector<double>& v) {
	std::vector<double> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] *= v[i];
	}
	return w;
}
std::vector<double> operator*(const std::vector<double>& u, const double& v) {
	std::vector<double> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = v*u[i];
	}
	return w;
}
std::vector<double> operator*(const double& v, const std::vector<double>& u) {
	return u*v;
}
double dot(const std::vector<double>& u, const std::vector<double>& v) {
	double w = 0.0f;
	for (int i = 0; i < u.size() && i < v.size(); i++) {
		w += v[i]*u[i];
	}
	return w;
}
std::vector<double> cross(const std::vector<double>& u, const std::vector<double>& v) {
	std::vector<double> w;
	w.resize(3);
	w.at(0) = u.at(1) * v.at(2) - u.at(2) * v.at(1);
	w.at(1) = u.at(2) * v.at(0) - u.at(0) * v.at(2);
	w.at(2) = u.at(0) * v.at(1) - u.at(1) * v.at(0);
	return w;
}
double magnitude(std::vector<double>& u) {
	double m = 0.0f;
	// std::cout << "u.size() = " << u.size() << std::endl;
	for (int i = 0; i < u.size(); i++) {
		m += u[i]*u[i];
	}
	return sqrt(m);
}
void normalise(std::vector<double>& u) {
	double m = magnitude(u);
	u *= (1/m);
}

ordering4 make_order(int _p1, int _p2, int _p3, int _p4) {
	ordering4 o;
	o.p1 = _p1; o.p2 = _p2; o.p3 = _p3; o.p4 = _p4;
	return o;
}
