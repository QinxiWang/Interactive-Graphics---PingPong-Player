/**
 @file App.h
 
 The default starter app is configured for OpenGL 3.3 and relatively recent
 GPUs.
 */
#ifndef App_h
#define App_h

#include "BaseApp.h"
#include <vector>
#include <glad/glad.h>
#include <glfw/glfw3.h>

namespace basicgraphics {
    class App : public BaseApp {
    public:
        
        App(int argc, char** argv, std::string windowName, int windowWidth, int windowHeight);
        ~App();
        
    private:
        std::unique_ptr<Sphere> ball;
        std::unique_ptr<Box> table;
        std::unique_ptr<Box> base;
        std::vector<std::unique_ptr<Line> > tableDetails;
        std::unique_ptr<Cylinder> ballShadow;
        std::unique_ptr<Box> paddleShadow;
        std::unique_ptr<Box> mirrorPaddleShadow;
        
        double tableWidth;
        double tableHeight;
        double tableLength;
        double netHeight;
        double paddleRadius;
        double paddleWidth;
        double ballRadius;
        double lastZHit;
        double lastPlayerHit;
        
        glm::vec2 initialVel;
        glm::vec3 initPos;
        
        glm::vec3 ballPos;
        glm::vec3 ballVelocity;
        
        bool ballOnTable();
        bool paddleOnTable();
        void createTable();
        void resetBall();
        void checkTableCollision();
        void checkNetCollision();
        void checkPaddleCollision();
        void checkMirrorPaddleCollision();
        void updateBallPos(float rdt);
        void checkBallGone();
        void winner(int player);
        mat4 getBallShadowTranslation();
        mat4 getPaddleShadowTranslation();
        mat4 getMirrorPaddleShadowTranslation();
        
        static const float GRAVITY;
        
    protected:
        
        void onRenderGraphics() override;
        void onEvent(std::shared_ptr<Event> event) override;
        void onSimulation(double rdt);
        
        // Use these functions to access the current state of the paddle!
        glm::vec3 getPaddlePosition() { return glm::column(paddleFrame, 3); }
        glm::vec3 getPaddleNormal() { return glm::vec3(0,0,-1);}
        glm::vec3 getPaddleVelocity() { return paddleVel; }
        
        // The paddle is drawn with two cylinders
        std::unique_ptr<Cylinder> paddle;
        std::unique_ptr<Cylinder> handle;
        std::unique_ptr<Cylinder> mirrorPaddle;
        std::unique_ptr<Cylinder> mirrorHandle;
        
        
        // This 4x4 matrix stores position and rotation data for the paddle.
        glm::mat4 paddleFrame;
        glm::mat4 prevPaddleFrame;
        
        // This vector stores the paddle's current velocity.
        glm::vec3 paddleVel;
        
        // This holds the time value for the last time onSimulate was called
        double lastTime;
    };
}

#endif
