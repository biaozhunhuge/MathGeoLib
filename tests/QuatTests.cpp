#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../src/MathGeoLib.h"
#include "../src/Math/myassert.h"
#include "TestRunner.h"
#include "TestData.h"

#include "../src/Math/SSEMath.h"
#include "../src/Math/quat_simd.h"

MATH_IGNORE_UNUSED_VARS_WARNING

using namespace TestData;

BENCHMARK(Quat_Transform_float3, "Quat::Transform(float3)")
{
	v2[i].Float3Part() = q[i].Transform(v[i].Float3Part());
}
BENCHMARK_END;

#ifdef MATH_SSE
BENCHMARK(quat_transform_vec4, "quat_transform_vec4")
{
	v2[i] = quat_transform_vec4(q[i].q, v01[i].v);
}
BENCHMARK_END;
#endif

BENCHMARK(Quat_Transform_float4, "Quat::Transform(float4)")
{
	v2[i] = q[i].Transform(v01[i]);
}
BENCHMARK_END;

BENCHMARK(Quat_SetFromAxisAngle_float4, "Quat::SetFromAxisAngle(float4,float)")
{
	q2[i].SetFromAxisAngle(nv[i], f[i]);
}
BENCHMARK_END;

BENCHMARK(Quat_ToAxisAngle_float3, "Quat::ToAxisAngle(float3,float)")
{
	q2[i].ToAxisAngle(*(float3*)&v3[i], f[i]);
}
BENCHMARK_END;

BENCHMARK(Quat_ToAxisAngle_float4, "Quat::ToAxisAngle(float4,float)")
{
	q2[i].ToAxisAngle(v3[i], f[i]);
}
BENCHMARK_END;

BENCHMARK(Quat_Axis, "Quat::Axis()")
{
	ve[i] = q2[i].Axis();
}
BENCHMARK_END;

BENCHMARK(Quat_to_float4x4, "Quat::ToFloat4x4")
{
	m[i] = q[i].ToFloat4x4();
}
BENCHMARK_END;

BENCHMARK(Quat_Slerp, "Quat::Slerp")
{
	q2[i] = q[i].Slerp(q2[i], uf[i]);
}
BENCHMARK_END;

RANDOMIZED_TEST(Quat_SetFromAxisAngle)
{
	float3 axis = float3::RandomDir(rng);
	float4 axis4(axis, 0.f);
	float f = rng.Float(-10000.f, 10000.f);
	Quat q, q2;
	q.SetFromAxisAngle(axis, f);
	q2.SetFromAxisAngle(axis4, f);
	assert2(q.Equals(q2), q, q2);
}

RANDOMIZED_TEST(Quat_Transform)
{
	Quat q = Quat::RandomRotation(rng);
	float3x3 m = float3x3(q);
	float3 v = float3::RandomSphere(rng, float3(0,0,0), 100.f);
	float3 mv = m*v;
	float3 qv = q*v;
	float3 qv2 = q.Transform(v);
	assert(mv.Equals(qv));
	assert(qv.Equals(qv2));

	float4 V(v, (float)rng.Int(0, 1));
	float4x4 M = float4x4(q);
	float4 MV = M*V;
	float4 qV = q*V;
	float4 qV2 = q.Transform(V);
	assert(MV.Equals(qV));
	assert(MV.Equals(qV2));
}

RANDOMIZED_TEST(Quat_float4x4_conv)
{
	Quat q = Quat::RandomRotation(rng);
	float4x4 m = q.ToFloat4x4();
	assert(m.TranslatePart().Equals(0,0,0));
	assert(!m.ContainsProjection());
	Quat q2 = m.Float3x3Part().ToQuat();
	assert(q.Equals(q2) || q.Equals(q2.Neg()));

	float4 v = float4::RandomGeneral(rng, -1000.f, 1000.f);
	v.w = (float)rng.Int(0,1);
	m = q.ToFloat4x4(v);
	assert(m.TranslatePart().Equals(v.xyz()));
	assert(!m.ContainsProjection());
	q2 = m.Float3x3Part().ToQuat();
	assert(q.Equals(q2) || q.Equals(q2.Neg()));
}

BENCHMARK(Quat_op_mul_Quat, "Quat * Quat")
{
	q2[i] = q[i] * q2[i];
}
BENCHMARK_END

BENCHMARK(Quat_op_div_Quat, "Quat / Quat")
{
	q2[i] = q[i] / q2[i];
}
BENCHMARK_END

BENCHMARK(Quat_Lerp, "Quat::Lerp")
{
	q2[i] = q[i].Lerp(q2[i], uf[i]);
}
BENCHMARK_END

BENCHMARK(Quat_RotateFromTo, "Quat::RotateFromTo")
{
	q2[i] = Quat::RotateFromTo(nv[i], nv2[i]);
}
BENCHMARK_END

#ifdef MATH_SIMD
BENCHMARK(quat_mul_quat, "test against Quat_op_mul_Quat")
{
	q2[i].q = quat_mul_quat(q[i].q, q2[i].q);
}
BENCHMARK_END

#ifdef ANDROID
BENCHMARK(quat_mul_quat_asm, "test against Quat_op_mul_Quat")
{
	quat_mul_quat_asm(q[i].ptr(), q2[i].ptr(), q2[i].ptr());
}
BENCHMARK_END
#endif

RANDOMIZED_TEST(quat_mul_quat)
{
	Quat q = Quat::RandomRotation(rng);
	Quat q2 = Quat::RandomRotation(rng);
	Quat correct = q * q2;
	Quat q3 = quat_mul_quat(q.q, q2.q);
	assert(q3.Equals(correct));
}

RANDOMIZED_TEST(Quat_ToAxisAngle_float3)
{
	Quat q = Quat::RandomRotation(rng);
	float3 axis;
	float angle;
	q.ToAxisAngle(axis, angle);
	Quat q2;
	q2.SetFromAxisAngle(axis, angle);
	assert(q.Equals(q2));
}

RANDOMIZED_TEST(Quat_ToAxisAngle_float4)
{
	Quat q = Quat::RandomRotation(rng);
	float4 axis;
	float angle;
	q.ToAxisAngle(axis, angle);

	vec axis2 = q.Axis();
	assert(axis2.xyz().Equals(axis.xyz()));
	Quat q2;
	q2.SetFromAxisAngle(axis, angle);
	assert(q.Equals(q2));
}

RANDOMIZED_TEST(Quat_RotateFromTo_float3)
{
	float3 v = float3::RandomDir(rng);
	float3 v2 = float3::RandomDir(rng);
	Quat rot = Quat::RotateFromTo(v, v2);
	float3 v2_ = rot * v;
	assert2(v2.Equals(v2_), v2, v2_);
}

RANDOMIZED_TEST(Quat_RotateFromTo_float4)
{
	float4 v = float4::RandomDir(rng);
	float4 v2 = float4::RandomDir(rng);
	Quat rot = Quat::RotateFromTo(v, v2);
	float4 v2_ = rot * v;
	assert2(v2.Equals(v2_), v2, v2_);
}

TEST(QuatNormalize)
{
	Quat q(-1.f, 2.f, 3.f, 4.f);
	float oldLength = q.Normalize();
	MARK_UNUSED(oldLength);
	assertcmp(oldLength, >, 0);
	assert(EqualAbs(q.x, -1.f / Sqrt(30.f)));
	assert(EqualAbs(q.y, 2.f / Sqrt(30.f)));
	assert(EqualAbs(q.z, 3.f / Sqrt(30.f)));
	assert(EqualAbs(q.w, 4.f / Sqrt(30.f)));

	Quat q2(0,0,0, 0.f);
	oldLength = q2.Normalize();
	MARK_UNUSED(oldLength);
	assert(oldLength == 0.f);
	assert(q2.x == 1.f);
	assert(q2.y == 0.f);
	assert(q2.z == 0.f);
	assert(q2.w == 0.f);
}

TEST(QuatNormalized)
{
	Quat q(-1.f, 2.f, 3.f, -4.f);
	Quat q2 = q.Normalized();
	assert(EqualAbs(q2.x, -1.f / Sqrt(30.f)));
	assert(EqualAbs(q2.y, 2.f / Sqrt(30.f)));
	assert(EqualAbs(q2.z, 3.f / Sqrt(30.f)));
	assert(EqualAbs(q2.w, -4.f / Sqrt(30.f)));
}

#ifdef ANDROID
RANDOMIZED_TEST(quat_mul_quat_asm)
{
	Quat q = Quat::RandomRotation(rng);
	Quat q2 = Quat::RandomRotation(rng);
	Quat correct = q * q2;
	Quat q3;
	quat_mul_quat_asm(q.ptr(), q2.ptr(), q3.ptr());
	assert(q3.Equals(correct));
}
#endif

#ifdef MATH_SSE
BENCHMARK(quat_div_quat, "test against Quat_op_div_Quat")
{
	q2[i].q = quat_div_quat(q[i].q, q2[i].q);
}
BENCHMARK_END

RANDOMIZED_TEST(quat_div_quat)
{
	Quat q = Quat::RandomRotation(rng);
	Quat q2 = Quat::RandomRotation(rng);
	Quat correct = q / q2;
	Quat q3 = quat_div_quat(q.q, q2.q);
	assert(q3.Equals(correct));
}
#endif
BENCHMARK(quat_lerp_simd, "test against Quat_Lerp")
{
	q2[i].q = vec4_lerp(q[i].q, q2[i].q, uf[i]);
}
BENCHMARK_END

RANDOMIZED_TEST(quat_lerp)
{
	Quat q = Quat::RandomRotation(rng);
	Quat q2 = Quat::RandomRotation(rng);
	float t = rng.Float();
	Quat correct = q.Lerp(q2, t);
	Quat q3 = vec4_lerp(q.q, q2.q, t);
	assert(q3.Equals(correct));
}
#endif
