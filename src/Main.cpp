#include "AliceBot.hpp"
#include "InputParser.hpp"
#include "OutputWriter.hpp"

#include <iostream>
#include <string>

namespace
{
    constexpr const char* s_defaultOutputFilePath = "result.txt";
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: task <input-file>\n";
        return 1;
    }

    const std::string inputFilePath = argv[1];

    Terracraft::InputParser inputParser;
    Terracraft::OutputWriter outputWriter;

    Terracraft::ParseResult parseResult = inputParser.ParseFile(inputFilePath);
    if (!parseResult.m_input.has_value())
    {
        outputWriter.WriteSingleLine(s_defaultOutputFilePath, parseResult.m_invalidLine);
        return 0;
    }

    Terracraft::ParsedInput parsedInput = std::move(parseResult.m_input.value());
    Terracraft::AliceBot bot(std::move(parsedInput.m_dungeon), parsedInput.m_settings);

    const std::vector<std::string> actions = bot.RunSimulation();
    if (!outputWriter.WriteLines(s_defaultOutputFilePath, actions))
    {
        std::cerr << "Failed to write result.txt\n";
        return 1;
    }

    return 0;
}
