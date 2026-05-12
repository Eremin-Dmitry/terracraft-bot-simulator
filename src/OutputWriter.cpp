#include "OutputWriter.hpp"

#include <fstream>

namespace Terracraft
{
    bool OutputWriter::WriteLines(const std::string& outputFilePath, const std::vector<std::string>& lines) const
    {
        std::ofstream outputFile(outputFilePath);
        if (!outputFile.is_open())
        {
            return false;
        }

        for (const std::string& line : lines)
        {
            outputFile << line << '\n';
        }

        return true;
    }

    bool OutputWriter::WriteSingleLine(const std::string& outputFilePath, const std::string& line) const
    {
        return WriteLines(outputFilePath, { line });
    }
}
