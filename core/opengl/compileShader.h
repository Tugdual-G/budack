#ifndef COMPILESHADER_H_
#define COMPILESHADER_H_

unsigned int compileVertexShader(const char *sourcefname);

unsigned int compileFragmentShader(const char *sourcefname);

unsigned int linkShaders(unsigned int Vshader, unsigned int Fshader);

unsigned int computeShaderProgram(const char *sourcefname);

#endif // COMPILESHADER_H_
