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
    if (!parseResult.input.has_value())
    {
        outputWriter.WriteSingleLine(s_defaultOutputFilePath, parseResult.invalidLine);
        return 0;
    }

    Terracraft::ParsedInput parsedInput = std::move(parseResult.input.value());
    Terracraft::AliceBot bot(std::move(parsedInput.dungeon), parsedInput.settings);

    const std::vector<std::string> actions = bot.RunSimulation();
    if (!outputWriter.WriteLines(s_defaultOutputFilePath, actions))
    {
        std::cerr << "Failed to write result.txt\n";
        return 1;
    }

    return 0;
}
