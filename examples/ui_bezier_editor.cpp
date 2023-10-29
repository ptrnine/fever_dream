#include <imgui-SFML.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "grx/ui/bezier_editor.hpp"

int main() {
    sf::RenderWindow window{
        sf::VideoMode(1800, 900),
        "ui_bezier_editor",
        sf::Style::Default,
        sf::ContextSettings{24, 8, 4, 3, 0},
    };
    window.setVerticalSyncEnabled(true);
    window.setActive();
    ImGui::SFML::Init(window);

    sf::Clock deltaClock;

    ui::bezier_editor bz_edit{"Bezier Edit"};

    sf::Clock clock;

    while (window.isOpen()) {
        auto timestep = clock.getElapsedTime();
        clock.restart();

        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Bezier Editor");
        bz_edit.update(timestep.asSeconds());
        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}
