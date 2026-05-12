#pragma once

#include <string>
#include <vector>

namespace Terracraft
{
    class OutputWriter
    {
    public:
        bool WriteLines(const std::string& outputFilePath, const std::vector<std::string>& lines) const;
        bool WriteSingleLine(const std::string& outputFilePath, const std::string& line) const;
    };
}
