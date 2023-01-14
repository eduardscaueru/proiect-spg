#include "lab_m2/Proiect/Proiect.h"

#include <vector>
#include <iostream>

using namespace std;
using namespace m2;

struct GerstnerWave {
    glm::vec2 direction;
    float amplitude;
    float steepness;
    float frequency;
    float speed;
    glm::vec2 padding;
};
vector<struct GerstnerWave> gerstner_waves;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

float randomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

Proiect::Proiect()
{
}


Proiect::~Proiect()
{
}


void Proiect::Init()
{
    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, 8, 8), glm::quat(glm::vec3(-40 * TO_RADIANS, 0, 0)));
    camera->Update();

    //ToggleGroundPlane();

    // Create a shader program for surface generation
    {
        Shader* shader = new Shader("SurfaceGeneration");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Proiect", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        //shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Proiect", "shaders", "GeometryShader.glsl"), GL_GEOMETRY_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Proiect", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    no_gerstner_waves = 50;
    for (int i = 0; i < no_gerstner_waves; i++) {
        struct GerstnerWave wave = struct GerstnerWave();
        if (i < 15) {
            wave.direction = glm::vec2(0.2, -0.8);
        }
        else if (i >= 15 && i < 30) {
            wave.direction = glm::vec2(-1, 1);
        }
        else {
            wave.direction = glm::vec2(1, -1);
        }
        wave.amplitude = randomFloat(0.5f, 1.f);
        wave.frequency = randomFloat(0.f, 1.f);
        wave.speed = randomFloat(1.f, 3.f);
        wave.steepness = randomFloat(0.0f, 1.0f);

        gerstner_waves.push_back(wave);
    }
    time = 0.0f;
    gerstner_waves_length = 1;

    {
        vector<VertexFormat> vertices;
        vector<unsigned int> indices;
        Mesh* mesh = new Mesh("water");
        int posX = 0;
        int posY = 5;
        int posZ = 0;
        int index = 0;
        for (int i = 0; i < 50; i++) {
            posX = 0;
            for (int j = 0; j < 50; j++) {
                vertices.push_back(VertexFormat(glm::vec3(posX, posY, posZ)));
                vertices.push_back(VertexFormat(glm::vec3(posX + 1, posY, posZ)));
                vertices.push_back(VertexFormat(glm::vec3(posX, posY, posZ + 1)));
                vertices.push_back(VertexFormat(glm::vec3(posX + 1, posY, posZ + 1)));

                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + 2);
                indices.push_back(index + 1);
                indices.push_back(index + 3);
                indices.push_back(index + 2);

                posX++;
                index += 4;
            }

            posZ++;
        }
        mesh->SetDrawMode(GL_TRIANGLES);
        mesh->InitFromData(vertices, indices);

        meshes[mesh->GetMeshID()] = mesh;
    }
}


void Proiect::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Proiect::RenderMeshInstanced(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int instances, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader
    glUseProgram(shader->program);

    // Bind model matrix
    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Bind view matrix
    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Draw the object instanced
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instances);
}


void Proiect::Update(float deltaTimeSeconds)
{
    ClearScreen(glm::vec3(0.121, 0.168, 0.372));

    auto& mesh = meshes["water"];
    auto& shader = shaders["SurfaceGeneration"];

    shader->Use();
    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));

    // Bind view matrix
    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    for (int i = 0; i < no_gerstner_waves; i++) {
        string s = "gerstner_waves[";
        s.append(std::to_string(i));
        s.append("].direction");
        glUniform2f(glGetUniformLocation(shader->program, s.c_str()), gerstner_waves[i].direction.x, gerstner_waves[i].direction.y);
        s = "gerstner_waves[";
        s.append(std::to_string(i));
        s.append("].amplitude");
        glUniform1f(glGetUniformLocation(shader->program, s.c_str()), gerstner_waves[i].amplitude);
        s = "gerstner_waves[";
        s.append(std::to_string(i));
        s.append("].steepness");
        glUniform1f(glGetUniformLocation(shader->program, s.c_str()), gerstner_waves[i].steepness);
        s = "gerstner_waves[";
        s.append(std::to_string(i));
        s.append("].frequency");
        glUniform1f(glGetUniformLocation(shader->program, s.c_str()), gerstner_waves[i].frequency);
        s = "gerstner_waves[";
        s.append(std::to_string(i));
        s.append("].speed");
        glUniform1f(glGetUniformLocation(shader->program, s.c_str()), gerstner_waves[i].speed);
    }
    time += deltaTimeSeconds;
    glUniform1f(glGetUniformLocation(shader->program, "time"), time);
    glUniform1i(glGetUniformLocation(shader->program, "gerstner_waves_length"), gerstner_waves_length);

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0);
}


void Proiect::FrameEnd()
{
    //DrawCoordinateSystem();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Proiect::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input

    // You can move the control points around by using the dedicated key,
    // in combination with Ctrl, Shift, or both.
    float delta = deltaTime;
    auto keyMaps = std::vector<std::pair<glm::vec3&, uint32_t>>
    {
        { control_p0, GLFW_KEY_1 },
        { control_p1, GLFW_KEY_2 },
        { control_p2, GLFW_KEY_3 },
        { control_p3, GLFW_KEY_4 }
    };

    for (const auto& k : keyMaps)
    {
        if (window->KeyHold(k.second))
        {
            if (mods & GLFW_MOD_SHIFT && mods & GLFW_MOD_CONTROL)
            {
                k.first.y -= delta;
            }
            else if (mods & GLFW_MOD_CONTROL)
            {
                k.first.y += delta;
            }
            else if (mods & GLFW_MOD_SHIFT)
            {
                k.first.x -= delta;
            }
            else
            {
                k.first.x += delta;
            }

            std::cout << glm::vec2(control_p0) << glm::vec2(control_p1) << glm::vec2(control_p2) << glm::vec2(control_p3) << "\n";
        }
    }
}


void Proiect::OnKeyPress(int key, int mods)
{
    // TODO(student): Use keys to change the number of instances and the
    // number of generated points. Avoid the camera keys, and avoid the
    // the keys from `OnInputUpdate`.
    if (key == GLFW_KEY_DOWN) {
        no_of_generated_points--;
    }
    if (key == GLFW_KEY_UP) {
        no_of_generated_points++;
    }
}


void Proiect::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Proiect::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Proiect::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Proiect::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Proiect::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Proiect::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
