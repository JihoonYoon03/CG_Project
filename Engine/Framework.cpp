#include "pch.h"
#include "Framework.h"

Framework* Framework::current_ = nullptr;

namespace
{
    constexpr int UpdateIntervalMs = 1000 / 240;

    void DrawText(const std::string& text, float x, float y, float scale)
    {
        glPushMatrix();
        glTranslatef(x, y, 0.0f);
        glScalef(scale, scale, scale);
        for (unsigned char character : text)
        {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, character);
        }
        glPopMatrix();
    }
} // namespace

int Framework::Run(int, char** argv, std::unique_ptr<Scene> scene, bool smokeTest, const std::string& capturePath)
{
    current_ = this;
    smokeTest_ = smokeTest;
    capturePath_ = capturePath;
    try
    {
        int glutArgc = 1;
        glutInit(&glutArgc, argv);
        glutInitContextVersion(3, 3);
        // 기존 GLUT 글꼴과 디버그 선은 호환 프로파일에서 그립니다.
        glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
        glutInitWindowSize(width_, height_);
        glutInitWindowPosition(100, 100);
        window_ = glutCreateWindow("Example1");
        if (window_ <= 0)
        {
            throw std::runtime_error("Cannot create OpenGL window");
        }
        glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
        if (smokeTest_)
        {
            glutHideWindow();
        }
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            throw std::runtime_error("GLEW initialization failed");
        }
        while (glGetError() != GL_NO_ERROR)
        {
        } // GLEW 초기화 과정의 기존 오류 상태 제거
        if (!GLEW_VERSION_3_3)
        {
            throw std::runtime_error("OpenGL 3.3 is required");
        }
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        scene_ = std::move(scene);
        scene_->Initialize();

        glutDisplayFunc(DisplayCallback);
        glutReshapeFunc(ReshapeCallback);
        glutKeyboardFunc(KeyDownCallback);
        glutKeyboardUpFunc(KeyUpCallback);
        glutMouseFunc(MouseCallback);
        glutMotionFunc(MotionCallback);
        glutPassiveMotionFunc(MotionCallback);
        glutCloseFunc(CloseCallback);
        if (!smokeTest_)
        {
            glutSetCursor(GLUT_CURSOR_NONE);
        }
        previousTime_ = std::chrono::steady_clock::now();
        glutTimerFunc(UpdateIntervalMs, TimerCallback, 0);
        glutMainLoop();
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        exitCode_ = 1;
    }
    // OpenGL 컨텍스트가 살아 있을 때 GPU 자원을 먼저 해제합니다.
    scene_.reset();
    if (window_ > 0)
    {
        const int window = window_;
        window_ = 0;
        glutDestroyWindow(window);
    }
    current_ = nullptr;
    return exitCode_;
}

void Framework::Tick()
{
    const auto now = std::chrono::steady_clock::now();
    const float elapsed = std::chrono::duration<float>(now - previousTime_).count();
    previousTime_ = now;
    const float deltaTime = smokeTest_ ? 0.1f : elapsed;
    scene_->Update(deltaTime, input_);
    input_.EndFrame();
    if (!smokeTest_ && elapsed > 0.0f)
    {
        glutSetWindowTitle(("FPS: " + std::to_string(static_cast<int>(1.0f / elapsed))).c_str());
    }
    if (smokeTest_)
    {
        Render(); // 숨긴 창에는 display 콜백이 오지 않을 수 있습니다.
    }
    else
    {
        glutPostRedisplay();
    }
}

void Framework::UploadLights(const Shader& shader) const
{
    shader.Set("ambientLight", scene_->ambientLight);
    int count = 0;
    for (const auto& light : scene_->GetLights())
    {
        if (!light.enabled)
        {
            continue;
        }
        if (count == Light::MaxCount)
        {
            break;
        }
        const std::string prefix = "lights[" + std::to_string(count++) + "].";
        shader.Set(prefix + "type", static_cast<int>(light.type));
        shader.Set(prefix + "color", light.color);
        shader.Set(prefix + "intensity", std::max(light.intensity, 0.0f));
        shader.Set(prefix + "position", light.position);
        shader.Set(prefix + "direction", light.direction);
        shader.Set(prefix + "range", std::max(light.range, 0.0f));
        const float outer = std::clamp(light.outerAngle, 0.1f, 89.0f);
        const float inner = std::clamp(light.innerAngle, 0.0f, outer - 0.1f);
        shader.Set(prefix + "innerCos", std::cos(glm::radians(inner)));
        shader.Set(prefix + "outerCos", std::cos(glm::radians(outer)));
    }
    shader.Set("lightCount", count);
}

void Framework::DrawObjects()
{
    const auto& camera = scene_->GetCamera();
    const glm::mat4 projection = camera.GetProjection(static_cast<float>(width_) / height_);
    const glm::mat4 view = camera.GetView();
    const Shader* previousShader = nullptr;
    for (const auto& object : scene_->GetObjects())
    {
        if (!object->active || !object->visible)
        {
            continue;
        }
        const auto& mesh = object->GetMesh();
        const auto& material = object->GetMaterial();
        if (!mesh || !material || !material->shader)
        {
            continue;
        }
        if (glm::any(glm::lessThan(glm::abs(object->transform.scale), glm::vec3(0.000001f))))
        {
            throw std::runtime_error("Object scale must be nonzero: " + object->name);
        }
        material->Apply();
        const Shader& shader = *material->shader;
        if (previousShader != &shader)
        {
            shader.Set("view", view);
            shader.Set("projection", projection);
            shader.Set("viewPosition", camera.position);
            UploadLights(shader);
            previousShader = &shader;
        }
        shader.Set("model", object->GetModelMatrix());
        mesh->Draw();
    }
}

void Framework::DrawColliders()
{
    if (!scene_->debugColliders)
    {
        return;
    }
    glUseProgram(0);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    const auto projection = scene_->GetCamera().GetProjection(static_cast<float>(width_) / height_);
    glLoadMatrixf(glm::value_ptr(projection));
    glMatrixMode(GL_MODELVIEW);
    const auto view = scene_->GetCamera().GetView();
    glLoadMatrixf(glm::value_ptr(view));
    glColor3f(1, 0, 0);
    for (const auto& object : scene_->GetObjects())
    {
        if (!object->active || object->collider == ColliderType::None)
        {
            continue;
        }
        glPushMatrix();
        if (object->collider == ColliderType::Box)
        {
            const auto box = object->GetWorldBox();
            const auto center = (box.min + box.max) * 0.5f;
            const auto size = box.max - box.min;
            glTranslatef(center.x, center.y, center.z);
            glScalef(size.x, size.y, size.z);
            glutWireCube(1.0);
        }
        else
        {
            const auto sphere = object->GetWorldSphere();
            glTranslatef(sphere.center.x, sphere.center.y, sphere.center.z);
            glutWireSphere(sphere.radius, 12, 8);
        }
        glPopMatrix();
    }
    glEnable(GL_CULL_FACE);
}

void Framework::DrawOverlay()
{
    const Hud hud = scene_->GetHud();
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (hud.crosshair)
    {
        glColor3f(1, 1, 1);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(0.025f, 0);
        glVertex2f(-0.025f, 0);
        glVertex2f(0, 0.025f);
        glVertex2f(0, -0.025f);
        glEnd();
        // 원본은 이후 HUD에도 선 두께 2를 유지합니다.
    }
    glColor3f(0, 0, 0);
    const std::array<glm::vec2, 4> positions{{{-0.99f, 0.9f}, {-0.99f, 0.8f}, {-0.3f, 0.9f}, {0.6f, 0.9f}}};
    for (size_t i = 0; i < hud.lines.size() && i < positions.size(); ++i)
    {
        DrawText(hud.lines[i], positions[i].x, positions[i].y, 0.0005f);
    }
    if (!hud.message.empty())
    {
        DrawText(hud.message, -0.6f, -0.15f, hud.messageScale);
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Framework::Render()
{
    if (!scene_)
    {
        return;
    }
    glViewport(0, 0, width_, height_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawObjects();
    DrawColliders();
    DrawOverlay();
    if (smokeTest_)
    {
        const GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            throw std::runtime_error("OpenGL error: " + std::to_string(error));
        }
        if (++frameCount_ == 40)
        {
            if (!capturePath_.empty())
            {
                CaptureFrame();
            }
            std::cout << "Render smoke test passed: " << scene_->GetObjects().size() << " objects, "
                      << scene_->GetLights().size() << " lights, 40 frames.\n";
            Close();
            return;
        }
    }
    glutSwapBuffers();
}

void Framework::CaptureFrame() const
{
    std::vector<unsigned char> pixels(static_cast<size_t>(width_) * height_ * 3);
    GLint alignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glPixelStorei(GL_PACK_ALIGNMENT, alignment);
    if (glGetError() != GL_NO_ERROR)
    {
        throw std::runtime_error("Framebuffer read failed");
    }
    std::ofstream output(capturePath_, std::ios::binary);
    if (!output)
    {
        throw std::runtime_error("Cannot save capture: " + capturePath_);
    }
    output << "P6\n" << width_ << ' ' << height_ << "\n255\n";
    for (int y = height_ - 1; y >= 0; --y)
    {
        output.write(reinterpret_cast<const char*>(pixels.data() + static_cast<size_t>(y) * width_ * 3), width_ * 3);
    }
}

void Framework::Close()
{
    glutLeaveMainLoop();
}

void Framework::Fail(const std::exception& error)
{
    std::cerr << error.what() << '\n';
    exitCode_ = 1;
    Close();
}

void Framework::DisplayCallback()
{
    try
    {
        current_->Render();
    }
    catch (const std::exception& e)
    {
        current_->Fail(e);
    }
}

void Framework::TimerCallback(int)
{
    if (!current_ || !current_->scene_)
    {
        return;
    }
    try
    {
        current_->Tick();
    }
    catch (const std::exception& e)
    {
        current_->Fail(e);
    }
    glutTimerFunc(UpdateIntervalMs, TimerCallback, 0);
}

void Framework::ReshapeCallback(int width, int height)
{
    current_->width_ = std::max(width, 1);
    current_->height_ = std::max(height, 1);
}

void Framework::KeyDownCallback(unsigned char key, int, int)
{
    if (key == 'q')
    {
        current_->Close();
        return;
    }
    current_->input_.SetKey(key, true);
}

void Framework::KeyUpCallback(unsigned char key, int, int)
{
    current_->input_.SetKey(key, false);
}

void Framework::MouseCallback(int button, int state, int, int)
{
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
    {
        return;
    }
    current_->input_.firePressed = true;
}

void Framework::MotionCallback(int x, int y)
{
    if (current_->smokeTest_)
    {
        return;
    }
    const int dx = x - current_->width_ / 2, dy = y - current_->height_ / 2;
    if (dx == 0 && dy == 0)
    {
        return;
    }
    current_->input_.mouseDelta += glm::vec2(dx, dy);
    glutWarpPointer(current_->width_ / 2, current_->height_ / 2);
}

void Framework::CloseCallback()
{
    // 창 X 버튼과 glutLeaveMainLoop 모두 컨텍스트 삭제 전에 이 콜백을 거칩니다.
    current_->scene_.reset();
    current_->window_ = 0;
}
