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

    sf::Texture texture;
    texture.loadFromFile("resources/test/think.png");
    texture.setSmooth(true);

    grx::Particles effect;
    effect.set_duration(2);

    std::vector<core::vec2f> velocities;
    std::vector<float> masses;

    for (size_t i = 0; i < 12; ++i) {
        constexpr auto pi2 = M_PIf32 * 2;
        auto angle = float(i) / 12 * pi2;

        auto x = std::cos(angle);
        auto y = std::sin(angle);

        velocities.push_back(core::vec2f{-y, x} * 600.f);
        masses.push_back(1.f);

        auto& think = effect.create_element(sf::CircleShape(50, 128));
        think.setTexture(&texture);
        think.setOrigin(50, 50);
        think.setPosition(core::vec2f{x * 50, y * 50});
    }

    auto& think = effect.create_element(sf::CircleShape(70, 128));
    think.setOrigin(70, 70);
    think.setPosition(0, 0);
    think.setTexture(&texture);

    velocities.push_back({0.f, 0.f});
    masses.push_back(1000.f);

    effect.add_handler("gravity", grx::particle::gravity(masses, velocities));
    effect.add_handler("opacity", [](sf::Shape& shape, const grx::ParticleState& state) {
        constexpr auto threshold = 0.4f;
        auto opacity = 1.f;
        if (state.time_elapsed_coef > threshold)
            opacity = 1.f - core::inverse_lerp(threshold, 1.0, state.time_elapsed_coef);

        opacity = core::dependence::quadratic(opacity);

        auto color = shape.getFillColor();
        color.a = uint8_t(255 * opacity);
        shape.setFillColor(color);
    });

    particles_mgr.add_effect("think", std::move(effect));

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
                particles_mgr.play("think", 0, {float(pos.x), float(pos.y)}, {0.5, 0.5});
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

        particles_mgr.update(timestep.asSeconds());
        scene.draw(wnd);
        wnd.display();
    }
}
