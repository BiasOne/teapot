#include "my_camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

// std
#include <cassert>
#include <limits>
#include <iostream>

MyCamera::MyCamera()
{
    // TODO: set default camera position
    setViewTarget(
        glm::vec3(0.0f, 0.0f, 10.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

void MyCamera::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far) 
{
    // Based on the equation from
    // https://www.songho.ca/opengl/gl_projectionmatrix.html
    //
    // Follow Vulkan's convention that X is right, Y is down and Z is into the screen

    m_m4ProjectionMatrix = glm::mat4{ 1.0f };

    //
    // glm matrix layout
    // 
    // [0,0] [1,0] [2,0] [3,0]
    // [0,1] [1,1] [2,1] [3,1]
    // [0,2] [1,2] [2,2] [3,2]
    // [0,3] [1,3] [2,3] [3,3]
    // 
    m_m4ProjectionMatrix[0][0] = 2.f / (right - left);
    m_m4ProjectionMatrix[1][1] = 2.f / (top - bottom);
    m_m4ProjectionMatrix[2][2] = -2.0f / (far - near);
    m_m4ProjectionMatrix[3][0] = -(right + left) / (right - left);
    m_m4ProjectionMatrix[3][1] = -(top + bottom) / (top - bottom);
    m_m4ProjectionMatrix[3][2] = -(far + near) / (far - near);
}

void MyCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
{
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

    // Use OpenGL matrix but follow Vulkan's convention that Y is down
    const float tanHalfFovy = tan(fovy / 2.f);

    // Based on the equation from
    // https://www.songho.ca/opengl/gl_projectionmatrix.html
    //
    // Follow Vulkan's convention that X is right, Y is down and Z is into the screen


    const float right = near * tanHalfFovy * aspect;  // Right is positive
    const float left = -1.0f * right;                 // Left is minus X (because near is a negative value)

    const float bottom = near * tanHalfFovy;          // bottom is positive (because Y is down)
    const float top = -1.0f * bottom;                 // top is negative

    //
    // Use this matrix such that it will become X - right, Y - up, Z - out of the screen
    //
    //
    // glm matrix layout
    // 
    // [0,0] [1,0] [2,0] [3,0]
    // [0,1] [1,1] [2,1] [3,1]
    // [0,2] [1,2] [2,2] [3,2]
    // [0,3] [1,3] [2,3] [3,3]
    // 
    m_m4ProjectionMatrix = glm::mat4{ 0.0f };
    m_m4ProjectionMatrix[0][0] = 2.0f * near / (right - left);
    m_m4ProjectionMatrix[2][0] = (right + left) / (right - left);
    m_m4ProjectionMatrix[1][1] = 2.0f * near / (top - bottom);
    m_m4ProjectionMatrix[2][1] = (top + bottom) / (top - bottom);
    m_m4ProjectionMatrix[2][2] = -1.0f * (far + near) / (far - near);
    m_m4ProjectionMatrix[3][2] = -2.0f * far * near / (far - near);
    m_m4ProjectionMatrix[2][3] = -1.0f;
}

void MyCamera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    // Note: 'position' in this function can be considered as the world coordinate system stays the same and 
    // the view coordinate system moves, but from the viewer's perspective, the world coordinate system is moved
    // to that 'position'.

    //
    // glm matrix layout
    // 
    // [0,0] [1,0] [2,0] [3,0]
    // [0,1] [1,1] [2,1] [3,1]
    // [0,2] [1,2] [2,2] [3,2]
    // [0,3] [1,3] [2,3] [3,3]
    //
    const glm::vec3 w{ glm::normalize(direction)}; // Z is out of the screen
    const glm::vec3 u{ glm::normalize(glm::cross(up, w)) };
    const glm::vec3 v{ glm::cross(w, u) };

    // View to world
    // [u.x] [v.x] [w.x] [p.x]
    // [u.y] [v.y] [w.y] [p.y]
    // [u.z] [v.z] [w.z] [p.z]
    // [0.0] [0.0] [0.0] [1.0]
    m_m4ViewMatrix = glm::mat4{ 1.f };
    m_m4ViewMatrix[0][0] = u.x;
    m_m4ViewMatrix[0][1] = u.y;
    m_m4ViewMatrix[0][2] = u.z;
    m_m4ViewMatrix[1][0] = v.x;
    m_m4ViewMatrix[1][1] = v.y;
    m_m4ViewMatrix[1][2] = v.z;
    m_m4ViewMatrix[2][0] = w.x;
    m_m4ViewMatrix[2][1] = w.y;
    m_m4ViewMatrix[2][2] = w.z;
    m_m4ViewMatrix[3][0] = position.x;
    m_m4ViewMatrix[3][1] = position.y;
    m_m4ViewMatrix[3][2] = position.z;

    // World to view
    m_m4ViewMatrix = glm::inverse(m_m4ViewMatrix);
}

void MyCamera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    // Z direction is out of the screen so we need to flip the view direction
    setViewDirection(position, position - target, up);
}

void MyCamera::setViewYXZ(glm::vec3 position, glm::vec3 rotation)
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
    const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
    const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };

    m_m4ViewMatrix = glm::mat4{ 1.f };
    m_m4ViewMatrix[0][0] = u.x;
    m_m4ViewMatrix[1][0] = u.y;
    m_m4ViewMatrix[2][0] = u.z;
    m_m4ViewMatrix[0][1] = v.x;
    m_m4ViewMatrix[1][1] = v.y;
    m_m4ViewMatrix[2][1] = v.z;
    m_m4ViewMatrix[0][2] = w.x;
    m_m4ViewMatrix[1][2] = w.y;
    m_m4ViewMatrix[2][2] = w.z;
    m_m4ViewMatrix[3][0] = -glm::dot(u, position);
    m_m4ViewMatrix[3][1] = -glm::dot(v, position);
    m_m4ViewMatrix[3][2] = -glm::dot(w, position);
}

void MyCamera::setMode(MyCameraMode mode)
{
    m_eMode = mode;

    std::cout << "MyCamera::setMode = " << (int)mode << std::endl;

    // Note: fit all doesn't require mouse operation
    if (m_eMode == MYCAMERA_FITALL)
    {
        std::cout << "Fit All" << std::endl;
        _fitAll();
    }
}

void MyCamera::setSceneMinMax(glm::vec3 min, glm::vec3 max)
{
    m_vSceneMin = min;
    m_vSceneMax = max;
}

void MyCamera::setButton(bool buttonPress, float x, float y)
{
    // set the temp tranform to identity matrix
    m_m4TempTransform = glm::mat4{ 1.f };

    if (buttonPress)
    {
        std::cout << "Mouse button pressed" << std::endl;
        m_vCurrPos.x = x;
        m_vCurrPos.y = y;
        m_vPrevPos = m_vCurrPos;
        m_bMoving = true;
    }
    else
    {
        std::cout << "Mouse button released" << std::endl;
        m_bMoving = false;
    }
}

void MyCamera::setMotion(bool buttonPress, float x, float y)
{
    if (!m_bMoving)
        return;

    // TODO handle the camera navitaion based on the navigation mode
    m_vCurrPos.x = x;
    m_vCurrPos.y = y;

    // ignore any small movement
    glm::vec2 delta = m_vCurrPos - m_vPrevPos;
    float mag = glm::length(delta);

    if (mag < 1.0e-6f) return;

    if (m_eMode == MYCAMERA_ROTATE) // Press key R
    {
        std::cout << "  Rotating..." << std::endl;
        _rotate(delta.x, delta.y);
    }
    else if (m_eMode == MYCAMERA_PAN) // Press key P
    {
        std::cout << "  Panning..." << std::endl;
        _pan(delta.x, delta.y);
    }
    else if (m_eMode == MYCAMERA_ZOOM) // Press key Z
    {
        std::cout << "  Zooming..." << std::endl;
        _zoom(delta.x, delta.y);
    }
    else if (m_eMode == MYCAMERA_TWIST) // Press key T
    {
        std::cout << "  Twisting..." << std::endl;
        _twist(delta.x, delta.y);
    }

    // TODO - Combine m_m4TempTransform with m_m4ViewMatrix to become the new m_m4ViewMatrix

    // Step 1: decompose m_m4TempTransform to translate only and rotation only matrices
    // Step 2: center of the scene graph and the inverse of the center
    // Step 3: previous transform matrix, which is m_m4ViewMatrix

    // Combine these 5 matrices together
    // m_m4ViewMatrix = m5 * m4 * m3 * m2 * m1;

    glm::mat4 translateOnlyMatrix = glm::mat4(1.0f);
    translateOnlyMatrix[3][0] = m_m4TempTransform[3][0];
    translateOnlyMatrix[3][1] = m_m4TempTransform[3][1];
    translateOnlyMatrix[3][2] = m_m4TempTransform[3][2];

    glm::mat4 rotateOnlyMatrix = m_m4TempTransform;
    rotateOnlyMatrix[3][0] = 0.0f;
    rotateOnlyMatrix[3][1] = 0.0f;
    rotateOnlyMatrix[3][2] = 0.0f;

    glm::vec3 lookAt = (m_vSceneMin + m_vSceneMax) / 2.0f;
    glm::mat4 center = glm::mat4(1.0f);
    center[3][0] = lookAt.x;
    center[3][1] = lookAt.y;
    center[3][2] = lookAt.z;

    glm::mat4 centerInverse = glm::mat4(1.0f);
    centerInverse[3][0] = -lookAt.x;
    centerInverse[3][1] = -lookAt.y;
    centerInverse[3][2] = -lookAt.z;

    m_m4ViewMatrix = translateOnlyMatrix * m_m4ViewMatrix * center * rotateOnlyMatrix * centerInverse;

	// When done, set the current mouse position as the previous one
    m_vPrevPos = m_vCurrPos;
}

void MyCamera::_pan(float dx, float dy)
{
    // TODO: Handle pan operation
    m_m4TempTransform = glm::mat4(1.0f);
    float distance = m_m4ViewMatrix[3][2];
    std::cout << "d = " << distance << std::endl;
    float fov = glm::radians(50.0f);
    float theta = fov / 2.0f;
    float b = 2.0f * distance * tan(theta);

    const float w = 800.0f;
    const float h = 600.0f;

    float dw = dx * b / w;
    float dh = dy * b / h;

    m_m4TempTransform[3][0] = -dw * 200.0f; // Scaling factor
    m_m4TempTransform[3][1] = -dh * 200.0f; // Scaling factor
}

void MyCamera::_zoom(float dx, float dy)
{
    // TODO: Handle zoom operation
    m_m4TempTransform = glm::mat4(1.0f);
    
    const float h = 600.0f;

    float d = m_m4ViewMatrix[3][2];

    float newD = d * 1.0f / (1.0f + dy / h * 2.0f);

    // we need to calculate the delta d rather new d here
    float deltaD = d * (newD - d);
    
    m_m4TempTransform[3][2] = deltaD;
}   

void MyCamera::_rotate(float dx, float dy)
{
    glm::vec3 sx, sy, sz, axis, tb_axis;
    float mag = sqrtf(dx * dx + dy * dy);
    float tb_angle = mag * 200.0f; // Scaling factor

    axis.x = dy;
    axis.y = dx;
    axis.z = 0.0f;

    _getScreenXYZ(sx, sy, sz);

    tb_axis = sx * axis.x + sy * axis.y + sz * axis.z;

    _atRotate(tb_axis.x, tb_axis.y, tb_axis.z, tb_angle);
}

void MyCamera::_atRotate(float x, float y, float z, float angle)
{
    int i = 0;
    float rad = angle * glm::pi<float>() / 180.0f; // Become radius
    float cosAng = (float)(cos(rad));
    float sinAng = (float)(sin(rad));

    glm::vec3 axis(x, y, z);
    axis = glm::normalize(axis);

    // Reset delta matrix
    m_m4TempTransform = glm::mat4(0.0f);

    float x2 = axis.x * axis.x;
    float xy = axis.x * axis.y;
    float xz = axis.x * axis.z;

    float y2 = axis.y * axis.y;
    float yz = axis.y * axis.z;
    float z2 = axis.z * axis.z;

    m_m4TempTransform[0][0] = x2 + (cosAng * (1 - x2));
    m_m4TempTransform[0][1] = xy - (cosAng * xy) + (sinAng * axis[2]);
    m_m4TempTransform[0][2] = xz - (cosAng * xz) - (sinAng * axis[1]);

    m_m4TempTransform[1][0] = xy - (cosAng * xy) - (sinAng * axis[2]);
    m_m4TempTransform[1][1] = y2 + (cosAng * (1 - y2));
    m_m4TempTransform[1][2] = yz - (cosAng * yz) + (sinAng * axis[0]);

    m_m4TempTransform[2][0] = xz - (cosAng * xz) + (sinAng * axis[1]);
    m_m4TempTransform[2][1] = yz - (cosAng * yz) - (sinAng * axis[0]);
    m_m4TempTransform[2][2] = z2 + (cosAng * (1 - z2));

    m_m4TempTransform[3][3] = 1.0f;
}

void MyCamera::_twist(float dx, float dy)
{
    // TODO: Handle twist operation
    m_m4TempTransform = glm::mat4(1.0f);

    const float w = 800.0f;
    const float h = 600.0f;

    // Step 1: normalize both positions by window width and height
    float prevX = m_vPrevPos.x / w;
    float prevY = m_vPrevPos.y / h;
    float currX = m_vCurrPos.x / w;
    float currY = m_vCurrPos.y / h;

    // Step 2: calculate delta rotation
    float prevTheta = atan2f(prevX - 0.5f, prevY - 0.5f);
    float theta     = atan2f(currX - 0.5f, currY - 0.5f);
    float delta_theta = 180.0f / glm::pi<float>() * (theta - prevTheta) * 200.0f; // scaling factor
  
    glm::vec3 sx, sy, sz;
    _getScreenXYZ(sx, sy, sz);

    _atRotate(sz.x, sz.y, sz.z, delta_theta);
}

void MyCamera::_fitAll()
{   
    m_m4TempTransform = glm::mat4(1.0f);

    glm::vec3 lookAt = (m_vSceneMin + m_vSceneMax) / 2.0f;
    float radius = glm::length(m_vSceneMax - m_vSceneMin) / 2.0f;    
    float fov = glm::radians(50.0f);
    float distance = radius / tan(fov / 2.0f) * 2.0; // Scale Factor

    glm::vec3 viewVector = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 lookFrom = lookAt - viewVector * distance;

    // Step 1: calculate the look from point based on the min and max of your scene graph
    // the new look from
    m_m4TempTransform[3][0] = -lookFrom.x;
    m_m4TempTransform[3][1] = -lookFrom.y;
    m_m4TempTransform[3][2] = -lookFrom.z;

    // Step 2: maintain the existing rotation from m_m4ViewMatrix
    glm::mat4 rotateOnlyMatrix = m_m4ViewMatrix;
    rotateOnlyMatrix[3][0] = 0.0f;
    rotateOnlyMatrix[3][1] = 0.0f;
    rotateOnlyMatrix[3][2] = 0.0f;

    // Step 3: find the center of your scene graph
    glm::mat4 center = glm::mat4(1.0f);
    center[3][0] = lookAt.x;
    center[3][1] = lookAt.y;
    center[3][2] = lookAt.z;

    glm::mat4 centerInverse = glm::mat4(1.0f);
    centerInverse[3][0] = -lookAt.x;
    centerInverse[3][1] = -lookAt.y;
    centerInverse[3][2] = -lookAt.z;

    // Combine all matrices to become the new m_m4ViewMatrix
    // this should be correct
    m_m4ViewMatrix = m_m4TempTransform * center * rotateOnlyMatrix * centerInverse;
    
}

void MyCamera::_getScreenXYZ(glm::vec3& sx, glm::vec3& sy, glm::vec3& sz)
{
    sx.x = m_m4ViewMatrix[0][0];
    sx.y = m_m4ViewMatrix[1][0];
    sx.z = m_m4ViewMatrix[2][0];
    sy.x = m_m4ViewMatrix[0][1];
    sy.y = m_m4ViewMatrix[1][1];
    sy.z = m_m4ViewMatrix[2][1];
    sz.x = m_m4ViewMatrix[0][2];
    sz.y = m_m4ViewMatrix[1][2];
    sz.z = m_m4ViewMatrix[2][2];
}

