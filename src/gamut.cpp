#include <gamut.hpp>

// Function to parse a single line of data
std::vector<std::string> parseLine(const std::string &line) {
  std::stringstream ss(line);
  std::string token;
  std::vector<std::string> tokens;
  while (std::getline(ss, token, ' ')) {
    tokens.push_back(token);
  }
  return tokens;
}

// Function to parse a double value from a string
double parseDouble(const std::string &str) {
  try {
    return std::stod(str);
  } catch (const std::invalid_argument &e) {
    // Handle invalid double format
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Function to parse a GamutVertex from a line
GamutVertex parseVertex(const std::vector<std::string> &tokens) {
  if (tokens.size() < 4) {
    // Handle invalid vertex format
    return {std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN()};
  }
  return {parseDouble(tokens[1]), parseDouble(tokens[2]),
          parseDouble(tokens[3])};
}

// Function to parse data based on keyword (can be extended for other data
// types)
bool parseDataLine(const std::vector<std::string> &tokens, GamutData &data) {
  if (tokens[0] == "DESCRIPTOR") {
    data.descriptor = tokens[1];
    for (size_t i = 2; i < tokens.size(); ++i) {
      data.descriptor += " " + tokens[i];
    }
  } else if (tokens[0] == "ORIGINATOR") {
    data.originator = tokens[1];
    for (size_t i = 2; i < tokens.size(); ++i) {
      data.originator += " " + tokens[i];
    }
  } else if (tokens[0] == "CREATED") {
    data.created = tokens[1];
    for (size_t i = 2; i < tokens.size(); ++i) {
      data.created += " " + tokens[i];
    }
  } else if (tokens[0] == "COLOR_REP") {
    data.color_rep = tokens[1];
  } else if (tokens[0] == "GAMUT_CENTER" || tokens[0] == "CSPACE_WHITE" ||
             tokens[0] == "GAMUT_WHITE" || tokens[0] == "CSPACE_BLACK" ||
             tokens[0] == "GAMUT_BLACK") {
    data.gamut_center = parseVertex(
        tokens); // Can be replaced with specific data parsing based on keyword
  } else if (tokens[0].substr(0, 4) == "CUSP_") {
    data.cusps.push_back(parseVertex(tokens));
  } else {
    // Handle unknown keywords or parsing errors
    return false;
  }
  return true;
}

// Function to read the gamut data from a file
std::shared_ptr<GamutData> readGamutData(const std::string &filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    return nullptr; // Error: could not open file
  }

  std::shared_ptr<GamutData> data =
      std::make_shared<GamutData>(); // Create a shared pointer to store the
                                     // gamut data

  std::string line;
  while (std::getline(file, line)) {
    std::vector<std::string> tokens = parseLine(line);
    if (!parseDataLine(tokens, *data)) {
      // Handle parsing errors
      return nullptr;
    }
  }

  return data;
}