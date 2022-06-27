#ifndef __OBJ_LOADER_HPP_
#define __OBJ_LOADER_HPP_
#include "modelloader.hpp"
#include <string>

class OBJLoader : public ModelLoader
{
private:
    OBJLoader() = delete;
    OBJLoader(const OBJLoader &) = delete;
    OBJLoader &operator=(const OBJLoader &) = delete;
    GLsizei storeVertex(const std::string &vertex_str);
    void calcTangent(const GLsizei &ind_0, const GLsizei &ind_1, const GLsizei &ind_2);
    bool read();
    bool readMaterial(const std::string &mtl);

public:
    OBJLoader(const std::string &path);
};

#endif