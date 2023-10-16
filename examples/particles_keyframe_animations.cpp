#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>

#include "grx/scene.hpp"
#include "grx/particles.hpp"

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

    grx::Scene scene;
    grx::ParticlesMgr particles_mgr{scene};

    grx::Particles effect;
    effect.set_duration(1.5);
    auto& square = effect.create_element(sf::RectangleShape({100, 100}));
    square.setOrigin(square.getSize() * 0.5f);

    grx::AnimKeySequence<float> rotation_keys;
    rotation_keys.push_linear_to_bezier(0, 0, {0.47, 1.64});
    rotation_keys.push_bezier(180, 0.5, {0.41, 0.8}, {0.47, 1.64});
    rotation_keys.push_bezier_to_linear(360, 1, {0.41, 0.8});

    grx::AnimKeySequence<core::vec2f> scale_keys;
    scale_keys.push_linear_to_bezier({1, 1}, 0, {0.47, 1.64});
    scale_keys.push_bezier({2, 2}, 0.5, {0.41, 0.8}, {0.47, 1.64});
    scale_keys.push_bezier_to_linear({1, 1}, 1, {0.41, 0.8});

    grx::AnimKeySequence<core::vec2f> position_keys;
    position_keys.push_linear_to_bezier({0, 0}, 0, {0.47, 1.64});
    position_keys.push_bezier({600, 400}, 0.5, {0.41, 0.8}, {0.47, 1.64});
    position_keys.push_bezier_to_linear({600, 200}, 1, {0.41, 0.8});

    effect.add_handler("scale", grx::particle::scale(scale_keys));
    effect.add_handler("rotation", grx::particle::rotation(rotation_keys));
    effect.add_handler("position", grx::particle::position(position_keys));
    particles_mgr.add_effect("square", std::move(effect));

    sf::Clock clock;
    bool running = true;

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
                particles_mgr.play("square", 0, {float(pos.x), float(pos.y)});
            }
        }

        if (frames % 100 == 0) {
            //std::cout << "objects: " << scene.get_elements_count()
            //          << " fps: " << double(frames) / fps_clock.getElapsedTime().asSeconds() << std::endl;
            frames = 0;
            fps_clock.restart();
        }
        ++frames;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        particles_mgr.update(timestep.asSeconds());
        scene.draw(wnd);
        wnd.display();
    }
}
