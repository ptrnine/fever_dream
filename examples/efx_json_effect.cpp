#include <iostream>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/OpenGL.hpp>

#include "grx/scene.hpp"
#include "grx/efx_editor.hpp"


int main() {
    core::vec2u      window_size{1800, 1000};
    sf::RenderWindow wnd{
        sf::VideoMode(window_size.x(), window_size.y()),
        "test window",
        sf::Style::Default,
        sf::ContextSettings{24, 8, 8},
    };
    wnd.setActive();
    wnd.setVerticalSyncEnabled(false);

    grx::scene scene;
    grx::efx_mgr efx_mgr{scene};
    grx::texture_mgr tx_mgr;

    grx::efx_editor::efx_builder efx_builder(tx_mgr, "examples/efx/test_effect.json");
    auto [efx_name, efx] = efx_builder.build();

    efx_mgr.add_effect(efx_name, std::move(efx));

    sf::Clock clock;
    bool      running = true;

    size_t    frames = 0;
    sf::Clock fps_clock;

    while (running) {
        auto timestep = clock.getElapsedTime();
        clock.restart();

        sf::Event event;
        while (wnd.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                running = false;
            if (event.type == sf::Event::MouseButtonPressed) {
                auto pos = sf::Mouse::getPosition(wnd);
                efx_mgr.play(efx_name, 0, {float(pos.x), float(pos.y)});
            }
        }

        if (frames % 100 == 0) {
            std::cout << "objects: " << scene.get_elements_count()
                      << " fps: " << double(frames) / fps_clock.getElapsedTime().asSeconds() << std::endl;
            frames = 0;
            fps_clock.restart();
        }
        ++frames;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        efx_mgr.update(timestep.asSeconds());
        scene.draw(wnd);
        wnd.display();
    }
}
