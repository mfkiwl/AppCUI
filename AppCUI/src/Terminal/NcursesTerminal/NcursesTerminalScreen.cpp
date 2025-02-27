#include "NcursesTerminal.hpp"

namespace AppCUI::Internal
{
using namespace Application;

const static size_t MAX_TTY_COL = 65535;
const static size_t MAX_TTY_ROW = 65535;

bool NcursesTerminal::InitScreen()
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    clear();

    colors.Init();

    size_t width  = 0;
    size_t height = 0;
    getmaxyx(stdscr, height, width);
    CHECK(height < MAX_TTY_ROW || width < MAX_TTY_COL, false, "Failed to get window sizes");
    // create canvases
    CHECK(screenCanvas.Create(width, height),
          false,
          "Fail to create an internal canvas of %d x %d size",
          width,
          height);
    CHECK(originalScreenCanvas.Create(width, height),
          false,
          "Fail to create the original screen canvas of %d x %d size",
          width,
          height);

    return true;
}

void NcursesTerminal::OnFlushToScreen()
{
    Graphics::Character* charsBuffer = this->screenCanvas.GetCharactersBuffer();
    const size_t width               = screenCanvas.GetWidth();
    const size_t height              = screenCanvas.GetHeight();
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const Graphics::Character ch = charsBuffer[y * width + x];

            cchar_t t = { 0, { ch.Code, 0 } };
            colors.SetColor(ch.Color.Foreground, ch.Color.Background);
            mvadd_wch(y, x, &t);
            colors.UnsetColor(ch.Color.Foreground, ch.Color.Background);
        }
    }
    if (mode == TerminalMode::TerminalInsert)
    {
        colors.SetColor(Graphics::Color::White, Graphics::Color::Green);
        mvaddch(height - 1, width - 3, ' ');
        mvaddch(height - 1, width - 2, 'I');
        mvaddch(height - 1, width - 1, ' ');
        colors.UnsetColor(Graphics::Color::White, Graphics::Color::Green);
    }
    else if (mode == TerminalMode::TerminalNormal)
    {
        colors.SetColor(Graphics::Color::White, Graphics::Color::DarkRed);
        mvaddch(height - 1, width - 3, ' ');
        mvaddch(height - 1, width - 2, 'N');
        mvaddch(height - 1, width - 1, ' ');
        colors.UnsetColor(Graphics::Color::White, Graphics::Color::DarkRed);
    }
    move(lastCursorY, lastCursorX);
    refresh();
}
void NcursesTerminal::OnFlushToScreen(const Graphics::Rect& /*r*/)
{
    // No implementation for the moment, copy the entire screem
    OnFlushToScreen();
}

bool NcursesTerminal::OnUpdateCursor()
{
    if (screenCanvas.GetCursorVisibility())
    {
        curs_set(1);
        move(screenCanvas.GetCursorY(), screenCanvas.GetCursorX());
    }
    else
    {
        curs_set(0);
    }
    refresh();
    return true;
}
void NcursesTerminal::RestoreOriginalConsoleSettings()
{
}
bool NcursesTerminal::HasSupportFor(Application::SpecialCharacterSetType type)
{
    switch (type)
    {
    case AppCUI::Application::SpecialCharacterSetType::Unicode:
    {
       auto term = getenv("TERM");
       if ((term) && (strcmp(term, "linux") == 0))
           return false; // we are in a real linux tty and as such this mode will not be supported
        // otherwise we are in an "X" mode terminal that has a font that propertly supports some characters
        return true;
    }
    case AppCUI::Application::SpecialCharacterSetType::LinuxTerminal:
        // Linux terminal always work (this is a subset of unicode characters so it will be available for both TTY and "X" mode terminals)
        return true;
    case AppCUI::Application::SpecialCharacterSetType::Ascii:
        // ascii always works
        return true;
    default:
        RETURNERROR(false, "Unknwon special character set --> this is a fallback case, it should not be reached !");
        break;
    }
}
void NcursesTerminal::UnInitScreen()
{
    endwin();
}
} // namespace AppCUI::Internal
