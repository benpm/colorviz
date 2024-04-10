// Camera class with various methods of movement
#pragma once

#include <util.hpp>
#include <vecmath.hpp>
#ifdef PLATFORM_WINDOWS
    #undef near
    #undef far
#endif

struct Camera
{
   public:
    Vector2f viewSize = Vector2f::Zero();
    Vector3f pos = Vector3f::Zero();
    Vector3f rot = Vector3f::Zero();
    Vector3f target = Vector3f::Zero();
    float near = 0.01f;
    float far = 1000.0f;
    float fov = 1.1f;
    float zoom = 1.0f;
    enum class Projection
    {
        perspective,
        orthographic
    } projection = Projection::perspective;

    enum class OrthoView
    {
        top,
        bottom,
        left,
        right,
        front,
        back
    };

    // Returns view transformation matrix
    const Matrix4f getView() const;
    // Returns projection transformation matrix
    const Matrix4f getProj() const;
    // Transforms given point to view space
    Vector3f toView(const Vector3f& point) const;
    // Transforms given point from screen space to world space
    Vector3f screenToWorld(const Vector2f& screenPoint, float depth) const;
    // Transforms given point from world space to screen space
    Vector2f worldToScreen(const Vector3f& worldPoint) const;
    // Transforms given point from world space to clip space
    Vector3f worldToClip(const Vector3f& worldPoint) const;
    // Transforms given point from clip space to world space
    Vector3f clipToWorld(const Vector3f& clipPoint) const;
    // Transforms given 2D point from screen space to clip space (z = 0)
    Vector2f screenToClip(const Vector2f& screenPoint) const;
    // Copy import settings from another camera
    void from(const Camera& other);
    // Produces a ray in world space from the camera through the given screen point
    Ray getRay(const Vector2f& screenPoint) const;
    // switch to orthographic projection
    void setProjectionType(Projection projection);

    friend class CameraControl;
};

class CameraControl
{
   private:
    Vector3f target = Vector3f::Zero();
    float distance = 1.0f;
    float theta = 0.0f;
    float phi = 0.0f;
    float panStartTheta = 0.0f;
    float panStartPhi = 0.0f;
    Vector3f posStart = Vector3f::Zero();

    // Computes camera position and rotation as an orbit camera (uses theta, phi, distance, target)
    void orbit();

   public:
    Vector3f pos = Vector3f::Zero();
    Vector3f rot = Vector3f::Zero();
    float zoom = 1.0f;
    enum class Mode
    {
        fly,
        orbit,
        trackball,
        track2D
    } mode = Mode::fly;

    // Orbit camera: start orbit panning (should be called on click)
    void orbitPanStart();
    // Orbit camera: pan (should be called on drag)
    void orbitPan(Vector2f delta);
    // Orbit camera: set target point
    void orbitTarget(Vector3f target);
    // Orbit camera: get target point
    const Vector3f& orbitTarget() const;
    // Orbit camera: set camera distance to target
    void orbitDist(float distance);
    // Orbit camera: get camera distance to target
    float orbitDist() const;
    // Orbit camera: set camera theta (vertical angle)
    void orbitTheta(float theta);
    // Orbit camera: get camera theta (vertical angle)
    float orbitTheta() const;
    // Orbit camera: set camera phi (horizontal angle)
    void orbitPhi(float phi);
    // Orbit camera: get camera phi (horizontal angle)
    float orbitPhi() const;
    // Invert Phi
    void invertPhi();
    // Fly camera: move camera in specified direction relative to camera
    void flyDir(const Vector3f& dir);
    // Drag start
    void dragStart();
    /**
     * @brief Universal control for camera, uses current mode
     *
     * @param rotateDelta delta for rotation (mouse move delta)
     * @param dragDelta delta for dragging (click and drag delta)
     * @param moveDelta keyboard input, moves camera position
     */
    void control(const Vector2f& rotateDelta, const Vector2f& dragDelta, const Vector2f& moveDelta);
    // Updates the given camera from current state
    void update(Camera& cam, const Vector2f& viewSize) const;
    // Modifies "zoom", which does something slightly different depending on the projection
    void universalZoom(float delta);
    // switch to orthographic view
    void setOrthoView(Camera& cam, Camera::OrthoView view);
};