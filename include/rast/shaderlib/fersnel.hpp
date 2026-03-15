#pragma once
namespace rast::shaderlib::fersnel {
	template <typename T>
	inline T schlick(T cosTheta, T R0) {
		T a = T(1.0) - cosTheta;
		T b = a * a;
		return (T(1.0) - R0) * b * b * a + R0;
	}
	template <typename Vec3>
	Vec3 schlick(typename Vec3::value_type cosTheta, Vec3 F0) {
		return Vec3(
			schlick(cosTheta, F0[0]),
			schlick(cosTheta, F0[1]),
			schlick(cosTheta, F0[2])
		);
	}
}