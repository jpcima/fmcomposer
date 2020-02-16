#include <SFML/Window.hpp>

#if (SFML_VERSION_MAJOR > 2) || (SFML_VERSION_MAJOR == 2 && SFML_VERSION_MINOR >= 5)

// use clipboard API of SFML >= 2.5.0

typedef sf::Clipboard Clipboard;

#else

#include <clip.h>

class Clipboard {
public:
    static sf::String getString()
    {
        std::string temp;
        if (!clip::get_text(temp))
            return sf::String();
        return sf::String::fromUtf8(temp.begin(), temp.end());
    }

    static void setString(const sf::String& text)
    {
        std::basic_string<sf::Uint8> utf8 = text.toUtf8();
        clip::set_text(reinterpret_cast<const char *>(utf8.data()));
    }
};

#endif
