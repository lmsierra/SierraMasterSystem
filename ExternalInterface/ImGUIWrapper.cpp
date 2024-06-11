#include "ImGUIWrapper.h"

#include "SDLInterface.h"
#include "Z80.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"

static struct ImFont* roboto_medium_font;
static struct ImFont* proggy_tiny_font;

void ImGUIWrapper::Init(const SDLInterface* sdl_interface)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

/*
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
*/
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    static constexpr const char* ProggyTinyFontPath   = "Libs/imgui/fonts/ProggyTiny.ttf";
    static constexpr const char* RobotoMediumFontPath = "Libs/imgui/fonts/Roboto-Medium.ttf";

    proggy_tiny_font = io.Fonts->AddFontFromFileTTF(ProggyTinyFontPath, 18.0f);
    roboto_medium_font = io.Fonts->AddFontFromFileTTF(RobotoMediumFontPath, 18.0f);

    IM_ASSERT(proggy_tiny_font != nullptr);
    IM_ASSERT(roboto_medium_font != nullptr);
    
    ImGui_ImplSDL2_InitForSDLRenderer(sdl_interface->GetWindow(), sdl_interface->GetRenderer());
    ImGui_ImplSDLRenderer2_Init(sdl_interface->GetRenderer());
}

void ImGUIWrapper::Tick(float DeltaTime)
{

}

void ImGUIWrapper::ProcessEvent(const SDL_Event* event)
{
    ImGui_ImplSDL2_ProcessEvent(event);
}

void ImGUIWrapper::NewFrame()
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGUIWrapper::DrawRegisters(const Z80* z80)
{
    ImGui::Begin("Registers", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    const ImVec4 title_color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::TextColored(title_color, "S Z Y H X P/V N C");
    AddBinaryText("%c %c %c %c %c  %c  %c %c", z80->m_reg_AF.lo);

    ImGui::Separator();

    if (ImGui::BeginTable("register_table", 3))
    {
        AddRegisterText("A",  z80->m_reg_AF.hi);
        AddRegisterText("F",  z80->m_reg_AF.lo);
        AddRegisterText("A'", z80->m_reg_AF_shadow.hi);
        AddRegisterText("F'", z80->m_reg_AF_shadow.lo);
        AddRegisterText("B",  z80->m_reg_BC.hi);
        AddRegisterText("C",  z80->m_reg_BC.lo);
        AddRegisterText("B'", z80->m_reg_BC_shadow.hi);
        AddRegisterText("C'", z80->m_reg_BC_shadow.lo);
        AddRegisterText("D'", z80->m_reg_DE.hi);
        AddRegisterText("E'", z80->m_reg_DE.lo);
        AddRegisterText("D'", z80->m_reg_DE_shadow.hi);
        AddRegisterText("E'", z80->m_reg_DE_shadow.lo);
        AddRegisterText("H",  z80->m_reg_HL.hi);
        AddRegisterText("L",  z80->m_reg_HL.lo);
        AddRegisterText("H'", z80->m_reg_HL_shadow.hi);
        AddRegisterText("L'", z80->m_reg_HL_shadow.lo);

        ImGui::EndTable();
    }

    ImGui::End();
}

void ImGUIWrapper::Render(const SDLInterface* sdl_interface)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    auto renderer = sdl_interface->GetRenderer();

    ImGui::Render();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

void ImGUIWrapper::Shutdown()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ImGUIWrapper::AddBinaryText(const char* format, const byte value)
{
    ImGui::Text(format, (value & 0b10000000) ? '1' : '0',
                        (value & 0b01000000) ? '1' : '0',
                        (value & 0b00100000) ? '1' : '0',
                        (value & 0b00010000) ? '1' : '0',
                        (value & 0b00001000) ? '1' : '0',
                        (value & 0b00000100) ? '1' : '0',
                        (value & 0b00000010) ? '1' : '0',
                        (value & 0b00000001) ? '1' : '0');

}

void ImGUIWrapper::AddRegisterText(const char* title, const word value)
{
    const ImVec4 title_color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextColored(title_color, title);
    ImGui::TableNextColumn();
    AddBinaryText("%c%c%c%c %c%c%c%c", value);
    ImGui::TableNextColumn();
    ImGui::Text("%02X", value);
}
