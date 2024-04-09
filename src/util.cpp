#include <util.hpp>

uint32_t globalID()
{
    static std::atomic_uint _id = 1u;
    return _id++;
}

#ifdef BUILD_DEBUG
std::unordered_map<uint32_t, std::string> _idToName = {};
std::string typeName(uint32_t id)
{
    return _idToName.at(id);
}
void setTypeName(uint32_t id, std::string name)
{
    _idToName.emplace(id, name);
}
#else
std::string typeName(uint32_t id)
{
    return {};
}
void setTypeName(uint32_t id, std::string name) {}
#endif

std::mt19937 random::generator(std::random_device{}());