#ifndef __LOOM_OBJECT_HPP__
#define __LOOM_OBJECT_HPP__

struct Yarn {
private:
public:

    void assemble(std::ifstream& input_file);
    void assemble(const char* source);
    void link();
    void write(const char* fname);

};

#endif//__LOOM_OBJECT_HPP__