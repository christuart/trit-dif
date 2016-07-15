#include "vector_ops.hh"

void operator+=(std::vector<float>& u, const std::vector<float>& v) { u = u+v; }
void operator-=(std::vector<float>& u, const std::vector<float>& v) { u = u-v; }
void operator*=(std::vector<float>& u, const std::vector<float>& v) { u = u*v; }
void operator*=(std::vector<float>& u, const float& v) { u = u*v; }
void operator*=(const float& v, std::vector<float>& u) { u = u*v; }

std::vector<float> operator+(const std::vector<float>& u, const std::vector<float>& v) {
	std::vector<float> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] += v[i];
	}
	return w;
}
std::vector<float> operator-(const std::vector<float>& u, const std::vector<float>& v) {
	std::vector<float> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] -= v[i];
	}
	return w;
}
std::vector<float> operator*(const std::vector<float>& u, const std::vector<float>& v) {
	std::vector<float> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] *= v[i];
	}
	return w;
}
std::vector<float> operator*(const std::vector<float>& u, const float& v) {
	std::vector<float> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = v*u[i];
	}
	return w;
}
std::vector<float> operator*(const float& v, const std::vector<float>& u) {
	return u*v;
}
float dot(const std::vector<float>& u, const std::vector<float>& v) {
	float w = 0.0f;
	for (int i = 0; i < u.size() && i < v.size(); i++) {
		w += v[i]*u[i];
	}
	return w;
}
std::vector<float> cross(const std::vector<float>& u, const std::vector<float>& v) {
	std::vector<float> w;
	w.reserve(3);
	for (int i = 0; i < u.size() && i < v.size(); i++) {
		
	}
	return w;
}
float magnitude(std::vector<float>& u) {
	float m = 0.0f;
	for (int i = 0; i < u.size(); i++) {
		m += u[i]*u[i];
	}
	return sqrt(m);
}
void normalise(std::vector<float>& u) {
	float m = magnitude(u);
	u *= (1/m);
}
