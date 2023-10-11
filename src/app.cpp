#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

int main() {
    sf::Window wnd(sf::VideoMode(800, 600), "test window");
    wnd.setActive();

    bool running = true;
    while (running) {
        sf::Event event;
        while (wnd.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                running = false;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        wnd.display();
    }
}
