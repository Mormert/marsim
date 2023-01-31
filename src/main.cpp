// MIT License

// Copyright (c) 2023 Erin Catto (Box2D author), Johan Lind, Ermias Tewolde

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define _CRT_SECURE_NO_WARNINGS
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1

#include "imgui/imgui.h"
#include "framework/imgui_impl_glfw.h"
#include "framework/imgui_impl_opengl3.h"
#include "framework/draw.h"
#include "framework/settings.h"
#include "simulation.h"
#include "mqtt.h"
#include "robot.h"
#include "volcano.h"

#include <algorithm>
#include <stdio.h>
#include <thread>
#include <chrono>

#if defined(_WIN32)
#include <crtdbg.h>
#endif

GLFWwindow* g_mainWindow = nullptr;
static Application* s_application = nullptr;
static Settings s_settings;
static bool s_rightMouseDown = false;
static b2Vec2 s_clickPointWS = b2Vec2_zero;
static float s_displayScale = 1.0f;
static const float s_imguiPadding = 8.f;

void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "GLFW error occured. Code: %d. Description: %s\n", error, description);
}

static void RestartSimulation()
{
	delete s_application;
    s_application = Simulation::Create();
    Mqtt::getInstance().setSimulationPtr(dynamic_cast<Simulation *>(s_application));
}

void SetupImGuiStyle()
{
    constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
    {
        return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
    };

    auto& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    const ImVec4 bgColor           = ColorFromBytes(37, 37, 38);
    const ImVec4 lightBgColor      = ColorFromBytes(82, 82, 85);
    const ImVec4 veryLightBgColor  = ColorFromBytes(90, 90, 95);

    const ImVec4 panelColor        = ColorFromBytes(51, 51, 55);
    const ImVec4 panelHoverColor   = ColorFromBytes(29, 151, 236);
    const ImVec4 panelActiveColor  = ColorFromBytes(0, 119, 200);

    const ImVec4 textColor         = ColorFromBytes(255, 255, 255);
    const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
    const ImVec4 borderColor       = ColorFromBytes(78, 78, 78);

    colors[ImGuiCol_Text]                 = textColor;
    colors[ImGuiCol_TextDisabled]         = textDisabledColor;
    colors[ImGuiCol_TextSelectedBg]       = panelActiveColor;
    colors[ImGuiCol_WindowBg]             = bgColor;
    colors[ImGuiCol_ChildBg]              = bgColor;
    colors[ImGuiCol_PopupBg]              = bgColor;
    colors[ImGuiCol_Border]               = borderColor;
    colors[ImGuiCol_BorderShadow]         = borderColor;
    colors[ImGuiCol_FrameBg]              = panelColor;
    colors[ImGuiCol_FrameBgHovered]       = panelHoverColor;
    colors[ImGuiCol_FrameBgActive]        = panelActiveColor;
    colors[ImGuiCol_TitleBg]              = bgColor;
    colors[ImGuiCol_TitleBgActive]        = bgColor;
    colors[ImGuiCol_TitleBgCollapsed]     = bgColor;
    colors[ImGuiCol_MenuBarBg]            = panelColor;
    colors[ImGuiCol_ScrollbarBg]          = panelColor;
    colors[ImGuiCol_ScrollbarGrab]        = lightBgColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    colors[ImGuiCol_ScrollbarGrabActive]  = veryLightBgColor;
    colors[ImGuiCol_CheckMark]            = panelActiveColor;
    colors[ImGuiCol_SliderGrab]           = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive]     = panelActiveColor;
    colors[ImGuiCol_Button]               = panelColor;
    colors[ImGuiCol_ButtonHovered]        = panelHoverColor;
    colors[ImGuiCol_ButtonActive]         = panelHoverColor;
    colors[ImGuiCol_Header]               = panelColor;
    colors[ImGuiCol_HeaderHovered]        = panelHoverColor;
    colors[ImGuiCol_HeaderActive]         = panelActiveColor;
    colors[ImGuiCol_Separator]            = borderColor;
    colors[ImGuiCol_SeparatorHovered]     = borderColor;
    colors[ImGuiCol_SeparatorActive]      = borderColor;
    colors[ImGuiCol_ResizeGrip]           = bgColor;
    colors[ImGuiCol_ResizeGripHovered]    = panelColor;
    colors[ImGuiCol_ResizeGripActive]     = lightBgColor;
    colors[ImGuiCol_PlotLines]            = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered]     = panelHoverColor;
    colors[ImGuiCol_PlotHistogram]        = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    //colors[ImGuiCol_ModalWindowDarkening] = bgColor;
    colors[ImGuiCol_DragDropTarget]       = bgColor;
    colors[ImGuiCol_NavHighlight]         = bgColor;
    //colors[ImGuiCol_DockingPreview]       = panelActiveColor;
    colors[ImGuiCol_Tab]                  = bgColor;
    colors[ImGuiCol_TabActive]            = panelActiveColor;
    colors[ImGuiCol_TabUnfocused]         = bgColor;
    colors[ImGuiCol_TabUnfocusedActive]   = panelActiveColor;
    colors[ImGuiCol_TabHovered]           = panelHoverColor;

    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 0.0f;
    style.FrameRounding     = 0.0f;
    style.GrabRounding      = 0.0f;
    style.PopupRounding     = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding       = 0.0f;
    style.FramePadding = ImVec2{s_imguiPadding * ImGui::GetIO().FontGlobalScale,s_imguiPadding * ImGui::GetIO().FontGlobalScale};
}

static void CreateUI(GLFWwindow* window, const char* glslVersion = NULL)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	bool success;
	success = ImGui_ImplGlfw_InitForOpenGL(window, false);
	if (success == false)
	{
		printf("ImGui_ImplGlfw_InitForOpenGL failed\n");
		assert(false);
	}

	success = ImGui_ImplOpenGL3_Init(glslVersion);
	if (success == false)
	{
		printf("ImGui_ImplOpenGL3_Init failed\n");
		assert(false);
	}

	// Search for font file
	const char* fontPath1 = "data/droid_sans.ttf";
	const char* fontPath2 = "../data/droid_sans.ttf";
	const char* fontPath = nullptr;
	FILE* file1 = fopen(fontPath1, "rb");
	FILE* file2 = fopen(fontPath2, "rb");
	if (file1)
	{
		fontPath = fontPath1;
		fclose(file1);
	}

	if (file2)
	{
		fontPath = fontPath2;
		fclose(file2);
	}

	if (fontPath)
	{
		ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath, 12.0f * s_displayScale);
	}

    SetupImGuiStyle();
}

static void ResizeWindowCallback(GLFWwindow*, int width, int height)
{
	g_camera.m_width = width;
	g_camera.m_height = height;
	s_settings.m_windowWidth = width;
	s_settings.m_windowHeight = height;

    ImGui::GetIO().FontGlobalScale = width / 1920.f;

    auto& style = ImGui::GetStyle();
    style.FramePadding = ImVec2{s_imguiPadding * ImGui::GetIO().FontGlobalScale,s_imguiPadding * ImGui::GetIO().FontGlobalScale};
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		return;
	}

	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			// Quit
			// glfwSetWindowShouldClose(g_mainWindow, GL_TRUE);
			break;

		case GLFW_KEY_LEFT:
			// Pan left
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(2.0f, 0.0f);
				s_application->ShiftOrigin(newOrigin);
			}
			else
			{
				g_camera.m_center.x -= 0.5f;
			}
			break;

		case GLFW_KEY_RIGHT:
			// Pan right
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(-2.0f, 0.0f);
				s_application->ShiftOrigin(newOrigin);
			}
			else
			{
				g_camera.m_center.x += 0.5f;
			}
			break;

		case GLFW_KEY_DOWN:
			// Pan down
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(0.0f, 2.0f);
				s_application->ShiftOrigin(newOrigin);
			}
			else
			{
				g_camera.m_center.y -= 0.5f;
			}
			break;

		case GLFW_KEY_UP:
			// Pan up
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(0.0f, -2.0f);
				s_application->ShiftOrigin(newOrigin);
			}
			else
			{
				g_camera.m_center.y += 0.5f;
			}
			break;

		case GLFW_KEY_HOME:
			g_camera.ResetView();
			break;

		case GLFW_KEY_Z:
			// Zoom out
			g_camera.m_zoom = b2Min(1.1f * g_camera.m_zoom, 20.0f);
			break;

		case GLFW_KEY_X:
			// Zoom in
			g_camera.m_zoom = b2Max(0.9f * g_camera.m_zoom, 0.02f);
			break;

		case GLFW_KEY_R:
			RestartSimulation();
			break;

		case GLFW_KEY_O:
			s_settings.m_singleStep = true;
			break;

		case GLFW_KEY_P:
			s_settings.m_pause = !s_settings.m_pause;
			break;

		case GLFW_KEY_TAB:
			g_debugDraw.m_showUI = !g_debugDraw.m_showUI;

		default:
			if (s_application)
			{
				s_application->Keyboard(key);
			}
		}
	}
	else if (action == GLFW_RELEASE)
	{
		s_application->KeyboardUp(key);
	}
	// else GLFW_REPEAT
}

static void CharCallback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
}

static void MouseButtonCallback(GLFWwindow* window, int32 button, int32 action, int32 mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

	double xd, yd;
	glfwGetCursorPos(g_mainWindow, &xd, &yd);
	b2Vec2 ps((float)xd, (float)yd);

	// Use the mouse to move things around.
	if (button == GLFW_MOUSE_BUTTON_1)
	{
        //<##>
        //ps.Set(0, 0);
		b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
		if (action == GLFW_PRESS)
		{
			if (mods == GLFW_MOD_SHIFT)
			{
				s_application->ShiftMouseDown(pw);
			}
			else
			{
				s_application->MouseDown(pw);
			}
		}

		if (action == GLFW_RELEASE)
		{
			s_application->MouseUp(pw);
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_2)
	{
		if (action == GLFW_PRESS)
		{
			s_clickPointWS = g_camera.ConvertScreenToWorld(ps);
			s_rightMouseDown = true;
		}

		if (action == GLFW_RELEASE)
		{
			s_rightMouseDown = false;
		}
	}
}

static void MouseMotionCallback(GLFWwindow*, double xd, double yd)
{
	b2Vec2 ps((float)xd, (float)yd);

	b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
	s_application->MouseMove(pw);

	if (s_rightMouseDown)
	{
		b2Vec2 diff = pw - s_clickPointWS;
		g_camera.m_center.x -= diff.x;
		g_camera.m_center.y -= diff.y;
		s_clickPointWS = g_camera.ConvertScreenToWorld(ps);
	}
}

static void ScrollCallback(GLFWwindow* window, double dx, double dy)
{
	ImGui_ImplGlfw_ScrollCallback(window, dx, dy);
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	if (dy > 0)
	{
		g_camera.m_zoom /= 1.1f;
	}
	else
	{
		g_camera.m_zoom *= 1.1f;
	}
}

static void UpdateUI()
{
	float menuWidth = 180.0f * s_displayScale * ImGui::GetIO().FontGlobalScale;
	if (g_debugDraw.m_showUI)
	{
		ImGui::SetNextWindowPos({g_camera.m_width - menuWidth - 10.0f, 10.0f});
		ImGui::SetNextWindowSize({menuWidth, g_camera.m_height - 20.0f});

		ImGui::Begin("Tools", &g_debugDraw.m_showUI, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		if (ImGui::BeginTabBar("ControlTabs", ImGuiTabBarFlags_None))
		{

                        if (ImGui::BeginTabItem("MQTT"))
                        {
                                static char mqttConnectString[256]{"bd86ad7d91574abab844ada07ad73b31.s1.eu.hivemq.cloud"};
                                static int mqttConnectPort{8883};

                                ImVec2 button_sz = ImVec2(-1, 0);

                                if(Mqtt::getInstance().isConnected())
                                {
                                    std::string connectedTo{mqttConnectString};
                                    ImGui::TextWrapped("%s", std::string{"You are connected to: " + std::string{mqttConnectString}}.c_str());

                                    if(ImGui::Button("Disconnect", button_sz))
                                    {
                                        Mqtt::getInstance().disconnectMqtt();
                                    }
                                }else
                                {
                                    ImGui::InputText("Address", mqttConnectString, sizeof(mqttConnectString), ImGuiInputTextFlags_AutoSelectAll);

                                    ImGui::InputInt("Port", &mqttConnectPort, 0);

                                    if(ImGui::Button("Connect", button_sz))
                                    {
                                        std::string address{mqttConnectString};
                                        Mqtt::getInstance().connectMqtt(address, mqttConnectPort);
                                    }
                                }

                                ImGui::Separator();

                                ImGui::TextWrapped("Emit as MessagePack to optimize network communication?\nIn future, this will be the default!");
                                ImGui::Checkbox("Use MessagePack (send)", Mqtt::getInstance().useMessagePackBool());

                                ImGui::Separator();

                                ImGui::TextWrapped("Emit with compression? (send)");

                                ImGui::RadioButton("None", Mqtt::getInstance().getCompressionInt(), 0); ImGui::SameLine();
                                ImGui::RadioButton("GZip", Mqtt::getInstance().getCompressionInt(), 1); ImGui::SameLine();
                                ImGui::RadioButton("ZLib", Mqtt::getInstance().getCompressionInt(), 2); ImGui::SameLine();

                                ImGui::Separator();
                                ImGui::Separator();
                                ImGui::Text("Amount of sent kilobytes (total):");
                                ImGui::Text("%f", (float)Mqtt::getInstance().getSentBytes()/1000.f);
                                ImGui::Text("Amount of sent messages (total):");
                                ImGui::Text("%d", Mqtt::getInstance().getMessagesSent());
                                ImGui::Text("Current emission (kilobytes/sec):");
                                ImGui::Text("%f", (float)Mqtt::getInstance().getEmissionSpeed()/1000.f);
                                ImGui::Separator();

                                ImGui::TextWrapped("Receive as MessagePack to optimize network communication?\nIn future, this will be the default!");
                                ImGui::Checkbox("Use MessagePack (receive)", Mqtt::getInstance().useMessagePackReceiveBool());

                                ImGui::TextWrapped("Receive with compression? (receive)");

                                ImGui::PushID("ReceiveComp");
                                ImGui::RadioButton("None", Mqtt::getInstance().getCompressionReceiveInt(), 0); ImGui::SameLine();
                                ImGui::RadioButton("GZip", Mqtt::getInstance().getCompressionReceiveInt(), 1); ImGui::SameLine();
                                ImGui::RadioButton("ZLib", Mqtt::getInstance().getCompressionReceiveInt(), 2); ImGui::SameLine();
                                ImGui::PopID();
                                ImGui::Separator();
                                ImGui::Text("Amount of received kilobytes (total):");
                                ImGui::Text("%f", (float)Mqtt::getInstance().receivedBytesTotal/1000.f);
                                ImGui::Text("Amount of received messages (total):");
                                ImGui::Text("%d", Mqtt::getInstance().receivedMessages);
                                ImGui::Text("Currently receiving (kilobytes/sec):");
                                ImGui::Text("%f", (float)Mqtt::getInstance().receivedBytesLastSecond/1000.f);

                                ImGui::Separator();
                                ImGui::Checkbox("Print sending msgs?", &Mqtt::getInstance().printSendingMsgs);
                                ImGui::Checkbox("Print receiving msgs?", &Mqtt::getInstance().printReceivingMsgs);

                                ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Robot"))
                        {

                                auto sim = dynamic_cast<Simulation*>(s_application);
                                auto robot = sim->GetRobot();
                                ImVec2 button_sz = ImVec2(-1, 0);


                                ImGui::Text("Close objects");

                                for(auto&& item : robot->getClosebyObjects())
                                {
                                    ImGui::Separator();

                                    auto pos = item->getPosition();
                                    std::string objectText = item->name + "[" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + "]";

                                    ImGui::BulletText("%s", objectText.c_str());

                                    ImGui::Separator();
                                }

                                ImGui::Text("Items in storage:");

                                int i = 0;
                                for(auto&& item : robot->getStorage())
                                {

                                    ImGui::PushID(i);

                                    ImGui::Separator();

                                    //auto windowWidth = ImGui::GetWindowSize().x;
                                    //auto textWidth   = ImGui::CalcTextSize(std::string{item["type"]}.c_str()).x;

                                    //ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
                                    ImGui::Text("%s. Mass: %f.", std::string{item["type"]}.c_str(), (float)item["mass"]);
                                    if(ImGui::Button("Drop", button_sz))
                                    {
                                        robot->drop(i);
                                    }


                                    ImGui::Separator();

                                    ImGui::PopID();
                                    i++;
                                }

                                ImGui::Separator();

                                ImGui::Text("Storage mass (tot): %f", robot->getStorageMass());

                                ImGui::Separator();

                                ImGui::Text("Items for pickup:");

                                for(auto&& item : robot->getItemsForPickup())
                                {
                                    ImGui::BulletText("%s", item->name.c_str());
                                }


                                ImGui::Separator();

                                if(ImGui::Button("Pickup", button_sz))
                                {
                                    robot->pickup();
                                }

                                ImGui::Separator();

                                ImGui::SliderFloat("Laser Angle", robot->LaserAngleDegreesPtr(), 0.f, 360.f);
                                if(ImGui::Button("Shoot Laser", button_sz))
                                {
                                    robot->shootLaser();
                                }

                                ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Sim"))
                        {
                                ImVec2 button_sz = ImVec2(-1, 0);

                                if (ImGui::Button("Restart Simulation (R)", button_sz))
                                {
                                    RestartSimulation();
                                }

                                if (ImGui::Button("Trigger Immediate Earthquake (F)", button_sz))
                                {
                                    auto sim = dynamic_cast<Simulation*>(s_application);
                                    sim->earthquake.trigger(350.f, 500);
                                }

                                if (ImGui::Button("Trigger Immediate Volcano (V)", button_sz))
                                {
                                    auto sim = dynamic_cast<Simulation*>(s_application);
                                    sim->volcano->trigger(3500.f, 500);
                                }

                                if (ImGui::Button("Quit", button_sz))
                                {
                                    glfwSetWindowShouldClose(g_mainWindow, GL_TRUE);
                                }

                                ImGui::EndTabItem();
                        }


			if (ImGui::BeginTabItem("Physics"))
			{
				ImGui::SliderInt("Vel Iters", &s_settings.m_velocityIterations, 0, 50);
				ImGui::SliderInt("Pos Iters", &s_settings.m_positionIterations, 0, 50);
				ImGui::SliderFloat("Hertz", &s_settings.m_hertz, 5.0f, 120.0f, "%.0f hz");

				ImGui::Separator();

				ImGui::Checkbox("Sleep", &s_settings.m_enableSleep);
				ImGui::Checkbox("Warm Starting", &s_settings.m_enableWarmStarting);
				ImGui::Checkbox("Time of Impact", &s_settings.m_enableContinuous);
				ImGui::Checkbox("Sub-Stepping", &s_settings.m_enableSubStepping);

				ImGui::Separator();

				ImGui::Checkbox("Shapes", &s_settings.m_drawShapes);
				ImGui::Checkbox("Joints", &s_settings.m_drawJoints);
				ImGui::Checkbox("AABBs", &s_settings.m_drawAABBs);
				ImGui::Checkbox("Contact Points", &s_settings.m_drawContactPoints);
				ImGui::Checkbox("Contact Normals", &s_settings.m_drawContactNormals);
				ImGui::Checkbox("Contact Impulses", &s_settings.m_drawContactImpulse);
				ImGui::Checkbox("Friction Impulses", &s_settings.m_drawFrictionImpulse);
				ImGui::Checkbox("Center of Masses", &s_settings.m_drawCOMs);
				ImGui::Checkbox("Statistics", &s_settings.m_drawStats);
				ImGui::Checkbox("Profile", &s_settings.m_drawProfile);

				ImVec2 button_sz = ImVec2(-1, 0);
				if (ImGui::Button("Pause Physics (P)", button_sz))
				{
					s_settings.m_pause = !s_settings.m_pause;
				}

				if (ImGui::Button("Single Step (O)", button_sz))
				{
					s_settings.m_singleStep = !s_settings.m_singleStep;
				}

				ImGui::EndTabItem();
			}

			ImGuiTreeNodeFlags leafNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			leafNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

			ImGui::EndTabBar();
		}

		ImGui::End();

		s_application->UpdateUI();
	}
}

int main(int, char**)
{
#if defined(_WIN32)
	// Enable memory-leak reports
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif

	char buffer[128];

	s_settings.Load();

	glfwSetErrorCallback(glfwErrorCallback);

	g_camera.m_width = s_settings.m_windowWidth;
	g_camera.m_height = s_settings.m_windowHeight;

	if (glfwInit() == 0)
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

#if __APPLE__
    const char* glslVersion = "#version 150";
#else
    const char* glslVersion = NULL;
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // To Enable MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);

    bool fullscreen = false;

    if (fullscreen)
    {
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        g_mainWindow = glfwCreateWindow(mode->width, mode->height, "Marsim - Mars Simulator", glfwGetPrimaryMonitor(), NULL);
    }
    else
    {
        g_mainWindow = glfwCreateWindow(g_camera.m_width, g_camera.m_height, "Marsim - Mars Simulator", NULL, NULL);
    }

	if (g_mainWindow == NULL)
	{
		fprintf(stderr, "Failed to open GLFW g_mainWindow.\n");
		glfwTerminate();
		return -1;
	}

	glfwGetWindowContentScale(g_mainWindow, &s_displayScale, &s_displayScale);

	glfwMakeContextCurrent(g_mainWindow);

	// Load OpenGL functions using glad
	int version = gladLoadGL(glfwGetProcAddress);
	printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	glfwSetWindowSizeCallback(g_mainWindow, ResizeWindowCallback);
	glfwSetKeyCallback(g_mainWindow, KeyCallback);
	glfwSetCharCallback(g_mainWindow, CharCallback);
	glfwSetMouseButtonCallback(g_mainWindow, MouseButtonCallback);
	glfwSetCursorPosCallback(g_mainWindow, MouseMotionCallback);
	glfwSetScrollCallback(g_mainWindow, ScrollCallback);

	g_debugDraw.Create();

	CreateUI(g_mainWindow, glslVersion);

    s_application = Simulation::Create();
    Mqtt::getInstance().setSimulationPtr(dynamic_cast<Simulation *>(s_application));


	// Control the frame rate. One draw per monitor refresh.
	//glfwSwapInterval(1);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	std::chrono::duration<double> frameTime(0.0);
	std::chrono::duration<double> sleepAdjust(0.0);

	while (!glfwWindowShouldClose(g_mainWindow))
	{
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

		glfwGetWindowSize(g_mainWindow, &g_camera.m_width, &g_camera.m_height);

        int bufferWidth, bufferHeight;
        glfwGetFramebufferSize(g_mainWindow, &bufferWidth, &bufferHeight);
        glViewport(0, 0, bufferWidth, bufferHeight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		if (g_debugDraw.m_showUI)
		{
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(ImVec2(float(g_camera.m_width), float(g_camera.m_height)));
			ImGui::SetNextWindowBgAlpha(0.0f);
			ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
			ImGui::End();

			s_application->DrawTitle("Marsim by Johan Lind & Ermias Tewolde");
		}

		s_application->Step(s_settings);

                auto sim = dynamic_cast<Simulation*>(s_application);
                Mqtt::getInstance().processMqtt(sim->GetStepCount());

		UpdateUI();

		// ImGui::ShowDemoWindow();

		if (g_debugDraw.m_showUI)
		{
			sprintf(buffer, "%.1f ms", 1000.0 * frameTime.count());
			g_debugDraw.DrawString(5, g_camera.m_height - 20 * ImGui::GetIO().FontGlobalScale, buffer);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_mainWindow);

		glfwPollEvents();

		// Throttle to cap at 60Hz. This adaptive using a sleep adjustment. This could be improved by
		// using mm_pause or equivalent for the last millisecond.
		std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		std::chrono::duration<double> target(1.0 / 60.0);
		std::chrono::duration<double> timeUsed = t2 - t1;
		std::chrono::duration<double> sleepTime = target - timeUsed + sleepAdjust;
		if (sleepTime > std::chrono::duration<double>(0))
		{
			std::this_thread::sleep_for(sleepTime);
		}

		std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
		frameTime = t3 - t1;

		// Compute the sleep adjustment using a low pass filter
		sleepAdjust = 0.9 * sleepAdjust + 0.1 * (target - frameTime);
	}

	delete s_application;
    s_application = nullptr;

	g_debugDraw.Destroy();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	glfwTerminate();

	s_settings.Save();

	return 0;
}
