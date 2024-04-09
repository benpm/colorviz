#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <optional>
#include <algorithm>
#ifdef PLATFORM_WINDOWS
    #undef near
    #undef far
#endif

inline constexpr double pi = 3.141592653589793238462643383279502884;
inline constexpr float tau = (float)(pi * 2.0);
inline constexpr float tau2 = (float)pi;
inline constexpr float tau4 = (float)(pi / 2.0);

using namespace Eigen;
using Transform3f = Transform<float, 3, Affine>;
using Vector2u = Vector2<uint32_t>;
using Vector3u = Vector3<uint32_t>;
using Vector4u = Vector4<uint32_t>;
using MatrixXu = Matrix<uint32_t, Dynamic, Dynamic>;
using RowVectorXu = Matrix<uint32_t, 1, Dynamic>;
using VectorXu = Matrix<uint32_t, Dynamic, 1>;

template <typename T>
concept c_Matrix = std::is_base_of_v<DenseBase<T>, T>;
template <typename T>
concept c_Vector = c_Matrix<T> &&
                       requires(T)
{
    T::RowsAtCompileTime == 1 || T::ColsAtCompileTime == 1;
};

enum class Axis : uint8_t
{
    X = 0u,
    Y = 1u,
    Z = 2u,
};

enum class AxisPlane : uint8_t
{
    XY = 3u,
    XZ = 4u,
    YZ = 5u,
};

// Represents a 3D infinite plane
struct Plane
{
    Vector3f origin = { 0.0f, 0.0f, 0.0f };
    Vector3f normal = { 0.0f, 0.0f, 0.0f };

    // Creates a plane with invalid origin and normal
    Plane() = default;
    // Creates a plane with given origin and normal
    Plane(const Vector3f& origin, const Vector3f& normal);
    // Creates an axis-aligned plane at the given origin
    Plane(AxisPlane plane, Vector3f origin = { 0.0f, 0.0f, 0.0f });
    // Apply the given transformation to this plane
    void transform(const Matrix4f& transform);
    // Returns the 2D coordinate of the given 3D point on this plane
    Vector2f project(const Vector3f& point) const;
};

// Represents a 3D triangle
class Triangle
{
   public:
    std::array<Vector3f, 3> verts;

    Triangle(const std::array<Vector3f, 3>& verts) : verts(verts){};
};

struct RayHit
{
    Vector3f point;
    Vector3f normal;
    float distance;
};

// Represents a 3D ray: origin and a direction
struct Ray
{
    Vector3f origin;
    Vector3f direction;

    Ray(const Vector3f& origin, const Vector3f& direction);

    // Intersection with plane
    std::optional<Vector3f> intersect(const Plane& plane) const;
    // Intersection with triangle
    std::optional<RayHit> intersect(const Triangle& tri) const;
    // Bi-directional intersect: intersection even if ray and plane are not facing each other
    std::optional<Vector3f> biIntersect(const Plane& plane) const;
    // Returns transformed ray
    Ray transformed(const Matrix4f& t) const;
    // Transforms ray in-place
    Ray& transform(const Matrix4f& t);
    // Returns point along ray at given distance
    Vector3f at(float distance) const;
};

// Linear interpolate
template <typename T> T lerp(const T& a, const T& b, float t)
{
    return a + (b - a) * t;
}
// Normalized atan2
float angle2D(const Vector2f& v);
// Returns the signed angle of 2D vectors from a to b
float angleTo(const Vector2f& a, const Vector2f& b);
// Convert radians to degrees
float degrees(float radians);
// Convert degrees to radians
float radians(float degrees);
// smoothstep
float smoothstep(float edge0, float edge1, float x);
/**
 * @brief Convert HSV to RGB
 *
 * @param hsv hue(0-360deg), saturation(0-1), value(0-1)
 * @return RGB
 */
Vector3f hsvToRgb(const Vector3f& hsv);
// Return a new identity transform
Transform3f identityTransform();
// Euler angles to quaternion
Quaternionf euler(const Vector3f& axisAngles);
/**
 * @brief Convert spherical to cartesian coordinates (+Y up) ...
 *        (theta=0, phi=0) -> (x=0, y=0, z=1)
 *
 *        distance from origin is always 1
 *
 * @param phi Horizontal angle
 * @param theta Vertical angle
 * @return Cartesian coordinate
 */
Vector3f spherePoint(float phi, float theta);
Vector3f spherePoint(const Vector2f& point);
// Convert cartesian to spherical coordinates (+Y up)
Vector2f pointSphere(const Vector3f& point);
// Converts a direction to an euler angles rotation
Vector3f dirToRot(const Vector3f& dir);
// Returns a 3x3 skew-symmetric matrix from given vec3
Matrix3f skew(const Vector3f& v);
// Rotates a vector by given euler angles
Vector3f rotate(const Vector3f& v, const Vector3f& axisAngles);
// Gets direction corresponding to given euler angles
Vector3f direction(const Vector3f& axisAngles);
// Angle from a to b
Vector3f towards(const Vector3f& a, const Vector3f& b);
// Perspective projection matrix
Matrix4f perspective(float fov, float aspect, float near, float far);
// Perspective projection matrix
Matrix4f perspective(const Vector4f& view, float near, float far);
// Orthographic projection matrix
Matrix4f orthographic(const Vector2f& size, float zoom, float near, float far);
// Project vector a onto b
template <int _size>
Vector<float, _size> project(const Vector<float, _size>& a, const Vector<float, _size>& b)
{
    return (a.dot(b.normalized())) * b;
}
// Returns normal vector from given triangle
Vector3f normal(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2);
// Clamps the given vector between min and max
Vector3f clamp(const Vector3f& v, const Vector3f& vmin, const Vector3f& vmax);
// Creates a transformation matrix from given translation, rotation, and scale
Matrix4f transform(
    const Vector3f& translation,
    const Vector3f& axisAngles,
    const Vector3f& scale = { 1.0f, 1.0f, 1.0f }
);
Matrix4f transform(
    const Vector3f& translation,
    const Matrix3f& rotMatrix,
    const Vector3f& scale = { 1.0f, 1.0f, 1.0f }
);
// Applies transformation to a point (applies translation, doesn't normalize)
Vector3f transformPoint(const Vector3f& point, const Matrix4f& transform);
// Applies transformation to a direction vector (ignores translation, normalizes)
Vector3f transformDir(const Vector3f& direction, const Matrix4f& transform);
// Creates a new identity transformation
Transform3f transformI();

Vector3f vec3(float v[3]);
Vector3f vec3(const Vector2f& v, float z = 0.0f);
Vector3f vec3(float xyz);
Vector2f vec2(const Vector3f& v);
Vector4f vec4(const Vector3f& v, float w = 1.0f);
Vector4f vec4(const Vector2f& v, float z, float w);

const Vector3f unitVec(const Axis axis);
const Vector3f unitVec(const AxisPlane plane);

template <typename TVector> struct EigenVectorFormatter
{
    const IOFormat ioFormat = IOFormat(4, DontAlignCols, ", ", ", ", "", "", "<", ">", ' ');

    std::string strFormat(const TVector& input)
    {
        std::stringstream ss;
        ss << input.format(ioFormat);
        return ss.str();
    }
};

// Formatter for eigen vector types
template <typename TValue, int _size>
std::ostream& operator<<(std::ostream& os, const Vector<TValue, _size>& v)
{
    return os << EigenVectorFormatter<Vector<TValue, _size>>().strFormat(v);
}

// Take a channel stacked matrix and slice out a vector
template <c_Matrix TMat> auto channelVec(const TMat& mat, const Vector2i& pos)
{
    using TVal = typename TMat::Scalar;
    constexpr int rows = TMat::RowsAtCompileTime;
    constexpr int cols = TMat::ColsAtCompileTime;
    constexpr int channels = rows / cols;
    using TVec = Vector<TVal, channels>;

    return TVec(mat.block<channels, 1>(pos.y() * channels, pos.x()));
}
