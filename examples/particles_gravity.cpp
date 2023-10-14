#include <iostream>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/OpenGL.hpp>

#include "grx/scene.hpp"
#include "grx/particles.hpp"

int main() {
    sf::ContextSettings context_settings{0, 0, 8, 4, 6};
    sf::Vector2f        window_size{1800, 1000};
    sf::RenderWindow    wnd(
        sf::VideoMode(window_size.x, window_size.y), "test window", sf::Style::Default, context_settings);
    wnd.setActive();
    wnd.setVerticalSyncEnabled(false);

    grx::Scene scene;

    grx::ParticlesMgr particles_mgr{scene};

    grx::Particles particles;
    particles.set_duration(grx::duration_endless);
    for (float x = 100; x < window_size.x - 100; x += 24) {
        for (float y = 100; y < window_size.y - 100; y += 24) {
            sf::CircleShape element{4};
            element.setFillColor(sf::Color::Yellow);
            element.setPosition(x, y);
            particles.get_elements().push_back(std::move(element));
        }
    }
    particles.add_handler("gravity", grx::particle::gravity(std::vector<float>(2278, 0.1f)));
    particles_mgr.add_effect("gravity-test", std::move(particles));

    bool      running = true;
    sf::Clock clock;

    size_t    frames = 0;
    sf::Clock fps_clock;

    particles_mgr.play("gravity-test", 0);

    while (running) {
        auto timestep = clock.getElapsedTime();
        clock.restart();

        if (frames % 100 == 0) {
            std::cout << "objects: " << scene.get_elements_count()
                      << " fps: " << double(frames) / fps_clock.getElapsedTime().asSeconds() << std::endl;
            frames = 0;
            fps_clock.restart();
        }
        ++frames;

        sf::Event event;
        while (wnd.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                running = false;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        particles_mgr.update(timestep.asSeconds());
        scene.draw(wnd);
        wnd.display();
    }
}
