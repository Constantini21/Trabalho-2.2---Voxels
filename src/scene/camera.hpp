#ifndef __CAMERA_HPP_
#define __CAMERA_HPP_

#include "../scene/glslprogram.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>


class Camera {
    private:
        bool orthogonal;
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 right;
        glm::vec3 up;


        float width;
        float height;

        float fov;
        float near;
        float far;

        glm::mat4 view_mat;
        glm::mat4 orthogonal_mat;
        glm::mat4 perspective_mat;

        float pitch;
        float yaw;

        Camera() = delete;

        void updateViewMatrix();
        void updateProjectionMatrices();


        static bool boosted;
        static float speed;
        static float boosted_speed;
        static float sensibility;
        static float zoom_factor;


    public:        
        enum Movement {
            RIGHT,
            LEFT,
            UP,
            DOWN,
            FRONT,
            BACK
        };

        Camera(const int &width, const int &height, const bool &orthogonal = false);
        bool isOrthogonal() const;

        glm::vec3 getPosition() const;
        glm::vec3 getDirection() const;
        glm::vec3 getRotation() const;
        glm::vec3 getUp() const;
        float getFOV() const;
        glm::vec2 getResolution() const;
        glm::vec2 getClipping() const;
        glm::mat4 getViewMatrix() const;
        glm::mat4 getOrthogonalMatrix() const;
        glm::mat4 getPerspectiveMatrix() const;
        glm::mat4 getProjectionMatrix() const;
        void setOrthogonal(const bool &status);
        void setPosition(const glm::vec3 &new_position);
        void setDirection(const glm::vec3 &direction);
        void setRotation(const glm::vec3 &angle);
        void setUp(const glm::vec3 &direction);
        void setFOV(const float &new_fov);
        void setResolution(const glm::vec2 &resolution);
        void setClipping(const glm::vec2 &clipping);
        void reset();
        void bind(GLSLProgram *const program) const;
        void travell(const Camera::Movement &direction, const double &time = 1.0 / 30.0);
        void translate(const glm::vec3 &delta);
        void zoom(const double &direction);
        void rotate(const glm::vec2 &delta);
        virtual ~Camera();
        static bool isBoosted();
        static float getSpeed();
        static float getBoostedSpeed();
        static float getSensibility();
        static float getZoomFactor();
        static void setBoosted(const bool &status);
        static void setSpeed(const float &speed);
        static void setBoostedSpeed(const float &speed);
        static void setSensibility(const float &sesibility);
        static void setZoomFactor(const float &factor);
};

#endif 