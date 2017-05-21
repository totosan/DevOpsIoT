#ifdef __cplusplus
class Updater
{
  public:
  Updater();
    void GetUpdate(char* fromUrl);
};
extern "C" Updater* Updater_create();
extern "C" void Updater_dispose(Updater* updaterClass);
extern "C" void Updater_do(Updater* updater, char* fromUrl);
#else
typedef struct Updater Updater;
#endif
