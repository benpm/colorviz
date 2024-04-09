#include <camera.hpp>

void CameraControl::orbit()
{
    this->mode = Mode::orbit;
    this->pos = this->target + spherePoint(this->phi, this->theta) * distance;
    this->rot = { this->theta, -this->phi, 0.0f };
}

void CameraControl::dragStart()
{
    switch (this->mode) {
        case Mode::orbit:
            this->orbitPanStart();
            break;
        default:
            this->posStart = this->pos;
            break;
    }
}

void CameraControl::orbitPanStart()
{
    this->panStartTheta = this->theta;
    this->panStartPhi = this->phi;
    this->posStart = this->target;
}

void CameraControl::orbitPan(Vector2f delta)
{
    this->theta = std::clamp(this->panStartTheta + delta.y(), -tau4, tau4);
    this->phi = this->panStartPhi + delta.x();
    this->orbit();
}

void CameraControl::orbitTarget(Vector3f target)
{
    this->target = target;
    this->orbit();
}
const Vector3f& CameraControl::orbitTarget() const
{
    return this->target;
}

void CameraControl::orbitDist(float distance)
{
    this->distance = distance;
    this->orbit();
}
float CameraControl::orbitDist() const
{
    return this->distance;
}

void CameraControl::orbitTheta(float theta)
{
    this->theta = theta;
    this->orbit();
}
float CameraControl::orbitTheta() const
{
    return this->theta;
}

void CameraControl::orbitPhi(float phi)
{
    this->phi = phi;
    this->orbit();
}
float CameraControl::orbitPhi() const
{
    return this->phi;
}

void CameraControl::invertPhi()
{
    this->phi = -this->phi;
    this->panStartPhi = -this->panStartPhi;
    this->orbit();
}

void CameraControl::flyDir(const Vector3f& dir)
{
    const Matrix4f view =
        identityTransform().rotate(euler(this->rot)).translate(-this->pos).matrix();
    this->pos += view.block<3, 3>(0, 0).transpose() * dir;
}

void CameraControl::control(
    const Vector2f& rotateDelta,
    const Vector2f& dragDelta,
    const Vector2f& moveDelta
)
{
    switch (this->mode) {
        case Mode::fly:
            this->flyDir(Vector3f(moveDelta.x(), 0.0f, -moveDelta.y()));
            this->rot += Vector3f(rotateDelta.y(), rotateDelta.x(), 0.0f);
            break;
        case Mode::orbit: {
            const Vector3f dir = vec3(moveDelta / this->zoom);
            const Matrix4f view =
                identityTransform().rotate(euler(this->rot)).translate(-this->target).matrix();
            this->target = this->posStart + view.block<3, 3>(0, 0).transpose() * dir;
            this->orbitPan({ dragDelta.x(), dragDelta.y() });
        } break;
        case Mode::trackball:
            break;
        case Mode::track2D:
            this->pos = this->posStart + vec3((dragDelta * 100000.0f) / this->zoom);
            this->pos.z() = 50.0f;
            break;
        default:
            break;
    }
}

void CameraControl::update(Camera& cam, const Vector2f& viewSize) const
{
    cam.pos = this->pos;
    cam.rot = this->rot;
    cam.zoom = this->zoom;
    cam.target = this->target;
    cam.viewSize = viewSize;
}

void CameraControl::universalZoom(float delta)
{
    this->orbitDist(this->orbitDist() * (1.0f + delta));
    this->zoom *= (1.0f - delta);
}

void Camera::setProjectionType(Projection projection)
{
    this->projection = projection;
}

void CameraControl::setOrthoView(Camera& cam, Camera::OrthoView view)
{
    float dist = (this->pos - this->target).norm();
    switch (view) {
        case Camera::OrthoView::front:
            this->pos = { 0.0f, 0.0f, dist };
            break;
        case Camera::OrthoView::back:
            this->pos = { 0.0f, 0.0f, -dist };
            break;
        case Camera::OrthoView::left:
            this->pos = { -dist, 0.0f, 0.0f };
            break;
        case Camera::OrthoView::right:
            this->pos = { dist, 0.0f, 0.0f };
            break;
        case Camera::OrthoView::top:
            this->pos = { 0.0f, dist, 0.0f };
            break;
        case Camera::OrthoView::bottom:
            this->pos = { 0.0f, -dist, 0.0f };
            break;
        default:
            break;
    }
    cam.setProjectionType(Camera::Projection::orthographic);
    Vector2f sphereAngles = pointSphere(this->pos);
    this->theta = sphereAngles.x();
    this->phi = sphereAngles.y();
    this->orbitPanStart();
    this->update(cam, cam.viewSize);
}

const Matrix4f Camera::getView() const
{
    return identityTransform().rotate(euler(rot)).translate(-pos).matrix();
}

const Matrix4f Camera::getProj() const
{
    float dist = (this->pos - this->target).norm();
    switch (this->projection) {
        case Projection::perspective:
            return perspective(
                this->fov, this->viewSize.x() / this->viewSize.y(), this->near, this->far
            );
        case Projection::orthographic:
            return orthographic(this->viewSize, this->fov * dist, -2 * dist, 2 * dist);
        default:
            return Matrix4f::Identity();
    }
}

Vector3f Camera::toView(const Vector3f& point) const
{
    return (this->getView() * Vector4f(point.x(), point.y(), point.z(), 1.0f)).head<3>();
}

Vector3f Camera::screenToWorld(const Vector2f& screenPoint, float depth) const
{
    const Vector3f clipPos(
        (screenPoint.x() / this->viewSize.x()) * 2.0f - 1.0f,
        1.0f - (screenPoint.y() / this->viewSize.y()) * 2.0f, depth * 2.0f - 1.0f
    );
    const Matrix4f invMat = (this->getProj() * this->getView()).inverse();
    return transformPoint(clipPos, invMat);
}

Vector2f Camera::worldToScreen(const Vector3f& worldPoint) const
{
    const Vector3f clipPos = this->worldToClip(worldPoint);
    return Vector2f(
        (clipPos.x() + 1.0f) / 2.0f * this->viewSize.x(),
        (1.0f - clipPos.y()) / 2.0f * this->viewSize.y()
    );
}

Vector3f Camera::worldToClip(const Vector3f& worldPoint) const
{
    return transformPoint(worldPoint, this->getProj() * this->getView());
}

Vector3f Camera::clipToWorld(const Vector3f& clipPoint) const
{
    return transformPoint(clipPoint, (this->getProj() * this->getView()).inverse());
}

Vector2f Camera::screenToClip(const Vector2f& screenPoint) const
{
    return Vector2f(
        (screenPoint.x() / this->viewSize.x()) * 2.0f - 1.0f,
        1.0f - (screenPoint.y() / this->viewSize.y()) * 2.0f
    );
}

void Camera::from(const Camera& other)
{
    this->pos = other.pos;
    this->rot = other.rot;
    this->fov = other.fov;
    this->zoom = other.zoom;
    this->projection = other.projection;
    this->near = other.near;
    this->far = other.far;
}

Ray Camera::getRay(const Vector2f& screenPoint) const
{
    // https://antongerdelan.net/opengl/raycasting.html

    const Vector4f rayClip = vec4(this->screenToClip(screenPoint), -1.0f, 1.0f);
    const Vector4f rayEye = this->getProj().inverse() * rayClip;
    const Vector3f rayWorld =
        (this->getView().inverse() * vec4(rayEye.head<2>(), -1.0f, 0.0f)).head<3>().normalized();
    return Ray(this->pos, rayWorld);
}