/** \file App.cpp */
#include "App.h"
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;
using namespace glm;


namespace basicgraphics {
    const float App::GRAVITY = -9.8;
    
    App::App(int argc, char** argv, std::string windowName, int windowWidth, int windowHeight) : BaseApp(argc, argv, windowName, windowWidth, windowHeight) {
        lastTime = glfwGetTime();
        
        tableWidth = 152.5;
        tableHeight = 76.0;
        tableLength = 274.0;
        netHeight = 10.0;
        
        paddleRadius = 8.0;
        paddleWidth = 1.0;
        
        ballRadius = 2;
        
        lastZHit = -1;
        lastPlayerHit = 0;
        
        vec4 shadowColor = vec4(0,0,0,0.2);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0);
        
        // Initialize the cylinders that make up the model. We're using unique_ptrs here so they automatically deallocate.
        paddle.reset(new Cylinder(vec3(0, 0, -0.5 * paddleWidth), vec3(0, 0, 0.5 * paddleWidth), paddleRadius, vec4(0.5, 0, 0, 1.0)));
        handle.reset(new Cylinder(vec3(0, -7.5, 0), vec3(0, -16, 0), 1.5, vec4(0.3, 0.4, 0, 1.0)));
        mirrorPaddle.reset(new Cylinder(vec3(0, 0, -0.5 * paddleWidth), vec3(0, 0, 0.5 * paddleWidth), paddleRadius, vec4(0.5, 0, 0, 1.0)));
        mirrorHandle.reset(new Cylinder(vec3(0, -7.5, 0), vec3(0, -16, 0), 1.5, vec4(0.3, 0.4, 0, 1.0)));
        
        paddleShadow.reset(new Box(vec3(-paddleRadius, 0, -paddleWidth), vec3(paddleRadius, 0.1, paddleWidth), shadowColor));
        mirrorPaddleShadow.reset(new Box(vec3(-paddleRadius, 0, -paddleWidth), vec3(paddleRadius, 0.1, paddleWidth), shadowColor));
        
        
        ball.reset(new Sphere(ballPos, ballRadius, vec4(1,1,1,1)));
        ballShadow.reset(new Cylinder(vec3(0,0,0), vec3(0,0,0.1), ballRadius * 0.7, shadowColor));
        
        createTable();
        resetBall();
    }
    
    App::~App() {}
    
    void App::resetBall(){
        ballPos = vec3(0, 5, -0.4 * tableLength);
        ballVelocity = vec3(0, 25, 40);
    }
    
    void App::updateBallPos(float rdt) {
        ballPos += rdt * (ballVelocity + 0.5f * vec3(0, GRAVITY, 0) * rdt);
        ballVelocity += vec3(0, GRAVITY, 0) * rdt;
        
    }
    
    void App::checkPaddleCollision() {
        if (ballVelocity.z < 0) {return;}
        
        vec3 paddlePos = column(paddleFrame, 3);
        vec3 prevPaddlePos = column(prevPaddleFrame, 3);
        Line paddlePath(prevPaddlePos, paddlePos, vec3(0,0,1), 0.1, vec4(0,0,0,0));
        
        vec3 closestPt = paddlePath.closestPoint(ballPos);
        float distance = sqrt(pow((closestPt.x - ballPos.x), 2) + pow((closestPt.y - ballPos.y), 2) + pow((closestPt.z - ballPos.z), 2));
        
        if (distance < paddleRadius) {
            //reflect ball's velocity vector accross normal
            ballVelocity = reflect(ballVelocity, getPaddleNormal());
            ballVelocity *= 1.2f;
            ballPos.y = paddlePos.y - ballRadius - 10;
            
            // Update lateral movement and ball speed based on paddle hit
            vec3 paddleDifference = prevPaddlePos - paddlePos;
            ballVelocity.x -= paddleDifference.x * 10;
            ballVelocity.z -= paddleDifference.z * 10;
            
            lastPlayerHit = 1;
        }
    }
    
    void App::checkMirrorPaddleCollision() {
        if (ballVelocity.z > 0) {return;}
        
        vec3 mirrorPaddlePos = column(paddleFrame, 3);
        mirrorPaddlePos.z = -mirrorPaddlePos.z;
        vec3 prevMirrorPaddlePos = column(prevPaddleFrame, 3);
        prevMirrorPaddlePos.z = -prevMirrorPaddlePos.z;
        Line paddlePath(prevMirrorPaddlePos, mirrorPaddlePos, vec3(0,0,1), 0.1, vec4(0,0,0,0));
        
        vec3 closestPt = paddlePath.closestPoint(ballPos);
        float distance = sqrt(pow((closestPt.x - ballPos.x), 2) + pow((closestPt.y - ballPos.y), 2) + pow((closestPt.z - ballPos.z), 2));
        
        if (distance < paddleRadius) {
            //reflect ball's velocity vector accross normal
            vec3 paddleNormal = getPaddleNormal();
            paddleNormal.z = -paddleNormal.z;
            ballVelocity = reflect(ballVelocity, paddleNormal);
            ballVelocity *= 1.2f;
            ballPos.y = mirrorPaddlePos.y - ballRadius + 10;
            
            // Update lateral movement and ball speed based on paddle hit
            vec3 paddleDifference = prevMirrorPaddlePos - mirrorPaddlePos;
            ballVelocity.x -= paddleDifference.x * 10;
            ballVelocity.z -= paddleDifference.z * 10;
            
            lastPlayerHit = 0;
        }
    }
    
    void App::checkTableCollision() {
        // Check if ball hit the table
        if ((ballPos.y <= 0.0 + ballRadius) && ballOnTable()) {
            ballPos.y = ballRadius;
            ballVelocity = reflect(ballVelocity, vec3(0,1,0));
            ballVelocity *= 0.85f;
            
            // Check if someone won
            if ((lastZHit < 0) && (ballPos.z < 0)) {
                winner(1);
            } else if ((lastZHit > 0) && (ballPos.z > 0)) {
                winner(0);
            }
        }
    }
    
    void App::winner(int player) {
        // server = 0, player = 1
        if (player) {
            std::cout << "Hey player: nice shot!" << std::endl;
        } else {
            std::cout << "Hey server: nice shot!" << std::endl;
        }
    }

    
    bool App::ballOnTable() {
        if (ballPos.z < -0.5 * tableLength ||
            ballPos.z > 0.5 * tableLength ||
            ballPos.x < -0.5 * tableWidth ||
            ballPos.x > 0.5 * tableWidth) {
            return false;
        }
        
        if (ballPos.y < -ballRadius) {
            return false;
        }
        return true;
    }
    
    bool App::paddleOnTable() {
        vec3 paddlePosition = paddleFrame[3];
        if (paddlePosition.z < -0.5 * tableLength ||
            paddlePosition.z > 0.5 * tableLength ||
            paddlePosition.x < -0.5 * tableWidth ||
            paddlePosition.x > 0.5 * tableWidth) {
            return false;
        }
        return true;
    }
    
    
    void App::checkNetCollision() {
        // Check if ball hit the net
        if ((-ballRadius <= ballPos.z) &&
            (ballPos.z <= ballRadius) &&
            (ballPos.y < netHeight + ballRadius) &&
            (-0.5 * 1.2 * tableWidth < ballPos.x) &&
            (ballPos.x < 0.5 * 1.2 * tableWidth)) {
            
            if (ballVelocity.z < 0) {
                ballVelocity = reflect(ballVelocity, vec3(0,0,1));
            }
            else if (ballVelocity.y == reflect(ballVelocity, vec3(0,0,1)).y){
                ballVelocity = vec3(ballVelocity.x, ballRadius, ballVelocity.z);
            }
            else {
                ballVelocity = reflect(ballVelocity, vec3(0,0,-1));
            }

            ballVelocity *= 0.5f;
            
            // Check who won
            if (ballPos.z < 0) {
                winner(1);
            } else {
                winner(0);
            }
        }
    }
    
    void App::checkBallGone() {
        if (ballPos.y < -10) {
            // Server hit ball and it bounced on his side last
            if ((lastZHit < 0) && (lastPlayerHit == 0)) {
                winner(1);
            }
            // Player hit ball and bounced on his side last
            else if((lastZHit > 0) && (lastPlayerHit == 1)) {
                winner(0);
            }
            // Server hit ball and bounced on other side last
            else if((lastZHit > 0) && (lastPlayerHit == 0)) {
                winner(1);
            }
            // Player hit ball and it bounced on other side last
            else if((lastZHit < 0) && (lastPlayerHit == 1)) {
                winner(0);
            }
            resetBall();
        }
    }
    
    void App::createTable() {
        // Create base
        base.reset(new Box(vec3(-0.5 * 0.75 * tableWidth, -tableHeight, -0.5 * 0.75 * tableLength),
                           vec3(0.5 * 0.75 * tableWidth, -5, 0.5 * 0.75 * tableLength),
                           vec4(0.5, 0.5, 0.5, 1.0)));
        
        // Create table
        table.reset(new Box(vec3(-0.5 * tableWidth, -5, -0.5 * tableLength),
                            vec3(0.5 * tableWidth, 0, 0.5 * tableLength),
                            vec4(0.0, 0.6, 0.3, 1.0)));
        
        // Table details
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(-0.5 * tableWidth + 1, 0.1, 0.5 * tableLength), vec3(-0.5 * tableWidth + 1, 0.1, -0.5 * tableLength), vec3(0,1,0), 1, vec4(1,1,1,1))));
        
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(0, 0.1, 0.5 * tableLength), vec3(0, 0.1, -0.5 * tableLength), vec3(0,1,0), 1, vec4(1,1,1,1))));
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(0.5 * tableWidth - 1, 0.1, 0.5 * tableLength), vec3(0.5 * tableWidth - 1, 0.1, -0.5 * tableLength), vec3(0,1,0), 1, vec4(1,1,1,1))));
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(0.5 * tableWidth-1, 0.1, -0.5 * tableLength), vec3(-0.5 * tableWidth+1, 0.1, -0.5 * tableLength), vec3(0,1,0), 1, vec4(1,1,1,1))));
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(0.5 * tableWidth - 1, 0.1, 0.5 * tableLength), vec3(-0.5 * tableWidth+1, 0.1, 0.5 * tableLength), vec3(0,1,0), 1, vec4(1,1,1,1))));
        
        
        // Create net
        for (int i = 2; i < netHeight; i+=2) {
            tableDetails.push_back(std::unique_ptr<Line>(new Line(vec3(-0.5 * 1.2 * tableWidth, 0.1 + i, 0), vec3(0.5 * 1.2 * tableWidth, 0.1 + i, 0), vec3(0, 0, 1), 0.2, vec4(0,0,0,1))));
        }
        
        for (int i = 2; i < (tableWidth * 1.2); i+=2) {
            tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(-0.5 * 1.2 * tableWidth + i, netHeight, 0), vec3(-0.5 * 1.2 * tableWidth + i, 0.1, 0), vec3(0, 0, 1), 0.2, vec4(0,0,0,1))));
        }
        
        // Create net outline
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(-0.5 * 1.2 * tableWidth, netHeight, 0), vec3(0.5 * 1.2 * tableWidth , netHeight, 0), vec3(0, 0, 1), 0.4, vec4(0.8, 0.8, 0.8, 0.8))));
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(-0.5 * 1.2 * tableWidth, 0.1, 0), vec3(0.5 * 1.2 * tableWidth , 0.1, 0), vec3(0, 0, 1), 0.4, vec4(0.8, 0.8, 0.8, 0.8))));
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(-0.5 * 1.2 * tableWidth, netHeight, 0), vec3(-0.5 * 1.2 * tableWidth, 0.1, 0), vec3(0, 0, 1), 0.4, vec4(0.8, 0.8, 0.8, 0.8))));
        tableDetails.push_back(std::unique_ptr<Line> (new Line(vec3(0.5 * 1.2 * tableWidth, netHeight, 0), vec3(0.5 * 1.2 * tableWidth, 0.1, 0), vec3(0, 0, 1), 0.4, vec4(0.8, 0.8, 0.8, 0.8))));
    }
    
    mat4 App::getBallShadowTranslation() {
        vec3 ballShadowPos = ballPos;
        ballShadowPos.y = 0;
        mat4 shadowTranslation = translate(mat4(1.0), ballShadowPos);
        return shadowTranslation;
    }
    
    mat4 App::getPaddleShadowTranslation() {
        vec3 paddleShadowPos = paddleFrame[3];
        paddleShadowPos.y = 0;
        mat4 shadowTranslation = translate(mat4(1.0), paddleShadowPos);
        return shadowTranslation;
    }
    
    mat4 App::getMirrorPaddleShadowTranslation() {
        vec3 paddleShadowPos = paddleFrame[3];
        paddleShadowPos.z = -paddleShadowPos.z;
        paddleShadowPos.y = 0;
        mat4 shadowTranslation = translate(mat4(1.0), paddleShadowPos);
        return shadowTranslation;
    }
    
    void App::onEvent(shared_ptr<Event> event) {
        string name = event->getName();
        if (name == "kbd_ESC_down") {
            glfwSetWindowShouldClose(_window, 1);
        }
        else if (name == "mouse_pointer") {
            vec2 mouseXY = event->get2DData();
            
            int width, height;
            glfwGetWindowSize(_window, &width, &height);
            
            // This block of code maps the 2D position of the mouse in screen space to a 3D position
            // 20 cm above the ping pong table.  It also rotates the paddle to make the handle move
            // in a cool way.  It also makes sure that the paddle does not cross the net and go onto
            // the opponent's side.
            float xneg1to1 = mouseXY.x / width * 2.0 - 1.0;
            float y0to1 = mouseXY.y / height;
            mat4 rotZ = toMat4(angleAxis(glm::sin(-xneg1to1), vec3(0, 0, 1)));
            
            glm::vec3 lastPaddlePos = glm::column(paddleFrame, 3);
            prevPaddleFrame = paddleFrame;
            paddleFrame = glm::translate(mat4(1.0), vec3(xneg1to1 * 100.0, 20.0, glm::max(y0to1 * 137.0 + 20.0, 0.0))) * rotZ;
            vec3 newPos = glm::column(paddleFrame, 3);
            
            // This is a weighted average.  Update the velocity to be 10% the velocity calculated
            // at the previous frame and 90% the velocity calculated at this frame.
            paddleVel = 0.1f*paddleVel + 0.9f*(newPos - lastPaddlePos);
        }
        else if (name == "kbd_SPACE_up") {
            // This is where you can "serve" a new ball from the opponent's side of the net
            // toward you when the spacebar is released. I found that a good initial position for the ball is: (0, 30, -130).
            // And, a good initial velocity is (0, 200, 400).  As usual for this program, all
            // units are in cm.
            resetBall();
        }
    }
    
    
    void App::onSimulation(double rdt) {
        // rdt is the change in time (dt) in seconds since the last call to onSimulation
        // So, you can slow down the simulation by half if you divide it by 2.
        rdt *= 1;
        updateBallPos(rdt);
        
        // Check if the ball hit the paddle
        checkPaddleCollision();
        checkMirrorPaddleCollision();
        
        // Check if the ball hit the table
        checkTableCollision();
        checkNetCollision();
        checkBallGone();
    }
    
    
    void App::onRenderGraphics() {
        
        double curTime = glfwGetTime();
        onSimulation(curTime - lastTime);
        lastTime = curTime;
        
        // Setup the camera with a good initial position and view direction to see the table
        glm::vec3 eye_world = glm::vec3(0, 100, 250);
        glm::mat4 view = glm::lookAt(eye_world, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        //eye_world = glm::vec3(glm::column(glm::inverse(view), 3));
        
        // Setup the projection matrix so that things are rendered in perspective
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)_windowWidth / (GLfloat)_windowHeight, 0.1f, 500.0f);
        // Setup the model matrix
        glm::mat4 model = glm::mat4(1.0);
        
        // Update shader variables
        _shader.setUniform("view_mat", view);
        _shader.setUniform("projection_mat", projection);
        _shader.setUniform("model_mat", model);
        _shader.setUniform("eye_world", eye_world);
        
        // Draw the table and ball here
        base->draw(_shader, mat4(1.0));
        table->draw(_shader, mat4(1.0));
        for (int i = 0; i < tableDetails.size(); i++) {
            tableDetails[i]->draw(_shader, mat4(1.0));
        }
        
        // Draw shadows
        if (ballOnTable()) {
            ballShadow->draw(_shader, getBallShadowTranslation());
        }
        if (paddleOnTable()) {
            paddleShadow->draw(_shader, getPaddleShadowTranslation());
            mirrorPaddleShadow->draw(_shader, getMirrorPaddleShadowTranslation());
        }
        
        mat4 translation = translate(mat4(1.0), ballPos);
        ball->draw(_shader, translation * model);
        
        // Draw the paddle using two cylinders
        paddle->draw(_shader, paddleFrame);
        handle->draw(_shader, paddleFrame);
        
        mat4 mirrorPaddleFrame = paddleFrame;
        mirrorPaddleFrame[3].z = -paddleFrame[3].z;
        mirrorPaddle->draw(_shader, mirrorPaddleFrame);
        mirrorHandle->draw(_shader, mirrorPaddleFrame);
        
        // Check for any opengl errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
    }
    
}
