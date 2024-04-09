#include <vecmath.hpp>

float angle2D(const Vector2f& v)
{
    return std::atan2(v.y(), v.x()) + tau2;
}

float angleTo(const Vector2f& a, const Vector2f& b)
{
    const Vector2f d = b - a;
    return std::fmod(pi * 2.5f - std::atan2(d.x(), d.y()), tau);
}

float degrees(float radians)
{
    return radians * 180.0f / tau2;
}

float radians(float degrees)
{
    return degrees * tau2 / 180.0f;
}

float smoothstep(float edge0, float edge1, float x)
{
    const float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

Vector3f hsvToRgb(const Vector3f& hsv)
{
    const float h = std::fmod(hsv.x(), 360.0f);
    const float s = hsv.y();
    const float v = hsv.z();

    const float c = v * s;
    const float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    const float m = v - c;

    Vector3f rgb;
    if (h < 60.0f) {
        rgb = { c, x, 0.0f };
    } else if (h < 120.0f) {
        rgb = { x, c, 0.0f };
    } else if (h < 180.0f) {
        rgb = { 0.0f, c, x };
    } else if (h < 240.0f) {
        rgb = { 0.0f, x, c };
    } else if (h < 300.0f) {
        rgb = { x, 0.0f, c };
    } else {
        rgb = { c, 0.0f, x };
    }

    return rgb + Vector3f(m, m, m);
}

Transform3f identityTransform()
{
    return Transform3f::Identity();
}

Quaternionf euler(const Vector3f& axisAngles)
{
    return AngleAxisf(axisAngles.x(), Vector3f::UnitX()) *
           AngleAxisf(axisAngles.y(), Vector3f::UnitY()) *
           AngleAxisf(axisAngles.z(), Vector3f::UnitZ());
}

Vector3f spherePoint(float phi, float theta)
{
    return {
        std::cos(theta) * std::sin(phi),
        std::sin(theta),
        std::cos(theta) * std::cos(phi),
    };
}

Vector3f spherePoint(const Vector2f& point)
{
    return spherePoint(point.x(), point.y());
}

Vector2f pointSphere(const Vector3f& p)
{
    return { std::atan2(p.y(), std::sqrt(p.x() * p.x() + p.z() * p.z())),
             std::atan2(p.x(), p.z()) + tau4 };
}

Vector3f dirToRot(const Vector3f& dir)
{
    return Quaternionf::FromTwoVectors(-Vector3f::UnitZ(), dir)
        .toRotationMatrix()
        .eulerAngles(0, 1, 2);
}

Matrix3f skew(const Vector3f& v)
{
    Matrix3f m;
    m << 0.0f, -v.z(), v.y(), v.z(), 0.0f, -v.x(), -v.y(), v.x(), 0.0f;
    return m;
}

Vector3f rotate(const Vector3f& v, const Vector3f& axisAngles)
{
    return euler(axisAngles) * v;
}

Vector3f direction(const Vector3f& axisAngles)
{
    // Rotation about x axis is up/down, rotation about y axis is left/right around sphere
    return spherePoint({ axisAngles.y(), axisAngles.x() });
}

Vector3f towards(const Vector3f& a, const Vector3f& b)
{
    return dirToRot(b - a);
}

Matrix4f perspective(float fov, float aspect, float near, float far)
{
    Matrix4f m = Matrix4f::Zero();
    m(0, 0) = 1.0f / (aspect * tanf(fov / 2.0f));
    m(1, 1) = 1.0f / tanf(fov / 2.0f);
    m(2, 2) = -(far + near) / (far - near);
    m(2, 3) = -2.0f * far * near / (far - near);
    m(3, 2) = -1.0f;
    return m;
}

Matrix4f perspective(const Vector4f& view, float near, float far)
{
    Matrix4f m = Matrix4f::Zero();
    m(0, 0) = 2.0f * near / (view.z() - view.x());
    m(1, 1) = 2.0f * near / (view.w() - view.y());
    m(2, 2) = -(far + near) / (far - near);
    m(2, 3) = -2.0f * far * near / (far - near);
    m(3, 2) = -1.0f;
    return m;
}

Matrix4f orthographic(const Vector2f& size, float zoom, float near, float far)
{
    float aspect = size.x() / size.y();
    float left = -aspect * zoom * 0.5f;
    float right = aspect * zoom * 0.5f;
    float bottom = -zoom * 0.5f;
    float top = zoom * 0.5f;
    Matrix4f m = Matrix4f::Zero();
    m(0, 0) = 2.0f / (right - left);
    m(1, 1) = 2.0f / (top - bottom);
    m(2, 2) = -2.0f / (far - near);
    m(0, 3) = -(right + left) / (right - left);
    m(1, 3) = -(top + bottom) / (top - bottom);
    m(2, 3) = -(far + near) / (far - near);
    m(3, 3) = 1.0f;
    return m;
}

Vector3f normal(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2)
{
    return (v1 - v0).cross(v2 - v0).normalized();
}

Vector3f clamp(const Vector3f& v, const Vector3f& vmin, const Vector3f& max)
{
    return v.cwiseMax(vmin).cwiseMin(max);
}

Matrix4f transform(const Vector3f& translation, const Vector3f& axisAngles, const Vector3f& scale)
{
    return identityTransform()
        .translate(translation)
        .rotate(euler(axisAngles))
        .scale(scale)
        .matrix();
}

Matrix4f transform(const Vector3f& translation, const Matrix3f& rotMatrix, const Vector3f& scale)
{
    return identityTransform().translate(translation).rotate(rotMatrix).scale(scale).matrix();
}

Vector3f transformPoint(const Vector3f& point, const Matrix4f& transform)
{
    const Vector4f v = transform * vec4(point, 1.0f);
    return v.head<3>() / v.w();
}

Vector3f transformDir(const Vector3f& point, const Matrix4f& transform)
{
    return (transform * vec4(point, 0.0f)).head<3>().normalized();
}

Transform3f transformI()
{
    return Transform3f::Identity();
}

// --- Plane --- //

Plane::Plane(const Vector3f& origin, const Vector3f& normal) : origin(origin), normal(normal) {}

Plane::Plane(AxisPlane plane, Vector3f origin) : origin(origin), normal(unitVec(plane)) {}

void Plane::transform(const Matrix4f& transform)
{
    this->origin = transformPoint(this->origin, transform);
    this->normal = transformDir(this->normal, transform);
}

Vector2f Plane::project(const Vector3f& point) const
{
    // Determine the 2D coordinate of the 3D point that lies on this plane
    /// TODO: There is a more robust way to do this using spherical coordinates
    const Vector3f v = point - this->origin;
    const Vector3f n = this->normal;
    const Vector3f u = n.cross(Vector3f::UnitY()).normalized();
    const Vector3f w = n.cross(u).normalized();
    return { v.dot(u), v.dot(w) };
}

// --- Ray --- //

Ray::Ray(const Vector3f& origin, const Vector3f& direction)
    : origin(origin), direction(direction.normalized())
{
}

std::optional<Vector3f> Ray::intersect(const Plane& plane) const
{
    const float d = plane.normal.dot(this->direction);
    if (d < 1e-6f) {
        return this->origin;
    }
    const float t = (plane.origin - this->origin).dot(plane.normal) / d;
    if (t < 0.0f) {
        return std::nullopt;
    }
    return this->origin + this->direction * t;
}

std::optional<RayHit> Ray::intersect(const Triangle& tri) const
{
    const auto& [v0, v1, v2] = tri.verts;
    const Vector3f v0v1 = v1 - v0;
    const Vector3f v0v2 = v2 - v0;
    const Vector3f pvec = this->direction.cross(v0v2);
    const float det = v0v1.dot(pvec);
    if (det < 1e-6) {
        return std::nullopt;
    }

    const float invDet = 1 / det;

    const Vector3f tvec = this->origin - v0;
    const float u = tvec.dot(pvec) * invDet;
    if (u < 0 || u > 1) {
        return std::nullopt;
    }

    const Vector3f qvec = tvec.cross(v0v1);
    const float v = this->direction.dot(qvec) * invDet;
    if (v < 0 || u + v > 1) {
        return std::nullopt;
    }

    const float t = v0v2.dot(qvec) * invDet;
    return RayHit{ .point = this->origin + this->direction * t,
                   .normal = v0v1.cross(v0v2).normalized(),
                   .distance = t };
}

std::optional<Vector3f> Ray::biIntersect(const Plane& plane) const
{
    const float d = plane.normal.dot(this->direction);
    if (d == 0.0f) {
        return std::nullopt;
    }
    const float t = (plane.origin - this->origin).dot(plane.normal) / d;
    if (t < 0.0f) {
        return std::nullopt;
    }
    return this->origin + this->direction * t;
}

Ray Ray::transformed(const Matrix4f& t) const
{
    return Ray(*this).transform(t);
}

Ray& Ray::transform(const Matrix4f& t)
{
    this->origin = transformPoint(this->origin, t);
    this->direction = transformDir(this->direction, t);
    return *this;
}

Vector3f Ray::at(float distance) const
{
    return this->origin + this->direction * distance;
}

Vector3f vec3(float v[3])
{
    return { v[0], v[1], v[2] };
}

Vector3f vec3(const Vector2f& v, float z)
{
    return { v.x(), v.y(), z };
}

Vector3f vec3(float xyz)
{
    return { xyz, xyz, xyz };
}

Vector2f vec2(const Vector3f& v)
{
    return { v.x(), v.y() };
}

Vector4f vec4(const Vector3f& v, float w)
{
    return { v.x(), v.y(), v.z(), w };
}

Vector4f vec4(const Vector2f& v, float z, float w)
{
    return { v.x(), v.y(), z, w };
}

const Vector3f unitVec(const Axis axis)
{
    switch (axis) {
        case Axis::X:
            return { 1.0f, 0.0f, 0.0f };
        case Axis::Y:
            return { 0.0f, 1.0f, 0.0f };
        case Axis::Z:
            return { 0.0f, 0.0f, 1.0f };
    }
    return {};
}

const Vector3f unitVec(const AxisPlane plane)
{
    switch (plane) {
        case AxisPlane::XY:
            return { 0.0f, 0.0f, 1.0f };
        case AxisPlane::XZ:
            return { 0.0f, 1.0f, 0.0f };
        case AxisPlane::YZ:
            return { 1.0f, 0.0f, 0.0f };
    }
    return {};
}
