#include "utils.hpp"

#if __APPLE__ && __MACH__
#include <sys/sysctl.h>
#else
#endif

namespace swarm
{
#ifdef _WIN32
  bool FileExists(const char *filename)
  {
    struct _stat status;
    return _access(filename, 0) == 0 && _stat(filename, &status) == 0 && (status.st_mode & _S_IFREG);
  }
#else
  bool FileExists(const char *filename)
  {
    struct stat status;
    return access(filename, 0) == 0 && stat(filename, &status) == 0 && (status.st_mode & S_IFREG);
  }
#endif

#ifdef _WIN32
  string toString(char const * const format, ... ) 
  {
    va_list args;
    va_start(args, format);

    const int len = _vscprintf(format, args) + 1;

    char* buf = (char*)_alloca(len);
    vsprintf_s(buf, len, format, args);
    va_end(args);

    return string(buf);
  }

  //-----------------------------------------------------------------------------
  string ToString(const char* fmt, va_list args)
  {
    const int len = _vscprintf(fmt, args) + 1;

    char* buf = (char*)_alloca(len);
    vsprintf_s(buf, len, fmt, args);
    va_end(args);

    return string(buf);
  }

#else
  string toString(char const * const format, ... ) 
  {
    va_list args;
    va_start(args, format);

    va_list argcopy;
    va_copy(argcopy, args);
    int len = vsnprintf(nullptr, 0, format, argcopy) + 1;
    va_end(argcopy);

    char* buf = (char*)alloca(len);
    vsprintf(buf, format, args);
    va_end(args);

    return string(buf);
  }

  //-----------------------------------------------------------------------------
  string ToString(const char* fmt, va_list args)
  {
    va_list argcopy;
    va_copy(argcopy, args);
    int len = vsnprintf(nullptr, 0, fmt, argcopy) + 1;
    va_end(argcopy);

    char* buf = (char*)alloca(len);
    vsprintf(buf, fmt, args);
    va_end(args);

    return string(buf);
  }

#endif

  //-----------------------------------------------------------------------------
  float randf(float a, float b)
  {
    float t = (float)rand() / RAND_MAX;
    return lerp(a, b, t);
  }

  float gaussianRand(float mean, float variance) {
    // Generate a gaussian from the sum of uniformly distributed random numbers
    // (Central Limit Theorem)
    double sum = 0;
    const int numIters = 10;
    for (int i = 0; i < numIters; ++i) {
      sum += randf(-variance, variance);
    }
    return (float)(mean + sum / numIters);
  }

  //-----------------------------------------------------------------------------
  sf::Vertex MakeVertex(int x, int y, sf::Color color)
  {
    return sf::Vertex(sf::Vector2f((float)x, (float)y), color);
  }


  //-----------------------------------------------------------------------------
  bool LoadFile(const char* filename, vector<char>* buf)
  {
    FILE* f = fopen(filename, "rb");
    if (!f)
      return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf->resize(size);
    size_t bytesRead = fread(buf->data(), 1, size, f);
    if (bytesRead != size)
    {
      fclose(f);
      return false;
    }

    return true;
  }

  //-----------------------------------------------------------------------------
  bool LoadFile(const char* filename, string* str)
  {
    vector<char> buf;
    if (!LoadFile(filename, &buf))
      return false;

    str->assign(buf.data(), buf.size());
    return true;
  }

  //-----------------------------------------------------------------------------
  string FindAppRoot()
  {
#ifdef _WIN32
    char startingDir[MAX_PATH];
    if (!_getcwd(startingDir, MAX_PATH))
      return startingDir;

    // keep going up directory levels until we find "app.json", or we hit the bottom..
    char prevDir[MAX_PATH], curDir[MAX_PATH];
    ZeroMemory(prevDir, sizeof(prevDir));

    while (true)
    {
      if (!_getcwd(curDir, MAX_PATH))
        break;

      // check if we haven't moved
      if (!strncmp(curDir, prevDir, MAX_PATH))
        break;

      memcpy(prevDir, curDir, MAX_PATH);

      if (FileExists("settings.pb"))
        return curDir;

      if (_chdir("..") == -1)
        break;
    }
#else
    char startingDir[256];
    getcwd(startingDir, 256);

    // keep going up directory levels until we find "app.json", or we hit the bottom..
    char prevDir[256], curDir[256];
    memset(prevDir, 0, sizeof(prevDir));

    while (true)
    {
      getcwd(curDir, 256);
      // check if we haven't moved
      if (!strcmp(curDir, prevDir))
        break;

      memcpy(prevDir, curDir, 256);

      if (FileExists("settings.pb"))
        return curDir;

      if (chdir("..") == -1)
        break;
    }
#endif
    return "";
  }


#ifdef _WIN32
#else
  bool IsDebuggerPresent()
  {
    int mib[4];
    struct kinfo_proc info;
    size_t size;

    info.kp_proc.p_flag = 0;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    size = sizeof(info);
    sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

    return ((info.kp_proc.p_flag & P_TRACED) != 0);
  }

  void DebugOutput(const char* fmt, ...)
  {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }

  #endif

}

