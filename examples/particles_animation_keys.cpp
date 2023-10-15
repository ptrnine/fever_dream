#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>

#include "grx/scene.hpp"
#include "grx/particles.hpp"

int main() {
    sf::Vector2f     window_size{1800, 1000};
    sf::RenderWindow wnd{
        sf::VideoMode(window_size.x, window_size.y),
        "test window",
        sf::Style::Default,
        sf::ContextSettings{24, 8, 8},
    };
    wnd.setActive();
    wnd.setVerticalSyncEnabled(false);

    grx::Scene scene;
    grx::ParticlesMgr particles_mgr{scene};

    grx::Particles effect;
    effect.set_duration(2);
    auto& square = effect.create_element(sf::RectangleShape({50, 50}));
    square.setOrigin(25, 25);

    grx::AnimKeySequence<sf::Vector2f> keys;
    keys.push(grx::AnimKey<sf::Vector2f>{{0, 0}, 0, grx::Interpolation::bezier, {}, {0, 1}});
    keys.push(grx::AnimKey<sf::Vector2f>{{600, 0}, 1, grx::Interpolation::bezier, {1, 0}, {}});

    effect.add_handler("position", grx::particle::position(keys));
    particles_mgr.add_effect("position", std::move(effect));

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
                particles_mgr.play("position", 0, {float(pos.x), float(pos.y)});
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
