#include "my_application.h"

// Render factory
#include "my_simple_render_factory.h"
#include "my_keyboard_controller.h"

// use radian rather degree for angle
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Std
#include <stdexcept>
#include <array>
#include <chrono>
#include <iostream>

MyApplication::MyApplication() :
    m_bPerspectiveProjection(true)
{
    _loadGameObjects();
}

void MyApplication::run() 
{
    static bool bPerspectiveCamera = true;

    m_myWindow.bindMyApplication(this);
    MySimpleRenderFactory simpleRenderFactory{ m_myDevice, m_myRenderer.swapChainRenderPass() };

    // Empty object to store camera transformation matrix
    auto viewerObject = MyGameObject::createGameObject();
    //MyKeyboardController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!m_myWindow.shouldClose()) 
    {
        // Note: depending on the platform (Windows, Linux or Mac), this function
        // will cause the event proecssing to block during a Window move, resize or
        // menu operation. Users can use the "window refresh callback" to redraw the
        // contents of the window when necessary during such operation.
        m_myWindow.pollEvents();

        // Need to get the call after glfwPollEvants because the call above may take time
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        // Don't use keyboard to move camera for this assginment
        //cameraController.moveInPlaneXZ(m_myWindow.glfwWindow(), frameTime, viewerObject);
        //m_myCamera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float apsectRatio = m_myRenderer.aspectRatio();

        // Put it here because the viewport may change
		// TODO: Adjust perspective and orthographic cameras so the entire model can be visible
        if (m_bPerspectiveProjection)
            // near and far will automatically apply negative values
            m_myCamera.setPerspectiveProjection(glm::radians(50.f), apsectRatio, 0.1f, 500.0f);
        else
            // because Y is down by default for Vulkan, when we set top to be minus value, we can flip the coordinate
            // such that Y is up. Because we move the part 2.5 units, the near and far value needs to cover the model
            // Also, near and far will automatically apply negative values
            m_myCamera.setOrthographicProjection(-apsectRatio * 3.0f, apsectRatio * 3.0f, -3.0f, 3.0f, -50.0f, 50.0f);

        // Please note that commandBuffer could be null pointer
        // if the swapChain needs to be recreated
        if (auto commandBuffer = m_myRenderer.beginFrame())
		{
            // In case we have multiple render passes for the current frame
            // begin offsreen shadow pass
            // render shadow casting objects
            // end offscreen shadow pass

            m_myRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderFactory.renderGameObjects(commandBuffer, m_vMyGameObjects, m_myCamera);
            m_myRenderer.endSwapChainRenderPass(commandBuffer);

            m_myRenderer.endFrame();
        }
    }

    // GPU will block until all CPU is complete
    vkDeviceWaitIdle(m_myDevice.device());
}

void MyApplication::switchProjectionMatrix()
{
    // Switch between perspective and orthographic projection matrix
    m_bPerspectiveProjection = !m_bPerspectiveProjection;
}

void MyApplication::_loadGameObjects()
{
    glm::vec3 min, max, smin, smax;
    m_objPosMin = min;
    m_objPosMax = max;

    // std::shared_ptr<MyModel> mymodel = MyModel::createModelFromFile(m_myDevice, "models/teapot.obj", min, max);    
    // auto teapot = MyGameObject::createGameObject();
    // teapot.model = mymodel;
    // teapot.transform.translation = { 0.0f, 0.0f, 0.0f};
    // teapot.transform.scale = { 1.0f, 1.0f, 1.0f };
    // m_vMyGameObjects.push_back(std::move(teapot));
    // m_myCamera.setSceneMinMax(min, max);

    std::shared_ptr<MyModel> mymodel = MyModel::createModelFromFile(m_myDevice, "models/mario.obj", min, max);    
    auto terrible_mario = MyGameObject::createGameObject();
    terrible_mario.model = mymodel;
    terrible_mario.transform.translation = { 0.0f, 0.0f, 0.0f};
    terrible_mario.transform.scale = { 1.0f, 1.0f, 1.0f };
    m_vMyGameObjects.push_back(std::move(terrible_mario));

    m_objPosMin = min;
    m_objPosMax = max;
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
    glm::vec3 worldMin = glm::vec3(modelMatrix * glm::vec4(min, 1.0f));
    glm::vec3 worldMax = glm::vec3(modelMatrix * glm::vec4(max, 1.0f));
    m_myCamera.setSceneMinMax(worldMin, worldMax);

    std::shared_ptr<MyModel> sceneModel = MyModel::createModelFromFile(m_myDevice, "models/scene.obj", smin, smax);
    auto scene = MyGameObject::createGameObject();
    scene.model = sceneModel;
    scene.transform.translation = { 2.0f, 4.0f, -2.0f };
    scene.transform.scale = { 1.0f, 1.0f, 1.0f };
    m_vMyGameObjects.push_back(std::move(scene));
    
    m_myCamera.setMode(MyCamera::MYCAMERA_FITALL);
}

void MyApplication::handleMovement(MyAppKeyMap key)
{
    const float speed = 0.01f;
    // code that moves the obj and faces it in the direction its moving
    if (key == KEY_LEFT) 
    {
        std::cout << "Move left"  << std::endl;
        m_vMyGameObjects[0].transform.rotation = {0.0, -1.57f, 0.0};
        m_vMyGameObjects[0].transform.translation.x -= speed;
    } 
    else if (key == KEY_RIGHT) {
        std::cout << "Move right" << std::endl;
        m_vMyGameObjects[0].transform.rotation = {0.0f, 1.57f, 0.0f};
        m_vMyGameObjects[0].transform.translation.x  += speed;
    }
    else if (key == KEY_FORWARD) {
        std::cout << "Move up"    << std::endl;
        m_vMyGameObjects[0].transform.rotation = {0.0, 3.14f, 0.0f};
        m_vMyGameObjects[0].transform.translation.z  -= speed;
    }
    else if (key == KEY_BACKWARD) {
        std::cout << "Move down"  << std::endl;
        m_vMyGameObjects[0].transform.rotation = {0.0f, 0.0f, 0.0f};
        m_vMyGameObjects[0].transform.translation.z += speed;
    }
    // keep track of current position to update fitall center
    glm::vec3 translation = m_vMyGameObjects[0].transform.translation;
    glm::vec3 scale = m_vMyGameObjects[0].transform.scale;
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translation) * glm::scale(glm::mat4(1.0f), scale);
    glm::vec3 worldMin = glm::vec3(modelMatrix * glm::vec4(m_objPosMin, 1.0f));
    glm::vec3 worldMax = glm::vec3(modelMatrix * glm::vec4(m_objPosMax, 1.0f));
    m_myCamera.setSceneMinMax(worldMin, worldMax);
}

void MyApplication::mouseButtonEvent(bool bMouseDown, float posx, float posy)
{
    m_bMouseButtonPress = bMouseDown;
    m_myCamera.setButton(m_bMouseButtonPress, posx, posy);
}

void MyApplication::mouseMotionEvent(float posx, float posy)
{
    m_myCamera.setMotion(m_bMouseButtonPress, posx, posy);
}

void MyApplication::setCameraNavigationMode(MyCamera::MyCameraMode mode)
{
    m_myCamera.setMode(mode);   
}

