#ifndef VEC_H
#define VEC_H

#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>

const double TAU = 6.283185307179586;

enum Dir { North = 0, South = 1, East = 2, West = 3 };
inline Dir flip(Dir dir) {
	switch (dir) {
		case North: return South;
		case South: return North;
		case East:  return West;
		case West:  return East;
	}
	assert(false);
}
inline Dir operator++(Dir& x) { return x = (Dir)((int)x + 1); }
inline std::ostream& operator<<(std::ostream& os, const Dir& obj) {
	switch (obj) {
		case North: os << "North"; break;
		case South: os << "South"; break;
		case East:  os << "East";  break;
		case West:  os << "West";  break;
	}
	return os;
}

template <typename N>
class Vector2 {
public:
	/** Creates a new Vector with the x and y coordinates the same. */
	explicit Vector2(N val = 0): x(val), y(val) { }
	/** Creates a new Vector with the given coordinates. */
	Vector2(N x, N y): x(x), y(y) { }
	/** Creates a new Vector from the given Dir. */
	explicit Vector2(Dir dir) {
		switch (dir) {
			case North: y = -1; break;
			case South: y =  1; break;
			case West:  x = -1; break;
			case East:  x =  1; break;
		}
	}
	/** Creates a new Vector from any vector-like. */
	template<typename M>
	explicit Vector2(Vector2<M> vec): x(vec.x), y(vec.y) { }

	/** Implicit conversion Vector to any vector-like */
	template<typename M>
	operator Vector2<M>() const {
		Vector2<M> v;
		v.x = x;
		v.y = y;
		return v;
	}

	/** @return The unit vector facing the given angle. */
	static Vector2<N> with_angle(double angle) {
		return Vector2<N>(cos(angle), sin(angle));
	}


	/** @return The dot product of this vector and rhs. */
	N dot(Vector2<N> rhs) const {
		return x * rhs.x + y * rhs.y;
	}

	/** @return The magnitude of the cross product of this vector and rhs. */
	N cross(Vector2<N> rhs) const {
		return x * rhs.y - rhs.x * y;
	}


	/** @return The magnitude of this vector. */
	double length() const {
		return sqrt(x * x + y * y);
	}

	/** @return The squared magnitude of this vector. */
	N sqr_length() const {
		return x * x + y * y;
	}

	/** @return This vector normalized to a unit vector. */
	Vector2<double> normalize() const {
		double len = length();
		if (len == 0) return Vector2<double>(0, 1);
		return Vector2<double>(x / len, y / len);
	}

	/** @return This vector rounded down to an int vector. */
	Vector2<int> floor() const {
		return Vector2<int>((int)::floor(x), (int)::floor(y));
	}

	/** @return The angle in radians of this vector, in range [-pi, pi]. */
	double angle() const {
		return atan2(y, x);
	}

	/** @return Vector2 where each element is absolute valued. */
	Vector2<N> abs() const {
		return Vector2<N>(std::abs(x), std::abs(y));
	}

	Vector2<N> swap() const {
		return Vector2<N>(y, x);
	}
	Vector2<N> max(Vector2<N> other) const {
		return Vector2<N>(std::max(x, other.x), std::max(y, other.y));
	}
	Vector2<N> min(Vector2<N> other) const {
		return Vector2<N>(std::min(x, other.x), std::min(y, other.y));
	}


	/** @return Whether or not the coordinates of this Vector and other are within threshold of each other. */
	bool equals(Vector2 other, double threshold) const {
		return std::abs(x - other.x) < threshold && std::abs(y - other.y) < threshold;
	}


	std::vector<Dir> dirs() const {
		std::vector<Dir> res;
		if      (y > 0) res.push_back(Dir::South);
		else if (y < 0) res.push_back(Dir::North);
		if      (x > 0) res.push_back(Dir::East);
		else if (x < 0) res.push_back(Dir::West);
		if (res.size() == 2 && std::abs(x) > std::abs(y)) std::swap(res[0], res[1]);
		return res;
	};


	/** Just adds the components. */
	Vector2<N>& operator+=(const Vector2<N>& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	/** Just subtracts the components. */
	Vector2<N>& operator-=(const Vector2<N>& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	/** Just multiplies the components. */
	Vector2<N>& operator*=(const Vector2<N>& rhs) {
		x *= rhs.x;
		y *= rhs.y;
		return *this;
	}
	/** Just divides the components. */
	Vector2<N>& operator/=(const Vector2<N>& rhs) {
		x /= rhs.x;
		y /= rhs.y;
		return *this;
	}
	/** Just adds rhs to each component. */
	Vector2<N>& operator+=(N rhs) {
		x += rhs;
		y += rhs;
		return *this;
	}
	/** Just subtracts rhs from each component. */
	Vector2<N>& operator-=(N rhs) {
		x -= rhs;
		y -= rhs;
		return *this;
	}
	/** Just multiplies each component by rhs. */
	Vector2<N>& operator*=(N rhs) {
		x *= rhs;
		y *= rhs;
		return *this;
	}
	/** Just divides each component by rhs. */
	Vector2<N>& operator/=(N rhs) {
		x /= rhs;
		y /= rhs;
		return *this;
	}
	/** Just returns the vector in the opposite direction. */
	Vector2<N> operator-() {
		return Vector2<N>(-x, -y);
	}

	/** x is at [0] and y is at [1] @{ */
	N& operator[](int idx) {
		if (idx) return y;
		return x;
	}
	N operator[](int idx) const {
		if (idx) return y;
		return x;
	}
	///@}

	/**
	 * Used for indexing 1D arrays representing 2D grids. Only works/make sense when
	 * used with an integer vector.
	 * @param width The width of the grid.
	 */
	inline N idx(N width) {
		return x + y * width;
	}


	N x = 0; ///< X coordinate of the vector.
	N y = 0; ///< Y coordinate of the vector.
};

template<typename N>
inline bool operator==(const Vector2<N>& lhs, const Vector2<N>& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}
template<typename N>
inline bool operator!=(const Vector2<N>& lhs, const Vector2<N>& rhs) {
	return !operator==(lhs, rhs);
}

template<typename N>
inline Vector2<N> operator+(Vector2<N> lhs, const Vector2<N>& rhs) {lhs += rhs; return lhs;}
template<typename N>
inline Vector2<N> operator-(Vector2<N> lhs, const Vector2<N>& rhs) {lhs -= rhs; return lhs;}
template<typename N>
inline Vector2<N> operator*(Vector2<N> lhs, const Vector2<N>& rhs) {lhs *= rhs; return lhs;}
template<typename N>
inline Vector2<N> operator/(Vector2<N> lhs, const Vector2<N>& rhs) {lhs /= rhs; return lhs;}
template<typename N>
inline Vector2<N> operator+(Vector2<N> lhs, double rhs) {lhs += rhs; return lhs;}
template<typename N>
inline Vector2<N> operator-(Vector2<N> lhs, double rhs) {lhs -= rhs; return lhs;}
template<typename N>
inline Vector2<N> operator*(Vector2<N> lhs, double rhs) {lhs *= rhs; return lhs;}
template<typename N>
inline Vector2<N> operator/(Vector2<N> lhs, double rhs) {lhs /= rhs; return lhs;}

template<typename N>
std::ostream& operator<<(std::ostream& os, const Vector2<N>& obj) {
	os << "(" << obj.x << ", " << obj.y << ")";
	return os;
}



/** Basic 3D vector class so that stuff need not depend on IrrLicht's vector class. */
template <typename N>
class Vector3 {
public:
	/** Creates a new Vector3 with the x, y and z coordinates the same. */
	Vector3(N val = 0): x(val), y(val), z(val) { }
	/** Creates a new Vector3 with the given Vector2 and z coordinate. */
	Vector3(Vector2<N> v, N z): x(v.x), y(v.y), z(z) { }
	/** Creates a new Vector3 with the given coordinates. */
	Vector3(N x, N y, N z): x(x), y(y), z(z) { }
	/** Creates a new Vector3 from any vector-like. */
	template<typename M>
	Vector3(Vector3<M> vec): x(vec.x), y(vec.y), z(vec.z) { }

	/** Implicit conversion Vector3 to any vector-like */
	template<typename M>
	operator Vector3<M>() const {
		Vector3<M> v;
		v.x = x;
		v.y = y;
		v.z = z;
		return v;
	}


	/** @return The 2D component of this vector (x, y) */
	Vector2<N> flat() const {
		return Vector2<N>(x, y);
	}


	/** @return The dot product of this vector and rhs. */
	N dot(Vector3<N> rhs) const {
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	/** @return The cross product of this vector and rhs. */
	N cross(Vector3<N> rhs) const {
		return Vector3<N>(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.z);
	}


	/** @return The magnitude of this vector. */
	double length() const {
		return sqrt(x * x + y * y + z * z);
	}

	/** @return The squared magnitude of this vector. */
	N sqr_length() const {
		return x * x + y * y + z * z;
	}

	/** @return This vector normalized to a unit vector. */
	Vector3<double> normalize() const {
		double len = length();
		if (len == 0) return Vector3<double>(0, 0, 1);
		return Vector3<double>(x / len, y / len, z / len);
	}

	/** @return This vector rounded down to an int vector. */
	Vector3<int> floor() const {
		return Vector3<int>((int)::floor(x), (int)::floor(y), (int)::floor(y));
	}

	/** @return Vector3 where each element is absolute valued. */
	Vector3<N> abs() const {
		return Vector3<N>(std::abs(x), std::abs(y), std::abs(z));
	}

	/** @return Vector3 where each element is the minimum of each from self and rhs. */
	Vector3<N> min(Vector3<N> rhs) const {
		return Vector3<N>(std::min(x, rhs.x), std::min(y, rhs.y), std::min(z, rhs.z));
	}


	/** @return Whether or not the coordinates of this Vector3 and other are within threshold of each other. */
	bool equals(Vector3 other, double threshold) const {
		return std::abs(x - other.x) < threshold &&
		       std::abs(y - other.y) < threshold &&
		       std::abs(z - other.z) < threshold;
	}


	/** Just adds the components. */
	Vector3<N>& operator+=(const Vector3<N>& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
	/** Just subtracts the components. */
	Vector3<N>& operator-=(const Vector3<N>& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}
	/** Just multiplies the components. */
	Vector3<N>& operator*=(const Vector3<N>& rhs) {
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}
	/** Just divides the components. */
	Vector3<N>& operator/=(const Vector3<N>& rhs) {
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		return *this;
	}
	/** Just adds rhs to each component. */
	Vector3<N>& operator+=(N rhs) {
		x += rhs;
		y += rhs;
		z += rhs;
		return *this;
	}
	/** Just subtracts rhs from each component. */
	Vector3<N>& operator-=(N rhs) {
		x -= rhs;
		y -= rhs;
		z -= rhs;
		return *this;
	}
	/** Just multiplies each component by rhs. */
	Vector3<N>& operator*=(N rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}
	/** Just divides each component by rhs. */
	Vector3<N>& operator/=(N rhs) {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}
	/** Just returns the vector in the opposite direction. */
	Vector3<N> operator-() {
		return Vector3<N>(-x, -y, -z);
	}

	/** x is at [0] and y is at [1] @{ */
	N& operator[](int idx) {
		if (!idx) return x;
		if (idx == 2) return z;
		return y;
	}
	N operator[](int idx) const {
		if (!idx) return x;
		if (idx == 2) return z;
		return y;
	}
	///@}


	N x; ///< X coordinate of the vector.
	N y; ///< Y coordinate of the vector.
	N z; ///< Z coordinate of the vector.
};

template<typename N>
inline bool operator==(const Vector3<N>& lhs, const Vector3<N>& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}
template<typename N>
inline bool operator!=(const Vector3<N>& lhs, const Vector3<N>& rhs) {
	return !operator==(lhs, rhs);
}

template<typename N>
inline Vector3<N> operator+(Vector3<N> lhs, const Vector3<N>& rhs) {lhs += rhs; return lhs;}
template<typename N>
inline Vector3<N> operator-(Vector3<N> lhs, const Vector3<N>& rhs) {lhs -= rhs; return lhs;}
template<typename N>
inline Vector3<N> operator*(Vector3<N> lhs, const Vector3<N>& rhs) {lhs *= rhs; return lhs;}
template<typename N>
inline Vector3<N> operator/(Vector3<N> lhs, const Vector3<N>& rhs) {lhs /= rhs; return lhs;}
template<typename N>
inline Vector3<N> operator+(Vector3<N> lhs, double rhs) {lhs += rhs; return lhs;}
template<typename N>
inline Vector3<N> operator-(Vector3<N> lhs, double rhs) {lhs -= rhs; return lhs;}
template<typename N>
inline Vector3<N> operator*(Vector3<N> lhs, double rhs) {lhs *= rhs; return lhs;}
template<typename N>
inline Vector3<N> operator/(Vector3<N> lhs, double rhs) {lhs /= rhs; return lhs;}

template<typename N>
std::ostream& operator<<(std::ostream& os, const Vector3<N>& obj) {
	os << "(" << obj.x << ", " << obj.y << ", " << obj.z << ")";
	return os;
}


// typedefs for common vectors
typedef Vector2<double> Vec2;
typedef Vector3<double> Vec3;
typedef Vector2<int>    Pos2;
typedef Vector3<int>    Pos3;

#endif // VEC_H


